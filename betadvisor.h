#ifndef BETADVISOR_H
#define BETADVISOR_H

#include <QObject>
#include <QString>
#include <QVector>

#include "betcalculator.h"

class QProcess;

struct BetAnalysisContext {
    double budget = 0.0;
    QString pickSideLabel;
    QVector<BetMatchSelection> selections;
    QString calculationSummary;
};

class BetAdvisor : public QObject
{
    Q_OBJECT

public:
    explicit BetAdvisor(QObject *parent = nullptr);

    void analyze(const BetAnalysisContext &context);
    bool isAnalyzing() const { return m_analyzing; }
    static QString apiKeyHint();

signals:
    void analyzeFinished(const QString &text);
    void analyzeFailed(const QString &errorMessage);
    void analyzeProgress(const QString &message);

private:
    static QString loadApiKey();
    static QString buildUserPrompt(const BetAnalysisContext &context);
    void finishAnalyze();
    void abortRequest();

    QProcess *m_process = nullptr;
    bool m_analyzing = false;
    quint64 m_generation = 0;
};

#endif // BETADVISOR_H
