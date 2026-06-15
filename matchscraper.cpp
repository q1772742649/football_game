#include "matchscraper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSslSocket>
#include <QUrl>
#include <functional>

namespace {

const char kPageUrl[] = "https://www.sporttery.cn/jc/zqszsc/";
const char kDetailReferer[] = "https://www.sporttery.cn/jc/zqdz/index.html";
const char kScheduleApiUrl[] =
    "https://webapi.sporttery.cn/gateway/uniform/football/getMatchListV1.qry?clientCode=3001";
const char kWorldCupLeagueId[] = "72";

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
    item.matchId = obj.value(QStringLiteral("matchId")).toVariant().toLongLong();
    return item;
}

QString fixedBonusUrl(qint64 matchId)
{
    return QStringLiteral("https://webapi.sporttery.cn/gateway/uniform/football/getFixedBonusV1.qry?clientCode=3001&matchId=%1")
        .arg(matchId);
}

} // namespace

MatchScraper::MatchScraper(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
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
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }

    if (m_activeProcess) {
        m_activeProcess->kill();
        m_activeProcess->deleteLater();
        m_activeProcess = nullptr;
    }
}

void MatchScraper::finishFetch()
{
    m_fetching = false;
    m_oddsTasks.clear();
    m_oddsFetched = 0;
    m_oddsWithData = 0;
    m_activeReply = nullptr;
    m_activeProcess = nullptr;
}

void MatchScraper::fetch()
{
    if (m_fetching)
        return;

    m_fetching = true;
    m_oddsTasks.clear();
    m_oddsFetched = 0;
    m_oddsWithData = 0;
    abortActiveRequest();

    emit fetchProgress(QStringLiteral("正在获取世界杯赛程..."));
    fetchUrlAsync(QString::fromLatin1(kScheduleApiUrl), QString::fromLatin1(kPageUrl),
                  [this](const QByteArray &data, const QString &error) {
        if (!error.isEmpty()) {
            finishFetch();
            emit fetchFailed(error);
            return;
        }
        parseScheduleResponse(data);
    });
}

void MatchScraper::fetchUrlAsync(const QString &url, const QString &referer,
                                 const std::function<void(const QByteArray &, const QString &)> &callback)
{
    if (!QSslSocket::supportsSsl()) {
        fetchViaCurlAsync(url, referer, callback);
        return;
    }

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"));
    request.setRawHeader("Referer", referer.toUtf8());

    m_activeReply = m_network->get(request);
    connect(m_activeReply, &QNetworkReply::finished, this, [this, callback]() {
        QNetworkReply *reply = m_activeReply;
        if (!reply)
            return;

        reply->deleteLater();
        m_activeReply = nullptr;

        if (reply->error() != QNetworkReply::NoError) {
            QString error = reply->errorString();
            if (error.contains(QStringLiteral("TLS"), Qt::CaseInsensitive)
                || error.contains(QStringLiteral("SSL"), Qt::CaseInsensitive)) {
                error += QStringLiteral("\n\n") + sslErrorHint();
            }
            callback(QByteArray(), error);
            return;
        }

        callback(reply->readAll(), QString());
    });
}

void MatchScraper::fetchViaCurlAsync(const QString &url, const QString &referer,
                                     const std::function<void(const QByteArray &, const QString &)> &callback)
{
    m_activeProcess = new QProcess(this);
    QProcess *process = m_activeProcess;

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, process, callback](int exitCode, QProcess::ExitStatus status) {
        if (process != m_activeProcess)
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

    connect(process, &QProcess::errorOccurred, this, [this, process, callback](QProcess::ProcessError) {
        if (process != m_activeProcess)
            return;

        const QString error = process->errorString();
        process->deleteLater();
        m_activeProcess = nullptr;
        callback(QByteArray(), QStringLiteral("curl 启动失败: %1\n\n%2").arg(error, sslErrorHint()));
    });

    process->start(QStringLiteral("curl.exe"), {
        QStringLiteral("-sL"),
        QStringLiteral("--max-time"),
        QStringLiteral("30"),
        QStringLiteral("-A"),
        QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"),
        QStringLiteral("-H"),
        QStringLiteral("Referer: ") + referer,
        url
    });
}

void MatchScraper::parseScheduleResponse(const QByteArray &bytes)
{
    m_dayGroups.clear();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        finishFetch();
        emit fetchFailed(QStringLiteral("JSON 解析失败: %1").arg(parseError.errorString()));
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

    startOddsFetch();
}

void MatchScraper::startOddsFetch()
{
    m_oddsTasks.clear();
    for (int dayIndex = 0; dayIndex < m_dayGroups.size(); ++dayIndex) {
        for (int matchIndex = 0; matchIndex < m_dayGroups[dayIndex].matches.size(); ++matchIndex) {
            MatchItem &match = m_dayGroups[dayIndex].matches[matchIndex];
            match.homeOdds.clear();
            match.awayOdds.clear();
            match.hasOdds = false;

            OddsTask task;
            task.dayIndex = dayIndex;
            task.matchIndex = matchIndex;
            m_oddsTasks.append(task);
        }
    }

    m_oddsFetched = 0;
    m_oddsWithData = 0;
    emit fetchProgress(QStringLiteral("赛程已获取，正在爬取固定奖金赔率（0/%1）...")
                           .arg(m_oddsTasks.size()));
    fetchNextOdds();
}

void MatchScraper::fetchNextOdds()
{
    if (m_oddsTasks.isEmpty()) {
        int totalMatches = 0;
        for (const MatchDayGroup &group : m_dayGroups)
            totalMatches += group.matchCount;

        const QString message = QStringLiteral("世界杯 %1 个日期，共 %2 场，已获取 %3 场赔率")
                                    .arg(m_dayGroups.size())
                                    .arg(totalMatches)
                                    .arg(m_oddsWithData);
        finishFetch();
        emit fetchFinished(true, message);
        return;
    }

    const OddsTask task = m_oddsTasks.takeFirst();
    const MatchItem &match = m_dayGroups[task.dayIndex].matches[task.matchIndex];
    const int totalCount = m_oddsFetched + m_oddsTasks.size() + 1;

    emit fetchProgress(QStringLiteral("正在获取赔率 %1 vs %2（%3/%4）...")
                           .arg(match.homeTeam)
                           .arg(match.awayTeam)
                           .arg(m_oddsFetched + 1)
                           .arg(totalCount));

    const QString url = fixedBonusUrl(match.matchId);
    fetchUrlAsync(url, QString::fromLatin1(kDetailReferer),
                  [this, task](const QByteArray &data, const QString &error) {
        if (!error.isEmpty()) {
            ++m_oddsFetched;
            fetchNextOdds();
            return;
        }

        parseOddsResponse(task.dayIndex, task.matchIndex, data);
        ++m_oddsFetched;
        fetchNextOdds();
    });
}

void MatchScraper::parseOddsResponse(int dayIndex, int matchIndex, const QByteArray &bytes)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return;

    const QJsonObject root = doc.object();
    if (root.value(QStringLiteral("errorCode")).toString() != QStringLiteral("0"))
        return;

    const QJsonObject value = root.value(QStringLiteral("value")).toObject();
    const QJsonObject oddsHistory = value.value(QStringLiteral("oddsHistory")).toObject();
    const QJsonArray hadList = oddsHistory.value(QStringLiteral("hadList")).toArray();
    if (hadList.isEmpty())
        return;

    const QJsonObject lastOdds = hadList.last().toObject();
    const QString homeWin = lastOdds.value(QStringLiteral("h")).toString().trimmed();
    const QString awayWin = lastOdds.value(QStringLiteral("a")).toString().trimmed();
    if (homeWin.isEmpty() || awayWin.isEmpty())
        return;

    MatchItem &match = m_dayGroups[dayIndex].matches[matchIndex];
    match.homeOdds = homeWin;
    match.awayOdds = awayWin;
    match.hasOdds = true;
    ++m_oddsWithData;
}
