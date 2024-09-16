// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CpuMONITOR_H
#define CpuMONITOR_H

#include <DApplicationHelper>

#include <QIcon>
#include <QList>
#include <QWidget>
#include <QPainterPath>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class Settings;
class QPropertyAnimation;
class CPUInfoModel;
class BaseCommandLinkButton;

class CpuMonitor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)

public:
    explicit CpuMonitor(QWidget *parent = nullptr);
    ~CpuMonitor();

public:
    void setDetailButtonVisible(bool visible);

signals:
    void signalDetailInfoClicked();
    void clicked(QString msgCode);

public slots:
    void changeTheme(DApplicationHelper::ColorType themeType);
    void updateStatus();
    void onDetailInfoClicked();

private:
    void resizeItemWidgetRect();

private:
    void changeFont(const QFont &font);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;

    //!
    //! \brief mouseMoveEvent 目前的策略是屏蔽鼠标移动拖拽的响应操作
    //! \param event
    //!
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    qreal progress() const
    {
        return m_progress;
    }
    void setProgress(qreal p)
    {
        m_progress = p;
    }

private:
    QIcon m_icon;

    QList<double> *cpuPercents;
    QPainterPath cpuPath;
    QColor numberColor;
    QColor ringBackgroundColor;
    QColor ringForegroundColor {"#0081FF"};
    QColor textColor;
    double ringBackgroundOpacity = 0.1;
    double ringForegroundOpacity = 1;
    int animationIndex = 0;
    int cpuRenderMaxHeight = 45;
    int percentRenderOffsetY = 131;
    int pointsNumber = 28;
    int ringRadius = 90;
    int ringRenderOffsetY = 86;
    int ringWidth = 8;
    int titleAreaPaddingX = 5;
    int titleRenderOffsetY = 161;
    int waveformsRenderOffsetX;
    int waveformsRenderOffsetY = 96;

    Settings *m_settings;

    QFont m_cpuUsageFont;
    QFont m_cpuDisplayFont;
    QFont m_detailFont;

    qreal m_progress {};
    QPropertyAnimation *m_animation {};
    CPUInfoModel *m_cpuInfomodel;

    BaseCommandLinkButton *m_detailButton;
};

#endif
