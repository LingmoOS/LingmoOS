// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_WIDGET_H
#define CPU_WIDGET_H

#include <QWidget>
#include <QPainterPath>
#include <QIcon>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE

class CpuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CpuWidget(QWidget *parent = nullptr);
    ~CpuWidget();

public slots:
//    void updateStatus(qreal totalCpuPercent, const QList<qreal> cPercents);
//    void slot_onCpuPer(QString cpuPer);
    void updateStatus();

protected:
    void paintEvent(QPaintEvent *event) override;

    bool eventFilter(QObject *target, QEvent *event) override;

    //!
    //! \brief mouseDoubleClickEvent 鼠标压下事件，唤醒系统监视器主进程，并跳转到CPU详情界面
    //! \param event
    //!
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void initConnection();
    void changeTheme(DApplicationHelper::ColorType themeType);
    void changeFont(const QFont &font);
    void getPainterPathByData(QList<double> *listData, QPainterPath &path, qreal maxVlaue);

private:
    QList<QList<qreal>> cpuPercents;
    QList<QPainterPath> cpuPaths;
    QList<double> *downloadSpeeds;
    QPainterPath downloadPath;

//    ReadCpuThread *m_readCpuThread = nullptr;

    int cpuRenderMaxHeight = 80;
    int cpuWaveformsRenderOffsetY = 112;
    int gridSize = 10;
    int pointsNumber = 25;
    int waveformRenderPadding = 20;
    int numCPU;
    double totalCpuPercent = 0;
    qreal m_cpuPer = 0;
    int m_width = 0;

    int pointerRadius = 6;

    QColor summaryColor;
    QColor textColor;
    QColor ltextColor;

    QFont m_TitleFont;
    QFont m_contentFont;
    QFont m_contentUnitFont;
    QFont m_subContentFont;
    QIcon m_icon {};

    bool m_isHover = false;

    int m_titleTrans = 0;
    int m_contentTrans = 0;
    int m_hoverTrans = 0;
};

#endif
