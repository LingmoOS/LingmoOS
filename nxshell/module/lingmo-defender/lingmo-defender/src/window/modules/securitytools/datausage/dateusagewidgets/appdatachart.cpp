// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appdatachart.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DListView>
#include <DSimpleListView>
#include <DPushButton>
#include <DTipLabel>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QDebug>
#include <QApplication>

DEF_USING_ANTIVIRUS_NAMESPACE

using namespace std;

const QColor BarSetNormalCorlor = QColor(175, 220, 255);
const QColor BarSetActivatedCorlor = QColor(34, 160, 255);
const QColor YAxisLabelLightColor = QColor(0, 26, 46);
const QColor YAxisLabelBlackColor = QColor(192, 198, 212);
const QColor BackgroundLightColor = QColor(255, 255, 255);
const QColor BackgroundBlackColor = QColor(0, 0, 0, 13);

AppDataChart::AppDataChart(QString appname, QWidget *parent)
    : DFrame(parent)
    , m_backgroundWidgt(nullptr)
    , m_chartView(nullptr)
    , m_dataModeSelectBtn(nullptr)
    , m_yAxisUnitLabel(nullptr)
    , m_barSet(nullptr)
    , m_firstLabel(nullptr)
    , m_finalLabel(nullptr)
    , m_activedLabel(nullptr)
    , m_activedValueLabel(nullptr)
    , m_activedIndex(0)
    , m_waitSpinner(nullptr)
{
    this->setAccessibleName("detailWidget_usageChartFrame");

    m_loadAppName = appname;
    m_timeRangeStyle = TimeRangeType::Today;
    m_usageStyle = UsageStyle::total;

    installEventFilter(this);

    initUI();

    //data connect
    connect(this, &AppDataChart::updateChartFinished, this, [=] {
        QApplication::restoreOverrideCursor();
    });
}

void AppDataChart::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_backgroundWidgt->resize(size());
}

void AppDataChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    ////更新标签坐标
    //first column
    QChart *pchart = m_chartView->chart();
    double val = m_barSet->at(0);
    QPointF point(0, 0.0);
    QPointF hourLabelPos = pchart->mapToPosition(point);
    m_firstLabel->adjustSize();
    m_firstLabel->move(int(hourLabelPos.x() - m_firstLabel->width() / 2 + 5), int(hourLabelPos.y()));

    //final column
    int endNum = m_barSet->count() - 1;
    point = QPointF(endNum, 0.0);
    hourLabelPos = pchart->mapToPosition(point);
    m_finalLabel->adjustSize();
    m_finalLabel->move(int(hourLabelPos.x() - m_finalLabel->width() / 2 + 5), int(hourLabelPos.y()));

    //actived column value
    val = m_barSet->at(m_activedIndex);
    point = QPointF(m_activedIndex, val);
    hourLabelPos = pchart->mapToPosition(point);
    m_activedValueLabel->adjustSize();
    m_activedValueLabel->move(int(hourLabelPos.x() - m_activedValueLabel->width() / 2),
                              int(hourLabelPos.y() - m_activedValueLabel->height()));

    //actived column bottom
    point = QPointF(m_activedIndex, 0.0);
    hourLabelPos = pchart->mapToPosition(point);
    m_activedLabel->adjustSize();
    m_activedLabel->move(int(hourLabelPos.x() - m_activedLabel->width() / 2 + 5),
                         int(hourLabelPos.y()));
}

bool AppDataChart::eventFilter(QObject *obj, QEvent *event)
{
    if (this == obj) {
        if (QEvent::PaletteChange == event->type()) {
            //随主题更新y轴刻度标签颜色
            if (!m_activedLabel)
                return false;
            QPalette pe;
            QColor labelCor = pe.color(m_activedLabel->foregroundRole());
            m_axisY->setLabelsColor(labelCor);
        }
    }
    return QWidget::eventFilter(obj, event); //其他事件按照原来处理
}

void AppDataChart::initUI()
{
    //设置背景色为透明
    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::Background, Qt::transparent);
    DApplicationHelper::instance()->setPalette(this, pa);

    //创建主题风格背景
    m_backgroundWidgt = new DFrame(this);
    m_backgroundWidgt->lower();
    m_backgroundWidgt->resize(size());

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    initChart();

    m_waitSpinner = new DSpinner(this);
    m_waitSpinner->setFixedSize(30, 30);
    mainLayout->addWidget(m_waitSpinner, 0, Qt::AlignCenter);
    m_waitSpinner->setVisible(false);
}

void AppDataChart::onClickWhichColumn(int index, QBarSet *barset)
{
    Q_UNUSED(barset);

    m_activedIndex = index;
    m_firstLabel->setVisible(false);
    m_finalLabel->setVisible(false);

    //更新被点击的栏的值
    QString activedValueStr = QString::number(m_barSet->at(m_activedIndex), 'd', 1);
    m_activedValueLabel->setText(activedValueStr);
    m_activedValueLabel->setVisible(true);

    //更新底部点击的标签
    if (Yesterday == m_timeRangeStyle || Today == m_timeRangeStyle)
        m_activedLabel->setText(QString("%1:00").arg(index));
    if (LastMonth == m_timeRangeStyle || ThisMonth == m_timeRangeStyle) {
        QString firstLableStr = m_firstLabel->text();
        QString activeStr = firstLableStr.split("-").first() + "-" + QString::number(index + 1);
        m_activedLabel->setText(activeStr);
    }
    m_activedLabel->setVisible(true);
}

void AppDataChart::initChart()
{
    QChart *chart = new QChart();
    chart->setParent(this);
    chart->setAttribute(Qt::WA_DeleteOnClose);
    chart->setBackgroundRoundness(8); //设置图表四角曲率
    chart->setMargins(QMargins(0, 65, 35, 0)); //设置内边界
    chart->layout()->setContentsMargins(0, 0, 0, 0); //设置外边界
    chart->setBackgroundBrush(Qt::transparent);

    m_chartView = new QChartView(chart);
    m_chartView->setAccessibleName("usageChartFrame_chartView");
    m_chartView->setBackgroundBrush(Qt::transparent);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    layout()->addWidget(m_chartView);

    QVBoxLayout *chartViewLayout = new QVBoxLayout;
    chartViewLayout->setContentsMargins(0, 0, 0, 0);
    m_chartView->setLayout(chartViewLayout);

    //data mode select
    m_dataModeSelectBtn = new DPushButton(tr("Data Used"), this);
    m_dataModeSelectBtn->setFlat(true);
    chartViewLayout->addWidget(m_dataModeSelectBtn, 0, Qt::AlignHCenter);

    QMenu *dataModeMenu = new QMenu(m_dataModeSelectBtn);
    m_dataModeSelectBtn->setMenu(dataModeMenu);
    QAction *totalAction = new QAction(tr("Total"), this);
    totalAction->setObjectName("Total");
    totalAction->setCheckable(true);
    totalAction->setChecked(true);
    dataModeMenu->addAction(totalAction);
    QAction *downloadedAction = new QAction(tr("Downloaded"), this);
    downloadedAction->setCheckable(true);
    downloadedAction->setObjectName("Downloaded");
    dataModeMenu->addAction(downloadedAction);
    QAction *upLoadedAction = new QAction(tr("Uploaded"), this);
    upLoadedAction->setObjectName("Uploaded");
    upLoadedAction->setCheckable(true);
    dataModeMenu->addAction(upLoadedAction);
    m_yAxisUnitLabel = new DTipLabel(tr("Usage").append("/MB"), this);
    m_yAxisUnitLabel->setFixedHeight(20);
    m_yAxisUnitLabel->setContentsMargins(5, 0, 0, 0);
    chartViewLayout->addWidget(m_yAxisUnitLabel, 0, Qt::AlignLeft);
    chartViewLayout->addStretch(1); //上部分结束

    //legend
    chart->legend()->setVisible(false);

    m_barSet = new QBarSet("", this);
    m_barSet->setColor(BarSetNormalCorlor);

    //bar series
    QBarSeries *barSeries = new QBarSeries(this);
    barSeries->append(m_barSet);
    barSeries->setVisible(true);
    barSeries->setName("appdata");
    chart->addSeries(barSeries);

    //创建X轴和Y轴
    m_axisX = new QBarCategoryAxis(this);
    m_axisX->setLabelsVisible(false);
    m_axisX->setLabelsColor(QColor(7, 28, 96));
    m_axisX->setGridLineVisible(false);

    m_axisY = new QValueAxis(this);
    m_axisY->setRange(0, 200); //改为setRange(0,1);则表示坐标为动态计算大小的
    m_axisY->setTickCount(6);
    m_axisY->setMinorTickCount(0);
    m_axisY->setLabelFormat("%d");

    chart->setAxisX(m_axisX, barSeries);
    chart->setAxisY(m_axisY, barSeries);

    m_firstLabel = new DLabel("00:00", this);
    m_finalLabel = new DLabel("24:00", this);
    m_activedLabel = new DLabel("12:00", this);
    m_activedLabel->setVisible(false);
    m_activedValueLabel = new DLabel(this);
    m_activedValueLabel->setVisible(false);

    //设置y轴刻度标签颜色
    QPalette pe;
    QColor labelCor = pe.color(m_activedLabel->foregroundRole());
    m_axisY->setLabelsColor(labelCor);

    //ui connect
    connect(barSeries, &QBarSeries::clicked, this, &AppDataChart::onClickWhichColumn);
    connect(dataModeMenu, &QMenu::triggered, this, &AppDataChart::selectDataMode);
}

void AppDataChart::setLoadAppName(QString name)
{
    m_loadAppName = name;
    updateChart();
}

void AppDataChart::resetLabels()
{
    if (1 > m_onShowFlowLst.size()) {
        return;
    }
    //设置图表第一栏和最后一栏时间标签
    QString format;
    if (Today == m_timeRangeStyle || Yesterday == m_timeRangeStyle) {
        format = "h:00";
    } else {
        format = "M-d";
    }
    QDateTime dateTime = QDateTime::fromString(m_onShowFlowLst.first().sTimeyMdh, "yyyy/MM/dd/hh");
    m_firstLabel->setText(dateTime.toString(format));
    dateTime = QDateTime::fromString(m_onShowFlowLst.last().sTimeyMdh, "yyyy/MM/dd/hh");
    m_finalLabel->setText(dateTime.toString(format));

    m_firstLabel->setVisible(true);
    m_finalLabel->setVisible(true);
    m_activedLabel->setVisible(false);
    m_activedValueLabel->setVisible(false);
}

void AppDataChart::setTimeRangeStyle(TimeRangeType style)
{
    m_timeRangeStyle = style;
    //等待接受数据中，切换为忙碌界面
    m_chartView->setVisible(false);
    m_waitSpinner->setVisible(true);
    m_waitSpinner->start();
    m_firstLabel->setVisible(false);
    m_finalLabel->setVisible(false);
    m_activedLabel->setVisible(false);
    m_activedValueLabel->setVisible(false);
}

void AppDataChart::onRecFlowList(const QString &pkgName, const int timeRangeTye, const DefenderProcInfoList &flowLst)
{
    Q_UNUSED(pkgName);
    Q_UNUSED(timeRangeTye);
    m_onShowFlowLst.clear();
    m_onShowFlowLst = flowLst;
    updateChart();
}

void AppDataChart::selectDataMode(QAction *action)
{
    for (QAction *ttaction : m_dataModeSelectBtn->menu()->actions()) {
        ttaction->setChecked(false);
    }
    action->setChecked(true);
    m_dataModeSelectBtn->setText(action->text());
    if ("Total" == action->objectName())
        m_usageStyle = UsageStyle::total;
    else if ("Downloaded" == action->objectName())
        m_usageStyle = UsageStyle::downloaded;
    else if ("Uploaded" == action->objectName())
        m_usageStyle = UsageStyle::uploaded;

    updateChart();
}

void AppDataChart::updateChart()
{
    int count = m_barSet->count();
    m_barSet->remove(0, count);
    m_axisX->clear();

    //当前显示的列表
    QList<double> onShowLst;
    double max = 0;
    m_thisTimeRangeDownloadedFlow = 0;
    m_thisTimeRangeUploadedFlow = 0;
    m_thisTimeRangeTotalFlow = 0;
    for (DefenderProcInfo app : m_onShowFlowLst) {
        m_thisTimeRangeDownloadedFlow += app.dDownloads; //统计当前时间跨度下的各种流量总和, 单位MB
        m_thisTimeRangeUploadedFlow += app.dUploads;
        m_thisTimeRangeTotalFlow = m_thisTimeRangeDownloadedFlow + m_thisTimeRangeUploadedFlow;

        double value; //根据流量显示的类型更新图表中各栏的值
        switch (m_usageStyle) {
        case total:
            value = app.dDownloads + app.dUploads;
            break;
        case downloaded:
            value = app.dDownloads;
            break;
        case uploaded:
            value = app.dUploads;
            break;
        }

        if (max < value)
            max = value; //更新本组数值中最大值，用于设定y轴范围
        onShowLst.append(value);
    }

    //根据最大值设定单位
    int unit = 1;
    if (GB_COUNT < max) {
        //单位为GB
        unit = GB_COUNT;
        m_yAxisUnitLabel->setText(tr("Usage").append("/GB"));
    } else if (MB_COUNT < max) {
        //单位为MB
        unit = MB_COUNT;
        m_yAxisUnitLabel->setText(tr("Usage").append("/MB"));
    } else {
        unit = KB_COUNT;
        m_yAxisUnitLabel->setText(tr("Usage").append("/KB"));
    }

    //更改y轴范围单位
    max = max / unit;

    //设置图表的x和y轴值
    for (int i = 0; i < onShowLst.size(); i++) {
        double yValue = onShowLst.at(i) / unit;
        m_barSet->append(yValue);
        m_axisX->append(QString::number(i + 1));
    }

    m_axisY->setRange(0, max * 1.2);
    if (5 > max) {
        max = 5; //y轴最小范围
        m_axisY->setRange(0, max);
    }

    //更新被点击的栏的值
    QString activedValueStr = QString::number(m_barSet->at(m_activedIndex), 'f', 1);
    m_activedValueLabel->setText(activedValueStr);

    //更新结束发送结束信号
    Q_EMIT updateChartFinished();
    //数据更新完成，隐藏等待界面
    m_chartView->setVisible(true);
    m_waitSpinner->setVisible(false);
    m_waitSpinner->stop();
    //重置图表中的标签
    resetLabels();
}

double AppDataChart::getThisTimeRangeDownloadedFlow()
{
    return m_thisTimeRangeDownloadedFlow;
}

double AppDataChart::getThisTimeRangeUploadedFlow()
{
    return m_thisTimeRangeUploadedFlow;
}

double AppDataChart::getThisTimeRangeTotalFlow()
{
    return m_thisTimeRangeTotalFlow;
}
