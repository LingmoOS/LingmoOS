// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmonitortipswidget.h"
#include "ddlog.h"
#include <QAccessible>
#include <QPainter>
#include <DApplication>
#include <QLineF>

DWIDGET_USE_NAMESPACE
using namespace DDLog;
SystemMonitorTipsWidget::SystemMonitorTipsWidget(QWidget *parent)
    : QFrame(parent)
{
}

void SystemMonitorTipsWidget::setSystemMonitorTipsText(QStringList strList)
{
    m_textList = strList;

    //  [   CPU: 100.0%    ↓ 1000kb/s  ]
    // 获取strList字符串，并存入cpu,mem,upLoad,download中
    QString cpu;
    QString mem;
    QString upLoad;
    QString downLoad;
    for (int i = 0; i < m_textList.size(); ++i) {
        if (i == 0)
            cpu = m_textList.at(i);
        else if (i == 1)
            mem = m_textList.at(i);
        else if (i == 2)
            downLoad = m_textList.at(i);
        else if (i == 3)
            upLoad = m_textList.at(i);
        else
            qCDebug(app) << "do not set the text";
    }

    // 设置左侧字符串宽度
    if (cpu.length() == 3) {
        m_leftWidth = fontMetrics().width(QString(" ") + DApplication::translate("Plugin.cpu", "CPU") + QString(": 0") + cpu + QString(" "));
    } else {
        m_leftWidth = fontMetrics().width(QString(" ") + DApplication::translate("Plugin.cpu", "CPU") + QString(": ") + cpu + QString(" "));
    }
    // 左侧宽度预留20个像素
    m_leftWidth += 20;

    // 设置右侧字符串宽度
    if (downLoad.length() == 3) {
        m_rightWidth = fontMetrics().width(QString("↓000") + downLoad + QString(" "));
    } else if (downLoad.length() == 4) {
        m_rightWidth = fontMetrics().width(QString("↓00") + downLoad + QString(" "));
    } else if (downLoad.length() == 5) {
        m_rightWidth = fontMetrics().width(QString("↓0") + downLoad + QString(" "));
    } else {
        m_rightWidth = fontMetrics().width(QString("↓") + downLoad + QString(" "));
    }
    // 设置右侧字符串宽度预留20个像素
    m_rightWidth += 20;

    // 设置当前窗体固定大小
    setFixedSize(5 + 5 + m_leftWidth + m_rightWidth, fontMetrics().boundingRect(m_textList.at(0)).height() * 2 + 20);
    update();
}

void SystemMonitorTipsWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    int rectHeight = rect().height();
    QPainter painter(this);
    painter.setPen(QPen(palette().brightText(), 1));

    QTextOption optionLeft;
    optionLeft.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QTextOption optionRight;
    optionRight.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QTextOption optionMid;
    optionMid.setAlignment(Qt::AlignCenter);

    // 获取strList字符串，并存入cpu,mem,upLoad,download中
    QString cpu;
    QString mem;
    QString upLoad;
    QString downLoad;
    for (int i = 0; i < m_textList.size(); ++i) {
        if (i == 0)
            cpu = m_textList.at(i);
        else if (i == 1)
            mem = m_textList.at(i);
        else if (i == 2)
            downLoad = m_textList.at(i);
        else if (i == 3)
            upLoad = m_textList.at(i);
        else
            qCDebug(app) << "do not set the text";
    }

    // 当没有数据更新时，设置默认的悬浮框内容
    if (m_textList.size() == 0) {
        cpu = "0.0";
        mem = "0.0";
        upLoad = "0KB/s";
        downLoad = "0KB/s";
    }

    //    int specialCharaWidth = fontMetrics().width(QString("↓"));
    int specialCharaWidth = 10;

    int specialCharaHeight = fontMetrics().width(QString("1")) + 5;
    // 左侧空白区域
    int leftMargin = 10;

    // 绘制左侧空白矩形区域
    painter.save();
    painter.setPen(QColor(Qt::transparent));
    painter.drawRect(QRectF(0.0, 0.0, leftMargin, rectHeight / 2.0));
    painter.restore();

    painter.setPen(QPen(palette().brightText(), 1));
    // 绘制CPU文字信息
    painter.drawText(QRectF(leftMargin, 0.0, m_leftWidth, rectHeight / 2.0), QString(" ") + DApplication::translate("Plugin.cpu", "CPU") + QString(": ") + cpu, optionLeft);

    // 绘制下箭头
    painter.save();
    painter.setPen(QPen(palette().brightText(), 2));
    painter.setRenderHints(QPainter::Antialiasing);
    QVector<QLineF> vecDown;
    vecDown.append(QLineF(QPointF(m_leftWidth + leftMargin, rectHeight / 2.0 - rectHeight / 4.0), QPointF(leftMargin + m_leftWidth + specialCharaWidth / 2, rectHeight / 2.0 - rectHeight / 4.0 + specialCharaHeight / 2.0)));
    vecDown.append(QLineF(QPointF(m_leftWidth + leftMargin + specialCharaWidth / 2, rectHeight / 2.0 - rectHeight / 4.0 + specialCharaHeight / 2.0), QPointF(leftMargin + m_leftWidth + specialCharaWidth, rectHeight / 2.0 - rectHeight / 4.0)));
    vecDown.append(QLineF(QPointF(m_leftWidth + leftMargin + specialCharaWidth / 2, rectHeight / 2.0 - rectHeight / 4.0 + specialCharaHeight / 2.0), QPointF(leftMargin + m_leftWidth + specialCharaWidth / 2, rectHeight / 2.0 - rectHeight / 4.0 - specialCharaHeight / 2.0)));
    painter.drawLines(vecDown);
    painter.restore();

    //    painter.drawText(QRectF(m_leftWidth, 0.0, specialCharaWidth, rectHeight / 2.0), QString("↓"), optionMid);
    // 绘值下载文字信息
    painter.drawText(QRectF(leftMargin + m_leftWidth + specialCharaWidth + 5, 0, m_rightWidth, rectHeight / 2.0), downLoad, optionLeft);

    //    specialCharaWidth = fontMetrics().width(QString("↑"));
    // 绘制左侧空白矩形区域
    painter.save();
    painter.setPen(QColor(Qt::transparent));
    painter.drawRect(QRectF(0, 0, leftMargin, rectHeight / 2.0));
    painter.restore();

    // 绘制内存文字信息
    painter.drawText(QRectF(leftMargin, rectHeight / 2.0, m_leftWidth, rectHeight / 2.0), QString(" ") + DApplication::translate("Plugin.mem", "MEM") + QString(": ") + mem, optionLeft);
    painter.save();
    painter.setPen(QPen(palette().brightText(), 2));
    painter.setRenderHints(QPainter::Antialiasing);
    // 绘制上箭头
    QVector<QLineF> vecUp;
    vecUp.append(QLineF(QPointF(leftMargin + m_leftWidth, rectHeight / 2.0 + rectHeight / 4.0), QPointF(leftMargin + m_leftWidth + specialCharaWidth / 2, rectHeight / 2.0 + rectHeight / 4.0 - specialCharaHeight / 2.0)));
    vecUp.append(QLineF(QPointF(leftMargin + m_leftWidth + specialCharaWidth / 2, rectHeight / 2.0 + rectHeight / 4.0 - specialCharaHeight / 2.0), QPointF(leftMargin + m_leftWidth + specialCharaWidth, rectHeight / 2.0 + rectHeight / 4.0)));
    vecUp.append(QLineF(QPointF(leftMargin + m_leftWidth + specialCharaWidth / 2, rectHeight / 2.0 + rectHeight / 4.0 - specialCharaHeight / 2.0), QPointF(leftMargin + m_leftWidth + specialCharaWidth / 2, rectHeight / 2.0 + rectHeight / 4.0 + specialCharaHeight / 2.0)));
    painter.drawLines(vecUp);
    painter.restore();
    //    painter.drawText(QRectF(m_leftWidth, rectHeight / 2.0, specialCharaWidth, rectHeight / 2.0), QString("↑"), optionMid);

    // 绘制上传文字信息
    painter.setPen(QPen(palette().brightText(), 1));
    painter.drawText(QRectF(leftMargin + m_leftWidth + specialCharaWidth + 5, rectHeight / 2.0, m_rightWidth, rectHeight / 2.0), upLoad, optionLeft);
}

bool SystemMonitorTipsWidget::event(QEvent *event)
{
    // 字体切换事件捕捉，当前获取到字体切换信号时，更新当前的悬浮框界面
    if (event->type() == QEvent::FontChange) {
        if (m_textList.size() > 0)
            setSystemMonitorTipsText(m_textList);
        else
            setSystemMonitorTipsText(QStringList() << "0.0"
                                                   << "0.0"
                                                   << "0KB/s"
                                                   << "0KB/s");
    } else if (event->type() == QEvent::Hide) {
        Q_EMIT visibleChanged(false);
    } else if (event->type() == QEvent::Show) {
        Q_EMIT visibleChanged(true);
    }

    return QFrame::event(event);
}
