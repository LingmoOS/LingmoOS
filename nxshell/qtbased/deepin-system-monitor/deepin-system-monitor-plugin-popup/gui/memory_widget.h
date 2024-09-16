// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef Memory_WIDGET_H
#define Memory_WIDGET_H

#include <QWidget>
#include <QPainterPath>
#include <QIcon>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE

class QPropertyAnimation;

class MemoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryWidget(QWidget *parent = nullptr);
    ~MemoryWidget();

public slots:
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
    void changeTheme(DApplicationHelper::ColorType themeType);
    void changeFont(const QFont &font);

private:
    int m_width = 0;
    int pointerRadius = 6;
    QColor summaryColor;
    QColor textColor;
    QColor ltextColor;
    QFont m_sectionFont;
    QFont m_memFont;
    QFont m_memUnitFont;
    QFont m_memTxtFont;

    QIcon m_icon {};

    QColor memoryBackgroundColor;
    QColor memoryColor {"#00C5C0"};
    QColor memoryForegroundColor {"#00C5C0"};
    QColor numberColor;
//    QColor summaryColor;
    QColor swapBackgroundColor;
    QColor swapColor {"#FEDF19"};
    QColor swapForegroundColor {"#FEDF19"};
//    QColor textColor;

    qreal memoryBackgroundOpacity = 0.1;
    qreal memoryForegroundOpacity = 1.0;
    qreal swapBackgroundOpacity = 0.1;
    qreal swapForegroundOpacity = 1.0;
    int insideRingRadius = 41;
    int memoryRenderSize = 9;
    int outsideRingRadius = 48;
    int ringCenterPointerX;
    int ringCenterPointerY = 55;
    int ringWidth = 6;

    qreal m_progress {};
    qreal m_lastMemPercent = 0.;
    qreal m_lastSwapPercent = 0.;
    QPropertyAnimation *m_animation {};


    QString m_memUsage;
    QString m_memUsageUnit;
    QString m_memTotal;
    QString m_memPercent;
    //交换内存
    QString m_swapUsage;
    QString m_swapUnit;
    QString m_swapTotal;
    QString m_swapPercent;

    bool m_isHover = false;
    int m_titleTrans = 0;
    int m_contentTrans = 0;
    int m_hoverTrans = 0;
};

#endif
