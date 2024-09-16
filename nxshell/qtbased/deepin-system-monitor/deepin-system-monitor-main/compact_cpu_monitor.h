// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPACTCPUMONITOR_H
#define COMPACTCPUMONITOR_H

#include <QWidget>
#include <QPainterPath>

class CPUInfoModel;
class BaseCommandLinkButton;

class CompactCpuMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit CompactCpuMonitor(QWidget *parent = nullptr);
    ~CompactCpuMonitor();

public:
    void setDetailButtonVisible(bool visible);

signals:
    void signalDetailInfoClicked();
    void clicked(QString msgCode);

public slots:
    void updateStatus();

private slots:
    void onDetailInfoClicked();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;


    //!
    //! \brief mouseMoveEvent 目前的策略是屏蔽鼠标移动拖拽的响应操作
    //! \param event
    //!
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    void changeFont(const QFont &font);
    void resizeItemRect();

private:
    QList<QList<qreal>> cpuPercents;
    QList<QPainterPath> cpuPaths;
    QList<QColor> cpuColors;
    int cpuRenderMaxHeight = 80;
    int cpuWaveformsRenderOffsetY = 112;
    int gridSize = 10;
    int pointsNumber = 25;
    int waveformRenderPadding = 20;
    int numCPU;
    double totalCpuPercent = 0;

    int pointerRadius = 6;

    QFont m_cpuFont;
    QFont m_statFont;

    QString m_detailText;
    CPUInfoModel *m_cpuInfomodel;
    BaseCommandLinkButton *m_detailButton;
};

#endif
