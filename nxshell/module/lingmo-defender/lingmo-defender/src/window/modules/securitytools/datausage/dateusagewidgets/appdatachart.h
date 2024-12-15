// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"
#include "dtkwidget_global.h"
#include "../datausagemodel.h"

#include <DFrame>
#include <DLabel>
#include <DTipLabel>
#include <DPushButton>
#include <DSpinner>

#include <QWidget>
#include <QFrame>
#include <QChartView>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QGraphicsLayout>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

DEF_NAMESPACE_BEGIN
DEF_NETPROTECTION_NAMESPACE_BEGIN
DWIDGET_USE_NAMESPACE
QT_CHARTS_USE_NAMESPACE

class AppDataChart : public DFrame
{
    Q_OBJECT
public:
    enum UsageStyle { total,
                      downloaded,
                      uploaded };

public:
    explicit AppDataChart(QString appname, QWidget *parent = nullptr);

    void setLoadAppName(QString name);
    void resetLabels();
    void updateChart();

    double getThisTimeRangeDownloadedFlow();
    double getThisTimeRangeUploadedFlow();
    double getThisTimeRangeTotalFlow();

    void loadDayData();
    void loadMonthData();

Q_SIGNALS:
    void updateChartFinished();

public Q_SLOTS:
    void onClickWhichColumn(int index, QBarSet *barset);
    void selectDataMode(QAction *action);
    void setTimeRangeStyle(TimeRangeType style);

    void onRecFlowList(const QString &pkgName, const int timeRangeTye, const DefenderProcInfoList &flowLst);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void initUI();
    void initChart();

private:
    DFrame *m_backgroundWidgt;
    QChartView *m_chartView;
    DPushButton *m_dataModeSelectBtn;
    DTipLabel *m_yAxisUnitLabel;
    QBarSet *m_barSet;
    QBarCategoryAxis *m_axisX;
    QValueAxis *m_axisY;
    DLabel *m_firstLabel;
    DLabel *m_finalLabel;
    DLabel *m_activedLabel;
    DLabel *m_activedValueLabel;
    int m_activedIndex; //点击的流量柱检索

    QString m_loadAppName;
    TimeRangeType m_timeRangeStyle; //应用流量统计显示的时间范围
    UsageStyle m_usageStyle; //应用流量统计显示的流量类型
    double m_thisTimeRangeDownloadedFlow;
    double m_thisTimeRangeUploadedFlow;
    double m_thisTimeRangeTotalFlow;
    DefenderProcInfoList m_onShowFlowLst;
    //等待界面
    DSpinner *m_waitSpinner;
};

DEF_NAMESPACE_END
DEF_NETPROTECTION_NAMESPACE_END
