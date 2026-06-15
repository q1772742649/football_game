#ifndef MATCHSCRAPER_H
#define MATCHSCRAPER_H

#include <QObject>
#include <QString>
#include <QVector>

class QNetworkAccessManager;
class QNetworkReply;
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
    struct OddsTask {
        int dayIndex = 0;
        int matchIndex = 0;
    };

    void parseScheduleResponse(const QByteArray &bytes);
    void startOddsFetch();
    void fetchNextOdds();
    void parseOddsResponse(int dayIndex, int matchIndex, const QByteArray &bytes);
    void fetchUrlAsync(const QString &url, const QString &referer,
                       const std::function<void(const QByteArray &, const QString &)> &callback);
    void fetchViaCurlAsync(const QString &url, const QString &referer,
                           const std::function<void(const QByteArray &, const QString &)> &callback);
    static QString sslErrorHint();
    void finishFetch();
    void abortActiveRequest();

    QNetworkAccessManager *m_network = nullptr;
    QNetworkReply *m_activeReply = nullptr;
    QProcess *m_activeProcess = nullptr;
    bool m_fetching = false;
    QVector<MatchDayGroup> m_dayGroups;
    QVector<OddsTask> m_oddsTasks;
    int m_oddsFetched = 0;
    int m_oddsWithData = 0;
};

#endif // MATCHSCRAPER_H
