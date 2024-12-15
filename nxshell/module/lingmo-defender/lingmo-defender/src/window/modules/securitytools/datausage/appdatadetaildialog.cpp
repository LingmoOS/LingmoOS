// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appdatadetaildialog.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DListView>
#include <DSimpleListView>
#include <DLabel>
#include <DTipLabel>
#include <DTitlebar>
#include <DPlatformWindowHandle>
#include <DBlurEffectWidget>
#include <DPushButton>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QMenu>
#include <QGraphicsLayout>
#include <QPixmap>
#include <QDebug>

DWIDGET_USE_NAMESPACE
DEF_USING_ANTIVIRUS_NAMESPACE
QT_CHARTS_USE_NAMESPACE

//时间跨度按钮选项的object name
const QString TodayActionName = "today";
const QString YesterdayActionName = "yesterday";
const QString ThisMonthActionName = "thisMonth";
const QString LastMonthActionName = "lastMonth";

AppDataDetailDialog::AppDataDetailDialog(QString appname, QString pkgName, QString icon,
                                         DataUsageModel *model, QWidget *parent)
    : DFrame(parent)
    , m_model(model)
    , m_blurEffect(nullptr)
    , m_mainWidgt(nullptr)
    , m_appName(appname)
    , m_pkgName(pkgName)
    , m_appIcon(icon)
    , m_totalUsageNum(nullptr)
    , m_downloadedUsageNum(nullptr)
    , m_uploadedUsageNum(nullptr)
    , m_timeRangeSelect(nullptr)
    , m_chart(nullptr)
{
    setAccessibleName("flowTableView_detailFrame");
    setWindowFlags(Qt::Dialog /*| Qt::WindowStaysOnTopHint*/);

    setWindowModality(Qt::ApplicationModal);

    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(840, 540);

    m_blurEffect = new DBlurEffectWidget(this);
    m_blurEffect->setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    m_blurEffect->resize(size());
    m_blurEffect->lower();

    //close button
    QHBoxLayout *titleBarLayout = new QHBoxLayout(this);
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    DTitlebar *titleBar = new DTitlebar(this);
    titleBar->setBackgroundTransparent(true);
    titleBar->setTitle("");
    titleBar->setMenuVisible(false);
    titleBarLayout->addWidget(titleBar, 0, Qt::AlignRight | Qt::AlignTop);

    initUI();
}

void AppDataDetailDialog::initUI()
{
    m_mainWidgt = new QWidget(this);
    m_mainWidgt->setAccessibleName("detailFrame_detailWidget");
    m_mainWidgt->move(0, 50);
    m_mainWidgt->resize(size().width(), size().height() - 50);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(44, 0, 44, 50);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop);
    m_mainWidgt->setLayout(mainLayout);

    //title
    QLabel *appDateDetailLabel = new QLabel(tr("Usage Details"), this);
    appDateDetailLabel->setAccessibleName("detailWidget_usageTitleLable");
    appDateDetailLabel->setContentsMargins(0, 0, 0, 0);
    QFont appDateDetailLabelFont = appDateDetailLabel->font();
    appDateDetailLabelFont.setBold(true);
    appDateDetailLabelFont.setPointSize(12);
    appDateDetailLabel->setFont(appDateDetailLabelFont);
    mainLayout->addWidget(appDateDetailLabel);
    mainLayout->addSpacing(15);

    //app label
    QHBoxLayout *appLabelLayout = new QHBoxLayout;
    appLabelLayout->setContentsMargins(0, 0, 0, 0);
    appLabelLayout->setMargin(0);
    appLabelLayout->setSpacing(0);
    mainLayout->addLayout(appLabelLayout);

    DLabel *m_appIconLabel = new DLabel("icon", this);
    m_appIconLabel->setAccessibleName("detailWidget_iconLable");
    m_appIconLabel->setContentsMargins(0, 0, 0, 0);
    m_appIconLabel->setPixmap(QIcon::fromTheme(m_appIcon).pixmap(40, 40));
    appLabelLayout->addWidget(m_appIconLabel, 0, Qt::AlignLeft);

    ////app label right part
    //app name
    QVBoxLayout *appLabelRightLayout = new QVBoxLayout;
    appLabelRightLayout->setContentsMargins(10, 0, 0, 0);
    appLabelRightLayout->setSpacing(0);
    appLabelLayout->addLayout(appLabelRightLayout);

    DLabel *m_appNameLabel = new DLabel(m_appName, this);
    m_appNameLabel->setAccessibleName("detailWidget_appNameLable");
    appLabelRightLayout->addWidget(m_appNameLabel, 0, Qt::AlignLeft);

    //app version
    QHBoxLayout *appVersionLayout = new QHBoxLayout;
    appVersionLayout->setContentsMargins(0, 0, 0, 0);
    appVersionLayout->setMargin(0);
    appVersionLayout->setSpacing(0);
    appLabelRightLayout->addLayout(appVersionLayout);

    DTipLabel *m_appVersion = new DTipLabel("", this);
    m_appVersion->setAccessibleName("detailWidget_appVerLable");
    DFontSizeManager::instance()->bind(m_appVersion, DFontSizeManager::T7);
    m_appVersion->setContentsMargins(0, 0, 0, 0);
    appVersionLayout->addWidget(m_appVersion, 0, Qt::AlignLeft);

    //获取应用版本
    QProcess *getVerProc = new QProcess(this);

    ////获取 英文名称
    QString pidName = "";
    QStringList firstCutStrLst = m_pkgName.split(".");
    if (!firstCutStrLst.isEmpty()) {
        pidName = firstCutStrLst.at(0);
    }

    QStringList paraLst;
    paraLst << "-f"
            << "${Version}\n"
            << "--show" << QString("*%1*").arg(pidName);
    connect(getVerProc, &QProcess::readyReadStandardOutput, this, [=]() {
        QString allVerStr = getVerProc->readAllStandardOutput().trimmed();

        QStringList verStrLst = allVerStr.split("\n");
        QString verStr = "";
        for (QString ver : verStrLst) {
            if ("\n" != ver) {
                verStr = ver;
                break;
            }
        }
        m_appVersion->setText(verStr);
        getVerProc->waitForFinished();
        getVerProc->deleteLater();
    });
    getVerProc->start("dpkg-query", paraLst);

    ////Total Usage, Downloaded Usage, Uploaded Usage, time range switch
    appVersionLayout->addStretch(1);
    DTipLabel *totalUsageLabel = new DTipLabel(tr("Total").append(": "), this);
    totalUsageLabel->setAccessibleName("detailWidget_totalUsageLable");
    appVersionLayout->addWidget(totalUsageLabel, 0, Qt::AlignLeft);
    m_totalUsageNum = new DTipLabel("0MB", this);
    m_totalUsageNum->setAccessibleName("detailWidget_totalUsageDataLable");
    appVersionLayout->addWidget(m_totalUsageNum, 0, Qt::AlignLeft);

    appVersionLayout->addSpacing(20);
    DTipLabel *downloadedUsageLabel = new DTipLabel(tr("Downloaded").append(": "), this);
    downloadedUsageLabel->setAccessibleName("detailWidget_downLoadLable");
    appVersionLayout->addWidget(downloadedUsageLabel, 0, Qt::AlignLeft);
    m_downloadedUsageNum = new DTipLabel("0MB", this);
    m_downloadedUsageNum->setAccessibleName("detailWidget_downLoadDataLable");
    appVersionLayout->addWidget(m_downloadedUsageNum, 0, Qt::AlignLeft);

    appVersionLayout->addSpacing(20);
    DTipLabel *uploadedUsageLabel = new DTipLabel(tr("Uploaded").append(": "), this);
    uploadedUsageLabel->setAccessibleName("detailWidget_upLoadLable");
    appVersionLayout->addWidget(uploadedUsageLabel, 0, Qt::AlignLeft);
    m_uploadedUsageNum = new DTipLabel("0MB", this);
    m_uploadedUsageNum->setAccessibleName("detailWidget_upLoadDataLable");
    appVersionLayout->addWidget(m_uploadedUsageNum, 0, Qt::AlignLeft);

    appVersionLayout->addSpacing(20);
    m_timeRangeSelect = new DPushButton(tr("Today"), this);
    m_timeRangeSelect->setAccessibleName("detailWidget_timeButton");
    m_timeRangeSelect->setFlat(true);
    appVersionLayout->addWidget(m_timeRangeSelect, 0, Qt::AlignRight);

    //time range switch
    QMenu *timeRangeMenu = new QMenu(this);
    timeRangeMenu->setAccessibleName("detailWidget_timeTangeMenu");
    m_timeRangeSelect->setMenu(timeRangeMenu);
    QAction *todayAction = new QAction(tr("Today"), this);
    todayAction->setObjectName(TodayActionName);
    todayAction->setCheckable(true);
    timeRangeMenu->addAction(todayAction);
    QAction *yesterdayAction = new QAction(tr("Yesterday"), this);
    yesterdayAction->setObjectName(YesterdayActionName);
    yesterdayAction->setCheckable(true);
    timeRangeMenu->addAction(yesterdayAction);
    QAction *thisMonthAction = new QAction(tr("This month"), this);
    thisMonthAction->setObjectName(ThisMonthActionName);
    thisMonthAction->setCheckable(true);
    timeRangeMenu->addAction(thisMonthAction);
    QAction *lastMonthAction = new QAction(tr("Last month"), this);
    lastMonthAction->setObjectName(LastMonthActionName);
    lastMonthAction->setCheckable(true);
    timeRangeMenu->addAction(lastMonthAction);

    //data chart
    m_chart = new AppDataChart(m_pkgName, this);
    mainLayout->addWidget(m_chart, 1);

    //connect
    connect(timeRangeMenu, &QMenu::triggered, this, &AppDataDetailDialog::selectTimeRangeStyle);
    //图表更新结束后响应updateThisRangeUsageNum()
    connect(m_chart, &AppDataChart::updateChartFinished, this, &AppDataDetailDialog::updateThisRangeUsageNum);
    //默认显示时间范围为当天
    selectTimeRangeStyle(todayAction);

    //链接 m_worker->sendAppFlowList m_char->onRecFlowList
    connect(m_model, &DataUsageModel::sendAppFlowList, m_chart, &AppDataChart::onRecFlowList);
}

void AppDataDetailDialog::selectTimeRangeStyle(QAction *action)
{
    for (QAction *ttaction : m_timeRangeSelect->menu()->actions()) {
        ttaction->setChecked(false);
    }
    action->setChecked(true);
    m_timeRangeSelect->setText(action->text());
    TimeRangeType timeRangeType = TimeRangeType::Today;
    if (TodayActionName == action->objectName()) {
        timeRangeType = TimeRangeType::Today;
    } else if (YesterdayActionName == action->objectName()) {
        timeRangeType = TimeRangeType::Yesterday;
    } else if (ThisMonthActionName == action->objectName()) {
        timeRangeType = TimeRangeType::ThisMonth;
    } else if (LastMonthActionName == action->objectName()) {
        timeRangeType = TimeRangeType::LastMonth;
    }
    m_model->asyncGetAppFlowList(m_pkgName, timeRangeType);
    m_chart->setTimeRangeStyle(timeRangeType);

    //更新此时间段应用总流量数据
    //初始化中连接了m_chart的图表更新结束信号与updateThisRangeUsageNum()
}

void AppDataDetailDialog::updateThisRangeUsageNum()
{
    QString totalUsageNumStr = Utils::changeFlowValueUnit(m_chart->getThisTimeRangeTotalFlow(), 1);
    m_totalUsageNum->setText(totalUsageNumStr);

    QString downloadedUsageNumStr = Utils::changeFlowValueUnit(m_chart->getThisTimeRangeDownloadedFlow(), 1);
    m_downloadedUsageNum->setText(downloadedUsageNumStr);

    QString uploadedUsageNumStr = Utils::changeFlowValueUnit(m_chart->getThisTimeRangeUploadedFlow(), 1);
    m_uploadedUsageNum->setText(uploadedUsageNumStr);
}

void AppDataDetailDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    m_blurEffect->resize(size());
    m_mainWidgt->resize(size().width(), size().height() - 50);
}

void AppDataDetailDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->deleteLater();
}
