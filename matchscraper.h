#ifndef MATCHSCRAPER_H
#define MATCHSCRAPER_H

#include <QObject>
#include <QString>
#include <QVector>

class QProcess;

struct MatchItem {
    QString homeTeam;
    QString awayTeam;
    QString matchNumStr;
    QString matchDate;
    QString matchTime;
    qint64 matchId = 0;
    QString homeOdds;
    QString awayOdds;
    bool hasOdds = false;
};

struct MatchDayGroup {
    QString businessDate;
    QString weekday;
    int matchCount = 0;
    QVector<MatchItem> matches;
};

class MatchScraper : public QObject
{
    Q_OBJECT

public:
    explicit MatchScraper(QObject *parent = nullptr);

    void fetch();
    bool isFetching() const { return m_fetching; }
    const QVector<MatchDayGroup> &dayGroups() const { return m_dayGroups; }

signals:
    void fetchFinished(bool success, const QString &message);
    void fetchFailed(const QString &errorMessage);
    void fetchProgress(const QString &message);

private:
    void parseScheduleResponse(const QByteArray &bytes);
    void fetchScheduleWithRetry(quint64 generation, int attempt);
    void fetchUrlAsync(const QString &url, const QString &referer, quint64 generation,
                       const std::function<void(const QByteArray &, const QString &)> &callback);
    void fetchViaCurlAsync(const QString &url, const QString &referer, quint64 generation,
                           const std::function<void(const QByteArray &, const QString &)> &callback);
    static QString sslErrorHint();
    void finishFetch();
    void abortActiveRequest();

    QProcess *m_activeProcess = nullptr;
    bool m_fetching = false;
    QVector<MatchDayGroup> m_dayGroups;
    int m_oddsWithData = 0;
    quint64 m_fetchGeneration = 0;
};

#endif // MATCHSCRAPER_H
