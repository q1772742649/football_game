#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "betadvisor.h"
#include "matchscraper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class GameView; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCalculateClicked();
    void onUpdateClicked();
    void onDateChanged(int index);
    void onPickMatchChanged(int index);
    void onPickSideChanged(int index);
    void onAiAnalyzeClicked();
    void onAnalyzeFinished(const QString &text);
    void onAnalyzeFailed(const QString &errorMessage);
    void onAnalyzeProgress(const QString &message);
    void onFetchFinished(bool success, const QString &message);
    void onFetchFailed(const QString &errorMessage);
    void onFetchProgress(const QString &message);

private:
    void applyDayToUi(int dayIndex);
    void applyPickSideToRow(int row);
    void applyPickSideToAllRows();
    bool collectSelections(QVector<BetMatchSelection> &selections, QString &errorMessage);

    Ui::GameView *ui;
    MatchScraper *m_scraper;
    BetAdvisor *m_advisor;
};
#endif // MAINWINDOW_H
