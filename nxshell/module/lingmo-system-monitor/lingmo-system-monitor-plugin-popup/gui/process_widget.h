// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_WIDGET_H
#define PROCESS_WIDGET_H

#include "process_table_view.h"

#include <QWidget>
#include <QPainterPath>
#include <QIcon>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE

class ProcessWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessWidget(QWidget *parent = nullptr);
    virtual ~ProcessWidget() override;

public slots:
    void updateStatus(qreal totalCpuPercent, const QList<qreal> cPercents);
    void autoHeight(int height);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *target, QEvent *event) override;

    //!
    //! \brief mouseDoubleClickEvent 鼠标压下事件，唤醒系统监视器主进程，并跳转到CPU详情界面
    //! \param event
    //!
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void changeTheme(DApplicationHelper::ColorType themeType);
    void changeFont(const QFont &font);

private:
    QList<QList<qreal>> cpuPercents;
    QList<QPainterPath> cpuPaths;
    QList<QColor> cpuColors;
    int m_width = 0;
    int cpuRenderMaxHeight = 80;
    int cpuWaveformsRenderOffsetY = 112;
    int gridSize = 10;
    int pointsNumber = 25;
    int waveformRenderPadding = 20;
    double totalCpuPercent = 0;

    int pointerRadius = 6;

    QColor summaryColor;
    QColor textColor;
    QColor ltextColor;
    QFont m_sectionFont;
    QFont m_contentFont;
    QFont m_subContentFont;

    QIcon m_icon {};

    qreal m_recvBps {};
    qreal m_sentBps {};
    qulonglong m_totalRecvBytes {};
    qulonglong m_totalSentBytes {};

    ProcessTableView *m_processTableView;

    bool m_isHover = false;
    int m_titleTrans = 0;
    int m_contentTrans = 0;
    int m_hoverTrans = 0;
    int m_processWidgetHeight = 255;
};

#endif
