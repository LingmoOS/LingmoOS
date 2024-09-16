// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPACTMEMORYMONITOR_H
#define COMPACTMEMORYMONITOR_H

#include <QWidget>

class QPropertyAnimation;
class MemStatModel;
class MemInfoModel;

namespace core {
namespace system {
class MemInfo;
}
}

class CompactMemoryMonitor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)

public:
    explicit CompactMemoryMonitor(QWidget *parent);
    ~CompactMemoryMonitor();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;

    //!
    //! \brief mouseMoveEvent 鼠标压下事件，唤醒系统监视器主进程，并跳转到CPU详情界面
    //! \param event
    //!
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;


private:
    void changeTheme(int themeType);
    void changeFont(const QFont &font);

    qreal progress() const;
    void setProgress(qreal p);

signals:
    void clicked(QString msgCode);

private slots:
    void onStatInfoUpdated();
    void animationFinshed();
    void onValueChanged();

private:
    QColor memoryBackgroundColor;
    QColor memoryColor {"#00C5C0"};
    QColor memoryForegroundColor {"#00C5C0"};
    QColor numberColor;
    QColor summaryColor;
    QColor swapBackgroundColor;
    QColor swapColor {"#FEDF19"};
    QColor swapForegroundColor {"#FEDF19"};
    QColor textColor;

    double memoryBackgroundOpacity = 0.1;
    double memoryForegroundOpacity = 1.0;
    double swapBackgroundOpacity = 0.1;
    double swapForegroundOpacity = 1.0;
    int insideRingRadius = 41;
    int memoryRenderSize = 9;
    int outsideRingRadius = 48;
    int pointerRadius = 3;
    int ringCenterPointerX;
    int ringCenterPointerY = 45;
    int ringWidth = 6;

    core::system::MemInfo *m_memInfo {};

    qreal m_progress {};
    qreal m_lastMemPercent = 0.;
    qreal m_lastSwapPercent = 0.;
    QPropertyAnimation *m_animation {};

    QFont m_contentFont;
    QFont m_subContentFont;
    QFont m_memPercentFont;
};

#endif
