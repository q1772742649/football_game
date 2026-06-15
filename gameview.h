#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GameView
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayoutFetch;
    QLabel *labelPickDate;
    QComboBox *comboPickDate;
    QPushButton *btnUpdate;
    QGroupBox *groupBoxTeams;
    QGridLayout *gridLayoutTeams;
    QLabel *labelHeaderCountry;
    QLabel *labelHeaderOdds;
    QLabel *labelHeaderCountry2;
    QLabel *labelHeaderOdds2;
    QLabel *labelCountry1;
    QLineEdit *editOdds1;
    QLabel *labelCountry2;
    QLineEdit *editOdds2;
    QLabel *labelCountry3;
    QLineEdit *editOdds3;
    QLabel *labelCountry4;
    QLineEdit *editOdds4;
    QLabel *labelCountry5;
    QLineEdit *editOdds5;
    QLabel *labelCountry6;
    QLineEdit *editOdds6;
    QLabel *labelCountry7;
    QLineEdit *editOdds7;
    QLabel *labelCountry8;
    QLineEdit *editOdds8;
    QHBoxLayout *horizontalLayoutAction;
    QLabel *labelPickMatch;
    QComboBox *comboPickMatch;
    QLabel *labelPickSide;
    QComboBox *comboPickSide;
    QLabel *labelMyAmount;
    QLineEdit *editMyAmount;
    QPushButton *btnCalculate;
    QGroupBox *groupBoxLog;
    QTextEdit *textEditLog;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow);
    void retranslateUi(QMainWindow *MainWindow);
};

namespace Ui {
    class GameView : public Ui_GameView {};
}

QT_END_NAMESPACE

#endif // GAMEVIEW_H
