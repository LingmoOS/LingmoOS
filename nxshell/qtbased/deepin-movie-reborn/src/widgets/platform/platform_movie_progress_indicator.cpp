// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  *@file 这个文件是负责实现全屏展示进度控件相关
  */
#include "platform_movie_progress_indicator.h"
#include "utils.h"

namespace dmr {
/**
 * @brief MovieProgressIndicator 构造函数
 * @param parent 父窗口
 */
Platform_MovieProgressIndicator::Platform_MovieProgressIndicator(QWidget *parent)
    : QFrame(parent)
{
    initMember();

    QFont font;
    //参考设计图
    font.setPixelSize(14);
    QFontMetrics fontMetrics(font);
    this->setFont(font);
    m_fixedSize = QSize(qMax(52, fontMetrics.width("999:99")), fontMetrics.height() + 10);
    this->setFixedSize(m_fixedSize);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(this->windowFlags() | Qt::Dialog);
}
/**
 * @brief paintEvent 重载绘制事件函数
 * @param pPaintEvent
 */
void Platform_MovieProgressIndicator::paintEvent(QPaintEvent *pPaintEvent)
{
    QString sTimeText = QTime::currentTime().toString("hh:mm");
    QPainter painter(this);
    painter.setFont(font());
    painter.setPen(QColor(255, 255, 255, static_cast<int>(255 * .4)));

    QFontMetrics fontMetrics(font());
    QRect fontRect = fontMetrics.boundingRect(sTimeText);
    fontRect.moveCenter(QPoint(rect().center().x(), fontRect.height() / 2));
    painter.drawText(fontRect, sTimeText);

    QPoint pos((m_fixedSize.width() - 48) / 2, rect().height() - 5);
    int pert = static_cast<int>(qMin(m_pert * 10, 10.0));
    for (int i = 0; i < 10; i++) {
        if (i >= pert) {
            painter.fillRect(QRect(pos, QSize(3, 3)), QColor(255, 255, 255, static_cast<int>(255 * .25)));
        } else {
            painter.fillRect(QRect(pos, QSize(3, 3)), QColor(255, 255, 255, static_cast<int>(255 * .5)));
        }
        pos.rx() += 5;
    }

    QFrame::paintEvent(pPaintEvent);
}
/**
 * @brief initMember 初始化成员变量
 */
void Platform_MovieProgressIndicator::initMember()
{
    m_nElapsed = 0;
    m_pert = 0;
    m_fixedSize = QSize(0, 0);
}
/**
 * @brief updateMovieProgress 更新电影进度控件
 * @param duration 总时长
 * @param pos 当前时长
 */
void Platform_MovieProgressIndicator::updateMovieProgress(qint64 duration, qint64 pos)
{
    m_nElapsed = pos;
    if (duration != 0)
        m_pert = static_cast<qreal>(((float)pos) / duration);
    update();
}

}
