#include "mainwindow.h"
#include "betcalculator.h"
#include "gameview.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVector>

namespace {

void updateRowSideCombo(QComboBox *combo, const QString &leftName, const QString &rightName)
{
    const int index = combo->currentIndex();
    combo->blockSignals(true);
    combo->clear();
    if (leftName == QStringLiteral("-") || rightName == QStringLiteral("-")) {
        combo->addItem(QStringLiteral("投左队"));
        combo->addItem(QStringLiteral("投右队"));
    } else {
        combo->addItem(QStringLiteral("投 %1").arg(leftName));
        combo->addItem(QStringLiteral("投 %1").arg(rightName));
    }
    combo->setCurrentIndex(index >= 0 && index < combo->count() ? index : 0);
    combo->blockSignals(false);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameView)
    , m_scraper(new MatchScraper(this))
{
    ui->setupUi(this);

    QLabel *leftLabels[] = { ui->labelCountry1, ui->labelCountry2, ui->labelCountry3, ui->labelCountry4 };
    QLabel *rightLabels[] = { ui->labelCountry5, ui->labelCountry6, ui->labelCountry7, ui->labelCountry8 };
    QComboBox *rowSides[] = { ui->comboRowSide1, ui->comboRowSide2, ui->comboRowSide3, ui->comboRowSide4 };
    for (int row = 0; row < BetCalculator::kMatchCount; ++row)
        updateRowSideCombo(rowSides[row], leftLabels[row]->text(), rightLabels[row]->text());

    connect(ui->btnCalculate, &QPushButton::clicked, this, &MainWindow::onCalculateClicked);
    connect(ui->btnUpdate, &QPushButton::clicked, this, &MainWindow::onUpdateClicked);
    connect(ui->comboPickDate, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDateChanged);
    connect(ui->comboPickMatch, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPickMatchChanged);
    connect(ui->comboPickSide, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPickSideChanged);
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

void MainWindow::onPickMatchChanged(int index)
{
    if (index < 0 || index >= BetCalculator::kMatchCount)
        return;

    QCheckBox *checks[] = { ui->checkMatch1, ui->checkMatch2, ui->checkMatch3, ui->checkMatch4 };

    checks[index]->setChecked(true);
    applyPickSideToRow(index);
}

void MainWindow::onPickSideChanged(int index)
{
    Q_UNUSED(index)

    const int matchRow = ui->comboPickMatch->currentIndex();
    if (matchRow < 0 || matchRow >= BetCalculator::kMatchCount)
        return;

    applyPickSideToRow(matchRow);
}

void MainWindow::applyPickSideToRow(int row)
{
    if (row < 0 || row >= BetCalculator::kMatchCount)
        return;

    QLineEdit *oddsEdits[] = {
        ui->editOdds1, ui->editOdds2, ui->editOdds3, ui->editOdds4,
        ui->editOdds5, ui->editOdds6, ui->editOdds7, ui->editOdds8
    };
    QComboBox *rowSides[] = { ui->comboRowSide1, ui->comboRowSide2, ui->comboRowSide3, ui->comboRowSide4 };

    const int pickMode = ui->comboPickSide->currentIndex();
    if (pickMode == BetCalculator::PickLeft || pickMode == BetCalculator::PickRight) {
        rowSides[row]->setCurrentIndex(pickMode);
        return;
    }

    const int leftIndex = row;
    const int rightIndex = row + BetCalculator::kMatchCount;
    const double leftOdds = BetCalculator::parsePositiveDouble(oddsEdits[leftIndex]->text(),
                                                               BetCalculator::defaultOdds(leftIndex));
    const double rightOdds = BetCalculator::parsePositiveDouble(oddsEdits[rightIndex]->text(),
                                                                BetCalculator::defaultOdds(rightIndex));
    const bool pickLeft = BetCalculator::resolvePickLeft(leftOdds, rightOdds, pickMode);
    rowSides[row]->setCurrentIndex(pickLeft ? 0 : 1);
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
    QCheckBox *checks[] = { ui->checkMatch1, ui->checkMatch2, ui->checkMatch3, ui->checkMatch4 };
    QComboBox *rowSides[] = { ui->comboRowSide1, ui->comboRowSide2, ui->comboRowSide3, ui->comboRowSide4 };

    ui->comboPickMatch->blockSignals(true);
    ui->comboPickMatch->clear();

    for (int row = 0; row < BetCalculator::kMatchCount; ++row) {
        const int leftIndex = row;
        const int rightIndex = row + BetCalculator::kMatchCount;

        if (row < group.matches.size()) {
            const MatchItem &match = group.matches.at(row);
            countryLabels[leftIndex]->setText(match.homeTeam);
            countryLabels[rightIndex]->setText(match.awayTeam);
            updateRowSideCombo(rowSides[row], match.homeTeam, match.awayTeam);
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
            updateRowSideCombo(rowSides[row], QStringLiteral("-"), QStringLiteral("-"));
            oddsEdits[leftIndex]->clear();
            oddsEdits[rightIndex]->clear();
        }
    }

    for (int row = 0; row < BetCalculator::kMatchCount; ++row)
        checks[row]->setChecked(false);

    ui->comboPickMatch->blockSignals(false);
    if (ui->comboPickMatch->count() > 0) {
        ui->comboPickMatch->setCurrentIndex(0);
        applyPickSideToRow(0);
    }
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
    QCheckBox *checks[] = { ui->checkMatch1, ui->checkMatch2, ui->checkMatch3, ui->checkMatch4 };
    QComboBox *rowSides[] = { ui->comboRowSide1, ui->comboRowSide2, ui->comboRowSide3, ui->comboRowSide4 };

    const double budget = BetCalculator::parsePositiveDouble(ui->editMyAmount->text(),
                                                             BetCalculator::kDefaultAmount);
    QVector<BetMatchSelection> selections;

    for (int row = 0; row < BetCalculator::kMatchCount; ++row) {
        if (!checks[row]->isChecked())
            continue;

        const int leftIndex = row;
        const int rightIndex = row + BetCalculator::kMatchCount;
        const QString leftName = countryLabels[leftIndex]->text();
        const QString rightName = countryLabels[rightIndex]->text();

        if (leftName == QStringLiteral("-") || rightName == QStringLiteral("-")) {
            ui->textEditLog->setPlainText(QStringLiteral("第 %1 行无比赛数据，请取消勾选。").arg(row + 1));
            return;
        }

        if (oddsEdits[leftIndex]->text().trimmed().isEmpty()
            || oddsEdits[rightIndex]->text().trimmed().isEmpty()) {
            ui->textEditLog->setPlainText(
                QStringLiteral("%1 vs %2 暂无赔率，请取消勾选或换一场。").arg(leftName, rightName));
            return;
        }

        BetMatchSelection item;
        item.row = row;
        item.leftName = leftName;
        item.rightName = rightName;
        item.leftOdds = BetCalculator::parsePositiveDouble(oddsEdits[leftIndex]->text(),
                                                           BetCalculator::defaultOdds(leftIndex));
        item.rightOdds = BetCalculator::parsePositiveDouble(oddsEdits[rightIndex]->text(),
                                                            BetCalculator::defaultOdds(rightIndex));
        item.pickLeft = rowSides[row]->currentIndex() == 0;
        selections.append(item);
    }

    const BetCalculationResult result = BetCalculator::calculate(budget, selections);
    if (!result.success) {
        ui->textEditLog->setPlainText(result.errorMessage);
        return;
    }

    ui->textEditLog->setPlainText(result.logText);
    ui->statusbar->showMessage(result.statusSummary);
}
