#include "gameview.h"

#include <QtWidgets/QAction>

QT_BEGIN_NAMESPACE

void Ui_GameView::setupUi(QMainWindow *MainWindow)
{
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(1080, 960);
    MainWindow->setStyleSheet(QString::fromUtf8(
        "QMainWindow { background-color: #F0F0F0; font-size: 14px; }"
        "QGroupBox {"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  border: 1px solid #CCCCCC;"
        "  border-radius: 2px;"
        "  margin-top: 8px;"
        "  padding-top: 12px;"
        "  background-color: #FFFFFF;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 8px;"
        "  padding: 0 4px;"
        "}"
        "QLabel { color: #1E1E1E; font-size: 14px; }"
        "QLineEdit {"
        "  font-size: 14px;"
        "  border: 1px solid #ABABAB;"
        "  border-radius: 2px;"
        "  padding: 3px 6px;"
        "  background-color: #FFFFFF;"
        "  min-height: 26px;"
        "}"
        "QLineEdit:focus { border: 1px solid #007ACC; }"
        "QComboBox {"
        "  font-size: 14px;"
        "  border: 1px solid #ABABAB;"
        "  border-radius: 2px;"
        "  padding: 3px 6px;"
        "  min-height: 26px;"
        "}"
        "QPushButton {"
        "  font-size: 14px;"
        "  border: 1px solid #ABABAB;"
        "  border-radius: 2px;"
        "  padding: 5px 18px;"
        "  background-color: #E1E1E1;"
        "  min-height: 28px;"
        "}"
        "QPushButton:hover { background-color: #E5F3FF; border: 1px solid #007ACC; }"
        "QPushButton:pressed { background-color: #CCE4F7; }"
        "QTextEdit {"
        "  border: 1px solid #ABABAB;"
        "  border-radius: 2px;"
        "  background-color: #FFFFFF;"
        "  font-family: Consolas, 'Courier New', monospace;"
        "  font-size: 14px;"
        "}"
        "QMenuBar {"
        "  font-size: 14px;"
        "  background-color: #F0F0F0;"
        "  border-bottom: 1px solid #CCCCCC;"
        "}"
        "QMenuBar::item:selected { background-color: #E5F3FF; }"
        "QStatusBar {"
        "  font-size: 14px;"
        "  background-color: #007ACC;"
        "  color: #FFFFFF;"
        "}"
    ));

    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    verticalLayout = new QVBoxLayout(centralwidget);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(8, 8, 8, 8);
    verticalLayout->setSpacing(8);

    horizontalLayoutFetch = new QHBoxLayout();
    horizontalLayoutFetch->setObjectName(QString::fromUtf8("horizontalLayoutFetch"));
    horizontalLayoutFetch->setSpacing(8);

    labelPickDate = new QLabel(centralwidget);
    labelPickDate->setObjectName(QString::fromUtf8("labelPickDate"));
    horizontalLayoutFetch->addWidget(labelPickDate);

    comboPickDate = new QComboBox(centralwidget);
    comboPickDate->setObjectName(QString::fromUtf8("comboPickDate"));
    comboPickDate->setMinimumWidth(180);
    horizontalLayoutFetch->addWidget(comboPickDate);

    btnUpdate = new QPushButton(centralwidget);
    btnUpdate->setObjectName(QString::fromUtf8("btnUpdate"));
    horizontalLayoutFetch->addWidget(btnUpdate);

    horizontalLayoutFetch->addStretch();

    verticalLayout->addLayout(horizontalLayoutFetch);

    groupBoxTeams = new QGroupBox(centralwidget);
    groupBoxTeams->setObjectName(QString::fromUtf8("groupBoxTeams"));
    gridLayoutTeams = new QGridLayout(groupBoxTeams);
    gridLayoutTeams->setObjectName(QString::fromUtf8("gridLayoutTeams"));
    gridLayoutTeams->setHorizontalSpacing(12);
    gridLayoutTeams->setVerticalSpacing(6);
    gridLayoutTeams->setContentsMargins(12, 16, 12, 12);

    labelHeaderCountry = new QLabel(groupBoxTeams);
    labelHeaderCountry->setObjectName(QString::fromUtf8("labelHeaderCountry"));
    labelHeaderCountry->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #007ACC;"));
    gridLayoutTeams->addWidget(labelHeaderCountry, 0, 0);

    labelHeaderOdds = new QLabel(groupBoxTeams);
    labelHeaderOdds->setObjectName(QString::fromUtf8("labelHeaderOdds"));
    labelHeaderOdds->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #007ACC;"));
    gridLayoutTeams->addWidget(labelHeaderOdds, 0, 1);

    labelHeaderCountry2 = new QLabel(groupBoxTeams);
    labelHeaderCountry2->setObjectName(QString::fromUtf8("labelHeaderCountry2"));
    labelHeaderCountry2->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #007ACC;"));
    gridLayoutTeams->addWidget(labelHeaderCountry2, 0, 2);

    labelHeaderOdds2 = new QLabel(groupBoxTeams);
    labelHeaderOdds2->setObjectName(QString::fromUtf8("labelHeaderOdds2"));
    labelHeaderOdds2->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #007ACC;"));
    gridLayoutTeams->addWidget(labelHeaderOdds2, 0, 3);

    labelCountry1 = new QLabel(groupBoxTeams);
    labelCountry1->setObjectName(QString::fromUtf8("labelCountry1"));
    gridLayoutTeams->addWidget(labelCountry1, 1, 0);
    editOdds1 = new QLineEdit(groupBoxTeams);
    editOdds1->setObjectName(QString::fromUtf8("editOdds1"));
    editOdds1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds1, 1, 1);

    labelCountry2 = new QLabel(groupBoxTeams);
    labelCountry2->setObjectName(QString::fromUtf8("labelCountry2"));
    gridLayoutTeams->addWidget(labelCountry2, 2, 0);
    editOdds2 = new QLineEdit(groupBoxTeams);
    editOdds2->setObjectName(QString::fromUtf8("editOdds2"));
    editOdds2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds2, 2, 1);

    labelCountry3 = new QLabel(groupBoxTeams);
    labelCountry3->setObjectName(QString::fromUtf8("labelCountry3"));
    gridLayoutTeams->addWidget(labelCountry3, 3, 0);
    editOdds3 = new QLineEdit(groupBoxTeams);
    editOdds3->setObjectName(QString::fromUtf8("editOdds3"));
    editOdds3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds3, 3, 1);

    labelCountry4 = new QLabel(groupBoxTeams);
    labelCountry4->setObjectName(QString::fromUtf8("labelCountry4"));
    gridLayoutTeams->addWidget(labelCountry4, 4, 0);
    editOdds4 = new QLineEdit(groupBoxTeams);
    editOdds4->setObjectName(QString::fromUtf8("editOdds4"));
    editOdds4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds4, 4, 1);

    labelCountry5 = new QLabel(groupBoxTeams);
    labelCountry5->setObjectName(QString::fromUtf8("labelCountry5"));
    gridLayoutTeams->addWidget(labelCountry5, 1, 2);
    editOdds5 = new QLineEdit(groupBoxTeams);
    editOdds5->setObjectName(QString::fromUtf8("editOdds5"));
    editOdds5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds5, 1, 3);

    labelCountry6 = new QLabel(groupBoxTeams);
    labelCountry6->setObjectName(QString::fromUtf8("labelCountry6"));
    gridLayoutTeams->addWidget(labelCountry6, 2, 2);
    editOdds6 = new QLineEdit(groupBoxTeams);
    editOdds6->setObjectName(QString::fromUtf8("editOdds6"));
    editOdds6->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds6, 2, 3);

    labelCountry7 = new QLabel(groupBoxTeams);
    labelCountry7->setObjectName(QString::fromUtf8("labelCountry7"));
    gridLayoutTeams->addWidget(labelCountry7, 3, 2);
    editOdds7 = new QLineEdit(groupBoxTeams);
    editOdds7->setObjectName(QString::fromUtf8("editOdds7"));
    editOdds7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds7, 3, 3);

    labelCountry8 = new QLabel(groupBoxTeams);
    labelCountry8->setObjectName(QString::fromUtf8("labelCountry8"));
    gridLayoutTeams->addWidget(labelCountry8, 4, 2);
    editOdds8 = new QLineEdit(groupBoxTeams);
    editOdds8->setObjectName(QString::fromUtf8("editOdds8"));
    editOdds8->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayoutTeams->addWidget(editOdds8, 4, 3);

    gridLayoutTeams->setColumnStretch(0, 1);
    gridLayoutTeams->setColumnStretch(1, 1);
    gridLayoutTeams->setColumnStretch(2, 1);
    gridLayoutTeams->setColumnStretch(3, 1);
    gridLayoutTeams->setColumnMinimumWidth(1, 80);
    gridLayoutTeams->setColumnMinimumWidth(3, 80);

    verticalLayout->addWidget(groupBoxTeams);

    horizontalLayoutAction = new QHBoxLayout();
    horizontalLayoutAction->setObjectName(QString::fromUtf8("horizontalLayoutAction"));
    horizontalLayoutAction->setSpacing(8);

    labelPickMatch = new QLabel(centralwidget);
    labelPickMatch->setObjectName(QString::fromUtf8("labelPickMatch"));
    horizontalLayoutAction->addWidget(labelPickMatch);

    comboPickMatch = new QComboBox(centralwidget);
    comboPickMatch->setObjectName(QString::fromUtf8("comboPickMatch"));
    comboPickMatch->setMinimumWidth(140);
    horizontalLayoutAction->addWidget(comboPickMatch);

    labelPickSide = new QLabel(centralwidget);
    labelPickSide->setObjectName(QString::fromUtf8("labelPickSide"));
    horizontalLayoutAction->addWidget(labelPickSide);

    comboPickSide = new QComboBox(centralwidget);
    comboPickSide->setObjectName(QString::fromUtf8("comboPickSide"));
    comboPickSide->setMinimumWidth(100);
    horizontalLayoutAction->addWidget(comboPickSide);

    labelMyAmount = new QLabel(centralwidget);
    labelMyAmount->setObjectName(QString::fromUtf8("labelMyAmount"));
    horizontalLayoutAction->addWidget(labelMyAmount);

    editMyAmount = new QLineEdit(centralwidget);
    editMyAmount->setObjectName(QString::fromUtf8("editMyAmount"));
    editMyAmount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    editMyAmount->setMaximumWidth(120);
    horizontalLayoutAction->addWidget(editMyAmount);

    btnCalculate = new QPushButton(centralwidget);
    btnCalculate->setObjectName(QString::fromUtf8("btnCalculate"));
    horizontalLayoutAction->addWidget(btnCalculate);

    horizontalLayoutAction->addStretch();

    verticalLayout->addLayout(horizontalLayoutAction);

    groupBoxLog = new QGroupBox(centralwidget);
    groupBoxLog->setObjectName(QString::fromUtf8("groupBoxLog"));
    QVBoxLayout *logLayout = new QVBoxLayout(groupBoxLog);
    logLayout->setContentsMargins(8, 12, 8, 8);

    textEditLog = new QTextEdit(groupBoxLog);
    textEditLog->setObjectName(QString::fromUtf8("textEditLog"));
    textEditLog->setReadOnly(true);
    logLayout->addWidget(textEditLog);

    verticalLayout->addWidget(groupBoxLog, 1);

    MainWindow->setCentralWidget(centralwidget);

    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 640, 22));

    QAction *actionFile = new QAction(menubar);
    actionFile->setObjectName(QString::fromUtf8("actionFile"));
    menubar->addAction(actionFile);

    QAction *actionEdit = new QAction(menubar);
    actionEdit->setObjectName(QString::fromUtf8("actionEdit"));
    menubar->addAction(actionEdit);

    QAction *actionView = new QAction(menubar);
    actionView->setObjectName(QString::fromUtf8("actionView"));
    menubar->addAction(actionView);

    QAction *actionHelp = new QAction(menubar);
    actionHelp->setObjectName(QString::fromUtf8("actionHelp"));
    menubar->addAction(actionHelp);

    MainWindow->setMenuBar(menubar);

    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    MainWindow->setStatusBar(statusbar);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
}

void Ui_GameView::retranslateUi(QMainWindow *MainWindow)
{
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "足球买球模拟器", nullptr));
    groupBoxTeams->setTitle(QApplication::translate("MainWindow", "参赛国家与赔率", nullptr));
    labelHeaderCountry->setText(QApplication::translate("MainWindow", "国家", nullptr));
    labelHeaderOdds->setText(QApplication::translate("MainWindow", "赔率", nullptr));
    labelHeaderCountry2->setText(QApplication::translate("MainWindow", "国家", nullptr));
    labelHeaderOdds2->setText(QApplication::translate("MainWindow", "赔率", nullptr));
    labelCountry1->setText(QApplication::translate("MainWindow", "巴西", nullptr));
    labelCountry2->setText(QApplication::translate("MainWindow", "阿根廷", nullptr));
    labelCountry3->setText(QApplication::translate("MainWindow", "德国", nullptr));
    labelCountry4->setText(QApplication::translate("MainWindow", "法国", nullptr));
    labelCountry5->setText(QApplication::translate("MainWindow", "西班牙", nullptr));
    labelCountry6->setText(QApplication::translate("MainWindow", "英格兰", nullptr));
    labelCountry7->setText(QApplication::translate("MainWindow", "葡萄牙", nullptr));
    labelCountry8->setText(QApplication::translate("MainWindow", "荷兰", nullptr));
    editOdds1->setPlaceholderText(QApplication::translate("MainWindow", "例如 1.85", nullptr));
    editOdds2->setPlaceholderText(QApplication::translate("MainWindow", "例如 2.10", nullptr));
    editOdds3->setPlaceholderText(QApplication::translate("MainWindow", "例如 3.50", nullptr));
    editOdds4->setPlaceholderText(QApplication::translate("MainWindow", "例如 4.00", nullptr));
    editOdds5->setPlaceholderText(QApplication::translate("MainWindow", "例如 5.50", nullptr));
    editOdds6->setPlaceholderText(QApplication::translate("MainWindow", "例如 6.00", nullptr));
    editOdds7->setPlaceholderText(QApplication::translate("MainWindow", "例如 8.00", nullptr));
    editOdds8->setPlaceholderText(QApplication::translate("MainWindow", "例如 10.0", nullptr));
    editOdds1->setText(QStringLiteral("1.45"));
    editOdds2->setText(QStringLiteral("2.10"));
    editOdds3->setText(QStringLiteral("3.50"));
    editOdds4->setText(QStringLiteral("4.00"));
    editOdds5->setText(QStringLiteral("6.3"));
    editOdds6->setText(QStringLiteral("6.00"));
    editOdds7->setText(QStringLiteral("8.00"));
    editOdds8->setText(QStringLiteral("10.0"));

    comboPickMatch->clear();
    comboPickMatch->addItem(QStringLiteral("巴西 vs 西班牙"));
    comboPickMatch->addItem(QStringLiteral("阿根廷 vs 英格兰"));
    comboPickMatch->addItem(QStringLiteral("德国 vs 葡萄牙"));
    comboPickMatch->addItem(QStringLiteral("法国 vs 荷兰"));

    comboPickSide->clear();
    comboPickSide->addItem(QApplication::translate("MainWindow", "主投左队", nullptr));
    comboPickSide->addItem(QApplication::translate("MainWindow", "主投右队", nullptr));

    labelPickMatch->setText(QApplication::translate("MainWindow", "选择比赛", nullptr));
    labelPickSide->setText(QApplication::translate("MainWindow", "主投方向", nullptr));
    labelPickDate->setText(QApplication::translate("MainWindow", "选择日期", nullptr));
    btnUpdate->setText(QApplication::translate("MainWindow", "更新", nullptr));
    labelMyAmount->setText(QApplication::translate("MainWindow", "我的金额", nullptr));
    editMyAmount->setPlaceholderText(QApplication::translate("MainWindow", "请输入金额", nullptr));
    editMyAmount->setText(QStringLiteral("100"));
    btnCalculate->setText(QApplication::translate("MainWindow", "计算", nullptr));
    groupBoxLog->setTitle(QApplication::translate("MainWindow", "日志", nullptr));
    textEditLog->setPlaceholderText(QApplication::translate("MainWindow", "计算结果将显示在这里...", nullptr));
    statusbar->showMessage(QApplication::translate("MainWindow", "就绪", nullptr));

    menubar->actions().at(0)->setText(QApplication::translate("MainWindow", "文件(&F)", nullptr));
    menubar->actions().at(1)->setText(QApplication::translate("MainWindow", "编辑(&E)", nullptr));
    menubar->actions().at(2)->setText(QApplication::translate("MainWindow", "视图(&V)", nullptr));
    menubar->actions().at(3)->setText(QApplication::translate("MainWindow", "帮助(&H)", nullptr));
}

QT_END_NAMESPACE
