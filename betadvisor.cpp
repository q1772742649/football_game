#include "betadvisor.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

namespace {

const char kApiUrl[] = "https://api.deepseek.com/chat/completions";
const char kModel[] = "deepseek-v4-flash";
const char kKeyFileName[] = "deepseek_api_key.txt";

QString curlExecutablePath()
{
    const QString systemCurl = QStringLiteral("C:/Windows/System32/curl.exe");
    if (QFile::exists(systemCurl))
        return systemCurl;
    return QStringLiteral("curl.exe");
}

QString pickTeamName(const BetMatchSelection &item)
{
    return item.pickLeft ? item.leftName : item.rightName;
}

double pickOddsValue(const BetMatchSelection &item)
{
    return item.pickLeft ? item.leftOdds : item.rightOdds;
}

} // namespace

BetAdvisor::BetAdvisor(QObject *parent)
    : QObject(parent)
{
}

QString BetAdvisor::apiKeyHint()
{
    const QString keyPath = QCoreApplication::applicationDirPath() + QLatin1Char('/') + QLatin1String(kKeyFileName);
    return QStringLiteral(
        "请在程序同目录创建文件 deepseek_api_key.txt，\n"
        "写入一行 DeepSeek API Key（sk- 开头），或设置环境变量 DEEPSEEK_API_KEY。\n"
        "Key 获取：https://platform.deepseek.com/\n"
        "示例路径：%1")
        .arg(keyPath);
}

QString BetAdvisor::loadApiKey()
{
    const QByteArray envKey = qgetenv("DEEPSEEK_API_KEY");
    if (!envKey.isEmpty())
        return QString::fromUtf8(envKey).trimmed();

    QStringList searchDirs;
    searchDirs << QCoreApplication::applicationDirPath();
    searchDirs << QDir::currentPath();

    for (const QString &dir : searchDirs) {
        const QString keyPath = QDir(dir).filePath(QLatin1String(kKeyFileName));
        QFile file(keyPath);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        const QString content = QString::fromUtf8(file.readAll());
        const QStringList lines = content.split(QLatin1Char('\n'));
        for (QString line : lines) {
            line = line.trimmed();
            if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
                continue;
            return line;
        }
    }
    return QString();
}

QString BetAdvisor::buildUserPrompt(const BetAnalysisContext &context)
{
    QString prompt;
    prompt += QStringLiteral("【投入金额】%1 元\n").arg(context.budget, 0, 'f', 2);
    prompt += QStringLiteral("【主投策略】%1\n").arg(context.pickSideLabel);
    prompt += QStringLiteral("【已选场次】%1 场\n\n").arg(context.selections.size());

    for (int i = 0; i < context.selections.size(); ++i) {
        const BetMatchSelection &item = context.selections.at(i);
        const QString pickName = pickTeamName(item);
        const double pickOdds = pickOddsValue(item);
        prompt += QStringLiteral("%1. %2 vs %3 | 左 %4(%5) 右 %6(%7) → 当前选择投 %8 @ %9\n")
                      .arg(i + 1)
                      .arg(item.leftName, item.rightName)
                      .arg(item.leftOdds, 0, 'f', 2)
                      .arg(item.leftName)
                      .arg(item.rightOdds, 0, 'f', 2)
                      .arg(item.rightName)
                      .arg(pickName)
                      .arg(pickOdds, 0, 'f', 2);
    }

    if (!context.calculationSummary.isEmpty()) {
        prompt += QStringLiteral("\n【程序已算出的数学结果（请以此为准核对数字）】\n");
        prompt += context.calculationSummary;
    }

    prompt += QStringLiteral(
        "\n请用中文分点分析：\n"
        "1. 串关/单场风险（是否必须全中、本金是否可能全失）\n"
        "2. 各场强弱与所选方向是否合理\n"
        "3. 综合赔率与回报的大致水平\n"
        "4. 更保守或更激进的替代思路（如有）\n"
        "5. 明确声明：仅供参考，不构成投注建议\n"
        "不要编造未提供的比分、新闻或战绩。");

    return prompt;
}

void BetAdvisor::abortRequest()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void BetAdvisor::finishAnalyze()
{
    m_analyzing = false;
    m_process = nullptr;
}

void BetAdvisor::analyze(const BetAnalysisContext &context)
{
    if (m_analyzing)
        return;

    if (context.selections.isEmpty()) {
        emit analyzeFailed(QStringLiteral("请至少勾选一场比赛后再进行 AI 分析。"));
        return;
    }

    const QString apiKey = loadApiKey();
    if (apiKey.isEmpty()) {
        emit analyzeFailed(apiKeyHint());
        return;
    }

    ++m_generation;
    const quint64 generation = m_generation;
    m_analyzing = true;
    abortRequest();

    emit analyzeProgress(QStringLiteral("正在请求 DeepSeek AI 分析..."));

    QJsonObject systemMessage;
    systemMessage.insert(QStringLiteral("role"), QStringLiteral("system"));
    systemMessage.insert(QStringLiteral("content"),
                         QStringLiteral("你是足球竞彩数据的风险分析助手，只做理性、克制的概率与资金风险说明，不提供确定性的赢钱承诺。"));

    QJsonObject userMessage;
    userMessage.insert(QStringLiteral("role"), QStringLiteral("user"));
    userMessage.insert(QStringLiteral("content"), buildUserPrompt(context));

    QJsonArray messages;
    messages.append(systemMessage);
    messages.append(userMessage);

    QJsonObject body;
    body.insert(QStringLiteral("model"), QString::fromLatin1(kModel));
    body.insert(QStringLiteral("messages"), messages);
    body.insert(QStringLiteral("max_tokens"), 1500);
    body.insert(QStringLiteral("temperature"), 0.4);

    QJsonObject thinking;
    thinking.insert(QStringLiteral("type"), QStringLiteral("disabled"));
    body.insert(QStringLiteral("thinking"), thinking);

    const QByteArray jsonBody = QJsonDocument(body).toJson(QJsonDocument::Compact);

    m_process = new QProcess(this);
    QProcess *process = m_process;

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, process, generation](int exitCode, QProcess::ExitStatus status) {
        if (process != m_process || generation != m_generation)
            return;

        const QByteArray output = process->readAllStandardOutput();
        const QByteArray stderrData = process->readAllStandardError();
        process->deleteLater();
        finishAnalyze();

        if (status != QProcess::NormalExit || exitCode != 0) {
            QString error = QString::fromLocal8Bit(stderrData.trimmed());
            if (error.isEmpty())
                error = QStringLiteral("curl 执行失败，退出码 %1").arg(exitCode);
            emit analyzeFailed(error);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(output, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            emit analyzeFailed(QStringLiteral("AI 响应 JSON 解析失败: %1").arg(parseError.errorString()));
            return;
        }

        const QJsonObject root = doc.object();
        if (root.contains(QStringLiteral("error"))) {
            const QJsonObject errObj = root.value(QStringLiteral("error")).toObject();
            emit analyzeFailed(errObj.value(QStringLiteral("message")).toString());
            return;
        }

        const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            emit analyzeFailed(QStringLiteral("AI 未返回有效内容。"));
            return;
        }

        const QJsonObject message = choices.first().toObject().value(QStringLiteral("message")).toObject();
        const QString content = message.value(QStringLiteral("content")).toString().trimmed();
        if (content.isEmpty()) {
            emit analyzeFailed(QStringLiteral("AI 返回内容为空。"));
            return;
        }

        emit analyzeFinished(content);
    });

    connect(process, &QProcess::errorOccurred, this, [this, process, generation](QProcess::ProcessError) {
        if (process != m_process || generation != m_generation)
            return;

        const QString error = process->errorString();
        process->deleteLater();
        finishAnalyze();
        emit analyzeFailed(QStringLiteral("启动 curl 失败: %1").arg(error));
    });

    const QString authHeader = QStringLiteral("Authorization: Bearer ") + apiKey;

    process->start(curlExecutablePath(), {
        QStringLiteral("-sL"),
        QStringLiteral("--max-time"),
        QStringLiteral("120"),
        QStringLiteral("-X"),
        QStringLiteral("POST"),
        QStringLiteral("-H"),
        authHeader,
        QStringLiteral("-H"),
        QStringLiteral("Content-Type: application/json"),
        QStringLiteral("-d"),
        QString::fromUtf8(jsonBody),
        QString::fromLatin1(kApiUrl)
    });
}
