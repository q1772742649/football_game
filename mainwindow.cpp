#include "mainwindow.h"
#include "gameview.h"

#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVector>

namespace {

const double kDefaultOdds[] = { 1.45, 2.10, 3.50, 4.00, 6.3, 6.00, 8.00, 10.0 };
const double kDefaultAmount = 100.0;
const int kMatchCount = 4;

double parsePositiveDouble(const QLineEdit *edit, double defaultValue)
{
    if (!edit || edit->text().trimmed().isEmpty())
        return defaultValue;

    bool ok = false;
    const double value = edit->text().trimmed().toDouble(&ok);
    if (!ok || value <= 0.0)
        return defaultValue;
    return value;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameView)
    , m_scraper(new MatchScraper(this))
{
    ui->setupUi(this);

    connect(ui->btnCalculate, &QPushButton::clicked, this, &MainWindow::onCalculateClicked);
    connect(ui->btnUpdate, &QPushButton::clicked, this, &MainWindow::onUpdateClicked);
    connect(ui->comboPickDate, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDateChanged);
    connect(m_scraper, &MatchScraper::fetchFinished, this, &MainWindow::onFetchFinished);
    connect(m_scraper, &MatchScraper::fetchFailed, this, &MainWindow::onFetchFailed);
    connect(m_scraper, &MatchScraper::fetchProgress, this, &MainWindow::onFetchProgress);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onUpdateClicked()
{
    if (m_scraper->isFetching()) {
        ui->statusbar->showMessage(QStringLiteral("正在更新中，请稍候..."));
        return;
    }

    ui->btnUpdate->setEnabled(false);
    ui->statusbar->showMessage(QStringLiteral("正在从竞彩网获取赛程..."));
    m_scraper->fetch();
}

void MainWindow::onFetchFinished(bool success, const QString &message)
{
    ui->btnUpdate->setEnabled(true);
    if (!success)
        return;

    ui->comboPickDate->blockSignals(true);
    ui->comboPickDate->clear();
    const QVector<MatchDayGroup> &groups = m_scraper->dayGroups();
    for (const MatchDayGroup &group : groups) {
        const QString text = QStringLiteral("%1 %2 (%3场)")
                                 .arg(group.weekday)
                                 .arg(group.businessDate)
                                 .arg(group.matchCount);
        ui->comboPickDate->addItem(text, group.businessDate);
    }
    ui->comboPickDate->blockSignals(false);

    if (!groups.isEmpty()) {
        ui->comboPickDate->setCurrentIndex(0);
        applyDayToUi(0);
    } else {
        ui->comboPickMatch->clear();
    }

    ui->statusbar->showMessage(message);
    ui->textEditLog->append(QStringLiteral("[更新] %1\n").arg(message));
}

void MainWindow::onFetchFailed(const QString &errorMessage)
{
    ui->btnUpdate->setEnabled(true);
    ui->statusbar->showMessage(QStringLiteral("更新失败"));
    ui->textEditLog->append(QStringLiteral("[错误] %1\n").arg(errorMessage));
}

void MainWindow::onFetchProgress(const QString &message)
{
    ui->statusbar->showMessage(message);
}

void MainWindow::onDateChanged(int index)
{
    if (index < 0)
        return;
    applyDayToUi(index);
}

void MainWindow::applyDayToUi(int dayIndex)
{
    const QVector<MatchDayGroup> &groups = m_scraper->dayGroups();
    if (dayIndex < 0 || dayIndex >= groups.size())
        return;

    const MatchDayGroup &group = groups.at(dayIndex);

    QLabel *countryLabels[] = {
        ui->labelCountry1, ui->labelCountry2, ui->labelCountry3, ui->labelCountry4,
        ui->labelCountry5, ui->labelCountry6, ui->labelCountry7, ui->labelCountry8
    };
    QLineEdit *oddsEdits[] = {
        ui->editOdds1, ui->editOdds2, ui->editOdds3, ui->editOdds4,
        ui->editOdds5, ui->editOdds6, ui->editOdds7, ui->editOdds8
    };

    ui->comboPickMatch->blockSignals(true);
    ui->comboPickMatch->clear();

    for (int row = 0; row < kMatchCount; ++row) {
        const int leftIndex = row;
        const int rightIndex = row + kMatchCount;

        if (row < group.matches.size()) {
            const MatchItem &match = group.matches.at(row);
            countryLabels[leftIndex]->setText(match.homeTeam);
            countryLabels[rightIndex]->setText(match.awayTeam);
            if (match.hasOdds) {
                oddsEdits[leftIndex]->setText(match.homeOdds);
                oddsEdits[rightIndex]->setText(match.awayOdds);
            } else {
                oddsEdits[leftIndex]->clear();
                oddsEdits[rightIndex]->clear();
            }

            const QString matchText = QStringLiteral("%1 vs %2")
                                          .arg(match.homeTeam)
                                          .arg(match.awayTeam);
            ui->comboPickMatch->addItem(matchText);
        } else {
            countryLabels[leftIndex]->setText(QStringLiteral("-"));
            countryLabels[rightIndex]->setText(QStringLiteral("-"));
            oddsEdits[leftIndex]->clear();
            oddsEdits[rightIndex]->clear();
        }
    }

    ui->comboPickMatch->blockSignals(false);
    if (ui->comboPickMatch->count() > 0)
        ui->comboPickMatch->setCurrentIndex(0);
}

void MainWindow::onCalculateClicked()
{
    const QLineEdit *oddsEdits[] = {
        ui->editOdds1, ui->editOdds2, ui->editOdds3, ui->editOdds4,
        ui->editOdds5, ui->editOdds6, ui->editOdds7, ui->editOdds8
    };
    const QLabel *countryLabels[] = {
        ui->labelCountry1, ui->labelCountry2, ui->labelCountry3, ui->labelCountry4,
        ui->labelCountry5, ui->labelCountry6, ui->labelCountry7, ui->labelCountry8
    };

    const double budget = parsePositiveDouble(ui->editMyAmount, kDefaultAmount);
    const int matchRow = ui->comboPickMatch->currentIndex();
    const bool pickLeft = ui->comboPickSide->currentIndex() == 0;

    if (matchRow < 0) {
        ui->textEditLog->setPlainText(QStringLiteral("请先选择比赛。"));
        return;
    }

    const int leftIndex = matchRow;
    const int rightIndex = matchRow + kMatchCount;

    const double leftOdds = parsePositiveDouble(oddsEdits[leftIndex], kDefaultOdds[leftIndex]);
    const double rightOdds = parsePositiveDouble(oddsEdits[rightIndex], kDefaultOdds[rightIndex]);

    const QString leftName = countryLabels[leftIndex]->text();
    const QString rightName = countryLabels[rightIndex]->text();

    if (leftName == QStringLiteral("-") || rightName == QStringLiteral("-")) {
        ui->textEditLog->setPlainText(QStringLiteral("当前日期该场次无比赛数据。"));
        return;
    }

    if (oddsEdits[leftIndex]->text().trimmed().isEmpty()
        || oddsEdits[rightIndex]->text().trimmed().isEmpty()) {
        ui->textEditLog->setPlainText(QStringLiteral("该场暂无胜平负固定奖金数据，请换一场或稍后再试。"));
        return;
    }

    const double pickOdds = pickLeft ? leftOdds : rightOdds;
    const double hedgeOdds = pickLeft ? rightOdds : leftOdds;
    const QString pickName = pickLeft ? leftName : rightName;
    const QString hedgeName = pickLeft ? rightName : leftName;

    const double hedgeStake = budget / hedgeOdds;
    const double pickStake = budget - hedgeStake;

    if (pickStake < 0.0) {
        ui->textEditLog->setPlainText(QStringLiteral("参数无效：对冲金额超过总成本，请检查赔率。"));
        return;
    }

    const double leftStake = pickLeft ? pickStake : hedgeStake;
    const double rightStake = pickLeft ? hedgeStake : pickStake;

    const double pickWinReturn = pickStake * pickOdds;
    const double pickWinProfit = pickWinReturn - budget;
    const double hedgeReturn = hedgeStake * hedgeOdds;

    QString log;
    log += QStringLiteral("========== 单场保本对冲（不含平局） ==========\n\n");
    log += QStringLiteral("总成本: %1\n").arg(budget, 0, 'f', 2);
    log += QStringLiteral("选择比赛: %1 vs %2\n")
               .arg(leftName)
               .arg(rightName);
    log += QStringLiteral("主投方向: %1（赔率 %2）\n")
               .arg(pickName)
               .arg(pickOdds, 0, 'f', 2);
    log += QStringLiteral("左队 %1 赔率 %2    右队 %3 赔率 %4\n\n")
               .arg(leftName)
               .arg(leftOdds, 0, 'f', 2)
               .arg(rightName)
               .arg(rightOdds, 0, 'f', 2);

    log += QStringLiteral("【本场左右分配】\n");
    log += QStringLiteral("  左边(%1) 投: %2 元\n").arg(leftName).arg(leftStake, 0, 'f', 2);
    log += QStringLiteral("  右边(%1) 投: %2 元\n\n").arg(rightName).arg(rightStake, 0, 'f', 2);

    log += QStringLiteral("【下注明细】\n");
    log += QStringLiteral("  %1  投 %2  [%3]\n")
               .arg(leftName, -8)
               .arg(leftStake, 0, 'f', 2)
               .arg(pickLeft ? QStringLiteral("主投") : QStringLiteral("对冲"));
    log += QStringLiteral("  %1  投 %2  [%3]\n")
               .arg(rightName, -8)
               .arg(rightStake, 0, 'f', 2)
               .arg(pickLeft ? QStringLiteral("对冲") : QStringLiteral("主投"));

    log += QStringLiteral("\n========== 结论 ==========\n");
    log += QStringLiteral("本场 %1 vs %2，在 %3 元内：\n")
               .arg(leftName)
               .arg(rightName)
               .arg(budget, 0, 'f', 0);
    log += QStringLiteral("  左边(%1) 投 %2 元\n")
               .arg(leftName)
               .arg(leftStake, 0, 'f', 2);
    log += QStringLiteral("  右边(%1) 投 %2 元\n")
               .arg(rightName)
               .arg(rightStake, 0, 'f', 2);
    log += QStringLiteral("\n若 %1 胜：回报 %2，盈利 %3（博取机会）\n")
               .arg(pickName)
               .arg(pickWinReturn, 0, 'f', 2)
               .arg(pickWinProfit, 0, 'f', 2);
    log += QStringLiteral("若 %1 胜：回报 %2，盈亏 %3（保本）\n")
               .arg(hedgeName)
               .arg(hedgeReturn, 0, 'f', 2)
               .arg(hedgeReturn - budget, 0, 'f', 2);

    ui->textEditLog->setPlainText(log);
    ui->statusbar->showMessage(QStringLiteral("%1 vs %2 | 左 %3 / 右 %4")
                                   .arg(leftName)
                                   .arg(rightName)
                                   .arg(leftStake, 0, 'f', 2)
                                   .arg(rightStake, 0, 'f', 2));
}
