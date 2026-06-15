#include "matchscraper.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTimer>
#include <functional>

namespace {

const char kPageUrl[] = "https://www.sporttery.cn/jc/zqszsc/";
const char kScheduleApiUrl[] =
    "https://webapi.sporttery.cn/gateway/uniform/football/getMatchListV1.qry?clientCode=3001";
const char kWorldCupLeagueId[] = "72";
const char kUserAgent[] =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
    "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";
const int kMaxFetchAttempts = 3;
const int kRetryDelayMs = 2000;

bool looksLikeJson(const QByteArray &bytes)
{
    const QByteArray trimmed = bytes.trimmed();
    return !trimmed.isEmpty() && trimmed.at(0) == '{';
}

bool isWorldCupMatch(const QJsonObject &obj)
{
    const QString leagueId = obj.value(QStringLiteral("leagueId")).toString();
    const QString leagueName = obj.value(QStringLiteral("leagueAbbName")).toString();
    return leagueId == QString::fromLatin1(kWorldCupLeagueId)
        || leagueName == QStringLiteral("世界杯");
}

MatchItem parseMatchItem(const QJsonObject &obj)
{
    MatchItem item;
    item.homeTeam = obj.value(QStringLiteral("homeTeamAllName")).toString();
    item.awayTeam = obj.value(QStringLiteral("awayTeamAllName")).toString();
    item.matchNumStr = obj.value(QStringLiteral("matchNumStr")).toString();
    item.matchDate = obj.value(QStringLiteral("matchDate")).toString();
    item.matchTime = obj.value(QStringLiteral("matchTime")).toString();
    item.matchId = static_cast<qint64>(obj.value(QStringLiteral("matchId")).toDouble());

    const QJsonArray oddsList = obj.value(QStringLiteral("oddsList")).toArray();
    auto tryPool = [&item](const QJsonArray &list, const QString &poolCode) -> bool {
        for (const QJsonValue &value : list) {
            const QJsonObject oddsObj = value.toObject();
            if (oddsObj.value(QStringLiteral("poolCode")).toString() != poolCode)
                continue;
            const QString homeWin = oddsObj.value(QStringLiteral("h")).toString().trimmed();
            const QString awayWin = oddsObj.value(QStringLiteral("a")).toString().trimmed();
            if (homeWin.isEmpty() || awayWin.isEmpty())
                continue;
            item.homeOdds = homeWin;
            item.awayOdds = awayWin;
            item.hasOdds = true;
            return true;
        }
        return false;
    };
    if (!tryPool(oddsList, QStringLiteral("HAD")))
        tryPool(oddsList, QStringLiteral("HHAD"));

    return item;
}

QString describeNonJsonResponse(const QByteArray &bytes)
{
    const QByteArray trimmed = bytes.trimmed();
    if (trimmed.isEmpty())
        return QStringLiteral("服务器返回空数据");

    const QString preview = QString::fromUtf8(trimmed.left(80));
    if (preview.startsWith(QStringLiteral("<!DOCTYPE"), Qt::CaseInsensitive)
        || preview.startsWith(QStringLiteral("<html"), Qt::CaseInsensitive)) {
        return QStringLiteral("服务器返回网页而非 JSON（可能被 WAF 拦截），请稍后重试");
    }
    return QStringLiteral("响应内容: %1...").arg(preview);
}

QString curlExecutablePath()
{
    const QString systemCurl = QStringLiteral("C:/Windows/System32/curl.exe");
    if (QFile::exists(systemCurl))
        return systemCurl;
    return QStringLiteral("curl.exe");
}

QStringList buildCurlArguments(const QString &url, const QString &referer)
{
    return {
        QStringLiteral("-sL"),
        QStringLiteral("--max-time"),
        QStringLiteral("30"),
        QStringLiteral("--retry"),
        QStringLiteral("2"),
        QStringLiteral("--retry-delay"),
        QStringLiteral("2"),
        QStringLiteral("--retry-all-errors"),
        QStringLiteral("-A"),
        QString::fromLatin1(kUserAgent),
        QStringLiteral("-H"),
        QStringLiteral("Accept: application/json, text/plain, */*"),
        QStringLiteral("-H"),
        QStringLiteral("Accept-Language: zh-CN,zh;q=0.9"),
        QStringLiteral("-H"),
        QStringLiteral("Origin: https://www.sporttery.cn"),
        QStringLiteral("-H"),
        QStringLiteral("Referer: ") + referer,
        url
    };
}

} // namespace

MatchScraper::MatchScraper(QObject *parent)
    : QObject(parent)
{
}

QString MatchScraper::sslErrorHint()
{
    return QStringLiteral(
        "HTTPS 不可用（Qt 未加载 OpenSSL）。\n"
        "请将 libcrypto-1_1-x64.dll 和 libssl-1_1-x64.dll 复制到 exe 同目录，\n"
        "或在 Qt Maintenance Tool 中安装 OpenSSL 组件后重新运行。");
}

void MatchScraper::abortActiveRequest()
{
    if (m_activeProcess) {
        m_activeProcess->kill();
        m_activeProcess->deleteLater();
        m_activeProcess = nullptr;
    }
}

void MatchScraper::finishFetch()
{
    m_fetching = false;
    m_activeProcess = nullptr;
}

void MatchScraper::fetch()
{
    if (m_fetching)
        return;

    ++m_fetchGeneration;
    const quint64 generation = m_fetchGeneration;

    m_fetching = true;
    m_oddsWithData = 0;
    abortActiveRequest();

    emit fetchProgress(QStringLiteral("正在获取世界杯赛程..."));
    fetchScheduleWithRetry(generation, 0);
}

void MatchScraper::fetchScheduleWithRetry(quint64 generation, int attempt)
{
    fetchUrlAsync(QString::fromLatin1(kScheduleApiUrl), QString::fromLatin1(kPageUrl), generation,
                  [this, generation, attempt](const QByteArray &data, const QString &error) {
        if (generation != m_fetchGeneration)
            return;

        const bool needRetry = !error.isEmpty() || !looksLikeJson(data);
        if (needRetry && attempt + 1 < kMaxFetchAttempts) {
            const QString reason = error.isEmpty()
                ? describeNonJsonResponse(data)
                : error;
            emit fetchProgress(QStringLiteral("获取失败，%1 秒后重试 (%2/%3)...")
                                   .arg(kRetryDelayMs / 1000)
                                   .arg(attempt + 2)
                                   .arg(kMaxFetchAttempts));
            QTimer::singleShot(kRetryDelayMs, this, [this, generation, attempt, reason]() {
                if (generation != m_fetchGeneration)
                    return;
                Q_UNUSED(reason)
                fetchScheduleWithRetry(generation, attempt + 1);
            });
            return;
        }

        if (!error.isEmpty()) {
            finishFetch();
            emit fetchFailed(error);
            return;
        }

        if (!looksLikeJson(data)) {
            finishFetch();
            emit fetchFailed(QStringLiteral("JSON 解析失败: illegal value（%1）")
                                 .arg(describeNonJsonResponse(data)));
            return;
        }

        parseScheduleResponse(data);
    });
}

void MatchScraper::fetchUrlAsync(const QString &url, const QString &referer, quint64 generation,
                                 const std::function<void(const QByteArray &, const QString &)> &callback)
{
    Q_UNUSED(generation)
    // curl 更稳定，避免 Qt SSL/WAF 返回空数据或 HTML 导致 JSON 解析失败
    fetchViaCurlAsync(url, referer, generation, callback);
}

void MatchScraper::fetchViaCurlAsync(const QString &url, const QString &referer, quint64 generation,
                                     const std::function<void(const QByteArray &, const QString &)> &callback)
{
    m_activeProcess = new QProcess(this);
    QProcess *process = m_activeProcess;

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, process, generation, callback](int exitCode, QProcess::ExitStatus status) {
        if (process != m_activeProcess || generation != m_fetchGeneration)
            return;

        const QByteArray output = process->readAllStandardOutput();
        const QByteArray stderrData = process->readAllStandardError();
        process->deleteLater();
        m_activeProcess = nullptr;

        if (status != QProcess::NormalExit || exitCode != 0) {
            QString error = QString::fromLocal8Bit(stderrData.trimmed());
            if (error.isEmpty())
                error = QStringLiteral("curl 执行失败，退出码 %1").arg(exitCode);
            error += QStringLiteral("\n\n") + sslErrorHint();
            callback(QByteArray(), error);
            return;
        }

        if (output.isEmpty()) {
            callback(QByteArray(), QStringLiteral("curl 未返回数据"));
            return;
        }

        callback(output, QString());
    });

    connect(process, &QProcess::errorOccurred, this, [this, process, generation, callback](QProcess::ProcessError) {
        if (process != m_activeProcess || generation != m_fetchGeneration)
            return;

        const QString error = process->errorString();
        process->deleteLater();
        m_activeProcess = nullptr;
        callback(QByteArray(), QStringLiteral("curl 启动失败: %1\n\n%2").arg(error, sslErrorHint()));
    });

    process->start(curlExecutablePath(), buildCurlArguments(url, referer));
}

void MatchScraper::parseScheduleResponse(const QByteArray &bytes)
{
    m_dayGroups.clear();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        finishFetch();
        emit fetchFailed(QStringLiteral("JSON 解析失败: %1（%2）")
                             .arg(parseError.errorString(), describeNonJsonResponse(bytes)));
        return;
    }

    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("errorCode")).toString() != QStringLiteral("0")) {
        finishFetch();
        emit fetchFailed(root.value(QStringLiteral("errorMessage")).toString());
        return;
    }

    const QJsonObject value = root.value(QStringLiteral("value")).toObject();
    const QJsonArray matchInfoList = value.value(QStringLiteral("matchInfoList")).toArray();

    for (const QJsonValue &dayValue : matchInfoList) {
        const QJsonObject dayObj = dayValue.toObject();
        MatchDayGroup group;
        group.businessDate = dayObj.value(QStringLiteral("businessDate")).toString();
        group.weekday = dayObj.value(QStringLiteral("weekday")).toString();

        const QJsonArray subMatchList = dayObj.value(QStringLiteral("subMatchList")).toArray();
        for (const QJsonValue &matchValue : subMatchList) {
            const QJsonObject matchObj = matchValue.toObject();
            if (!isWorldCupMatch(matchObj))
                continue;
            group.matches.append(parseMatchItem(matchObj));
        }

        group.matchCount = group.matches.size();
        if (group.matchCount <= 0)
            continue;

        if (!group.businessDate.isEmpty())
            m_dayGroups.append(group);
    }

    if (m_dayGroups.isEmpty()) {
        finishFetch();
        emit fetchFailed(QStringLiteral("未获取到世界杯赛程数据"));
        return;
    }

    int oddsFromSchedule = 0;
    for (const MatchDayGroup &group : m_dayGroups) {
        for (const MatchItem &match : group.matches) {
            if (match.hasOdds)
                ++oddsFromSchedule;
        }
    }

    m_oddsWithData = oddsFromSchedule;
    int totalMatches = 0;
    for (const MatchDayGroup &group : m_dayGroups)
        totalMatches += group.matchCount;

    const QString message = QStringLiteral("世界杯 %1 个日期，共 %2 场，已获取 %3 场赔率")
                                .arg(m_dayGroups.size())
                                .arg(totalMatches)
                                .arg(m_oddsWithData);
    finishFetch();
    emit fetchFinished(true, message);
}
