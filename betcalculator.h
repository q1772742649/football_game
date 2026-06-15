#ifndef BETCALCULATOR_H
#define BETCALCULATOR_H

#include <QString>
#include <QVector>

struct BetMatchSelection {
    int row = 0;
    QString leftName;
    QString rightName;
    double leftOdds = 0.0;
    double rightOdds = 0.0;
    bool pickLeft = true;
};

struct BetCalculationResult {
    bool success = false;
    QString errorMessage;
    QString logText;
    QString statusSummary;
};

class BetCalculator
{
public:
    static constexpr int kMatchCount = 4;
    static constexpr double kDefaultAmount = 100.0;

    enum PickSideMode {
        PickStrong = 0, // 赔率低的一方
        PickWeak = 1    // 赔率高的一方
    };

    static double defaultOdds(int index);
    static double parsePositiveDouble(const QString &text, double defaultValue);
    static bool resolvePickLeft(double leftOdds, double rightOdds, int pickSideMode);
    static BetCalculationResult calculate(double budget, const QVector<BetMatchSelection> &selections);
};

#endif // BETCALCULATOR_H
