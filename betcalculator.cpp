#include "betcalculator.h"

namespace {

const double kDefaultOdds[] = { 1.45, 2.10, 3.50, 4.00, 6.3, 6.00, 8.00, 10.0 };

BetCalculationResult singleMatchHedge(double budget, const BetMatchSelection &sel)
{
    BetCalculationResult result;

    const double leftOdds = sel.leftOdds;
    const double rightOdds = sel.rightOdds;
    const QString leftName = sel.leftName;
    const QString rightName = sel.rightName;
    const bool pickLeft = sel.pickLeft;

    const double pickOdds = pickLeft ? leftOdds : rightOdds;
    const double hedgeOdds = pickLeft ? rightOdds : leftOdds;
    const QString pickName = pickLeft ? leftName : rightName;
    const QString hedgeName = pickLeft ? rightName : leftName;

    const double hedgeStake = budget / hedgeOdds;
    const double pickStake = budget - hedgeStake;

    if (pickStake < 0.0) {
        result.errorMessage = QStringLiteral("参数无效：对冲金额超过总成本，请检查赔率。");
        return result;
    }

    const double leftStake = pickLeft ? pickStake : hedgeStake;
    const double rightStake = pickLeft ? hedgeStake : pickStake;
    const double pickWinReturn = pickStake * pickOdds;
    const double pickWinProfit = pickWinReturn - budget;
    const double hedgeReturn = hedgeStake * hedgeOdds;

    QString log;
    log += QStringLiteral("========== 单场保本对冲（不含平局） ==========\n\n");
    log += QStringLiteral("总成本: %1\n").arg(budget, 0, 'f', 2);
    log += QStringLiteral("选择比赛: %1 vs %2\n").arg(leftName, rightName);
    log += QStringLiteral("主投方向: %1（赔率 %2）\n").arg(pickName).arg(pickOdds, 0, 'f', 2);
    log += QStringLiteral("左队 %1 赔率 %2    右队 %3 赔率 %4\n\n")
               .arg(leftName).arg(leftOdds, 0, 'f', 2)
               .arg(rightName).arg(rightOdds, 0, 'f', 2);

    log += QStringLiteral("【本场左右分配】\n");
    log += QStringLiteral("  左边(%1) 投: %2 元\n").arg(leftName).arg(leftStake, 0, 'f', 2);
    log += QStringLiteral("  右边(%1) 投: %2 元\n\n").arg(rightName).arg(rightStake, 0, 'f', 2);

    log += QStringLiteral("【下注明细】\n");
    log += QStringLiteral("  %1  投 %2  [%3]\n")
               .arg(leftName, -8).arg(leftStake, 0, 'f', 2)
               .arg(pickLeft ? QStringLiteral("主投") : QStringLiteral("对冲"));
    log += QStringLiteral("  %1  投 %2  [%3]\n")
               .arg(rightName, -8).arg(rightStake, 0, 'f', 2)
               .arg(pickLeft ? QStringLiteral("对冲") : QStringLiteral("主投"));

    log += QStringLiteral("\n========== 结论 ==========\n");
    log += QStringLiteral("若 %1 胜：回报 %2，盈利 %3（博取机会）\n")
               .arg(pickName).arg(pickWinReturn, 0, 'f', 2).arg(pickWinProfit, 0, 'f', 2);
    log += QStringLiteral("若 %1 胜：回报 %2，盈亏 %3（保本）\n")
               .arg(hedgeName).arg(hedgeReturn, 0, 'f', 2).arg(hedgeReturn - budget, 0, 'f', 2);

    result.success = true;
    result.logText = log;
    result.statusSummary = QStringLiteral("单场 | %1 vs %2 | 左 %3 / 右 %4")
                             .arg(leftName, rightName)
                             .arg(leftStake, 0, 'f', 2)
                             .arg(rightStake, 0, 'f', 2);
    return result;
}

BetCalculationResult parlayAnalysis(double budget, const QVector<BetMatchSelection> &selections)
{
    BetCalculationResult result;

    double combinedOdds = 1.0;
    QString oddsFormula;
    QString log;
    log += QStringLiteral("========== %1串1 串关分析 ==========\n\n").arg(selections.size());
    log += QStringLiteral("总投入: %1 元\n").arg(budget, 0, 'f', 2);
    log += QStringLiteral("选中场次: %1\n\n").arg(selections.size());
    log += QStringLiteral("【串关明细】\n");

    for (int i = 0; i < selections.size(); ++i) {
        const BetMatchSelection &sel = selections.at(i);
        const QString pickName = sel.pickLeft ? sel.leftName : sel.rightName;
        const double pickOdds = sel.pickLeft ? sel.leftOdds : sel.rightOdds;

        combinedOdds *= pickOdds;
        if (i > 0)
            oddsFormula += QStringLiteral(" × ");
        oddsFormula += QString::number(pickOdds, 'f', 2);

        log += QStringLiteral("  %1. %2 vs %3 → 投 %4 @ %5\n")
                   .arg(i + 1)
                   .arg(sel.leftName, sel.rightName, pickName)
                   .arg(pickOdds, 0, 'f', 2);
    }

    const double potentialReturn = budget * combinedOdds;
    const double netProfit = potentialReturn - budget;
    const double breakEvenRate = combinedOdds > 0.0 ? (100.0 / combinedOdds) : 0.0;

    log += QStringLiteral("\n【赔率计算】\n");
    log += QStringLiteral("  综合赔率 = %1 = %2\n")
               .arg(oddsFormula)
               .arg(combinedOdds, 0, 'f', 4);
    log += QStringLiteral("  潜在回报 = %1 × %2 = %3 元\n")
               .arg(budget, 0, 'f', 2)
               .arg(combinedOdds, 0, 'f', 4)
               .arg(potentialReturn, 0, 'f', 2);
    log += QStringLiteral("  净盈利(全中) = %1 元\n").arg(netProfit, 0, 'f', 2);
    log += QStringLiteral("  参考胜率(保本) ≈ %1%（赔率隐含，仅供参考）\n")
               .arg(breakEvenRate, 0, 'f', 2);

    log += QStringLiteral("\n========== 结论 ==========\n");
    log += QStringLiteral("%1 场比赛必须全部猜中，方可获得 %2 元回报；\n")
               .arg(selections.size())
               .arg(potentialReturn, 0, 'f', 2);
    log += QStringLiteral("任意一场未中则串关失败，本金 %1 元全部损失。\n").arg(budget, 0, 'f', 2);
    log += QStringLiteral("串关风险随场次增加而显著上升，请谨慎投注。");

    result.success = true;
    result.logText = log;
    result.statusSummary = QStringLiteral("%1串1 | 综合赔率 %2 | 潜在回报 %3")
                             .arg(selections.size())
                             .arg(combinedOdds, 0, 'f', 2)
                             .arg(potentialReturn, 0, 'f', 2);
    return result;
}

} // namespace

double BetCalculator::defaultOdds(int index)
{
    if (index < 0 || index >= static_cast<int>(sizeof(kDefaultOdds) / sizeof(kDefaultOdds[0])))
        return 1.0;
    return kDefaultOdds[index];
}

double BetCalculator::parsePositiveDouble(const QString &text, double defaultValue)
{
    if (text.trimmed().isEmpty())
        return defaultValue;

    bool ok = false;
    const double value = text.trimmed().toDouble(&ok);
    if (!ok || value <= 0.0)
        return defaultValue;
    return value;
}

bool BetCalculator::resolvePickLeft(double leftOdds, double rightOdds, int pickSideMode)
{
    const bool leftIsStrong = leftOdds <= rightOdds;
    if (pickSideMode == PickWeak)
        return !leftIsStrong;
    return leftIsStrong;
}

BetCalculationResult BetCalculator::calculate(double budget, const QVector<BetMatchSelection> &selections)
{
    BetCalculationResult result;
    if (selections.isEmpty()) {
        result.errorMessage = QStringLiteral("请至少勾选一场比赛参与计算。");
        return result;
    }

    if (selections.size() == 1)
        return singleMatchHedge(budget, selections.first());

    return parlayAnalysis(budget, selections);
}
