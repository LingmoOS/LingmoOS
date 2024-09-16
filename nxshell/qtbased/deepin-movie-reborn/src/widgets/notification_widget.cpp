// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 *@file 这个文件是实现影院左上角信息弹窗
 */
#include "notification_widget.h"
#include "utils.h"

#include <DPlatformWindowHandle>
#include <dthememanager.h>
#include <dapplication.h>
#include <QPainterPath>

namespace dmr {
/**
 * @brief NotificationWidget 构造函数
 * @param parent 父窗口
 */
NotificationWidget::NotificationWidget(QWidget *parent)
    : QFrame(parent), m_pMainWindow(parent)
{
    initMember();
    setObjectName("NotificationFrame");

    m_pMainLayout = new QHBoxLayout();
    m_pMainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pMainLayout);

    m_pMsgLabel = new DLabel();
    m_pMsgLabel->setFrameShape(QFrame::NoFrame);
    //添加在两种主题下文字效果，使其更加明显
    int nType = DGuiApplicationHelper::instance()->themeType();
    if (nType == 2) {
        m_pMsgLabel->setForegroundRole(DPalette::TextLively);
    } else {
        m_pMsgLabel->setForegroundRole(QPalette::ToolTipText);
    }

    m_pTimer = new QTimer(this);
    if (!utils::check_wayland_env()) {
        m_pTimer->setInterval(2000);
    } else {
        m_pTimer->setInterval(500);
    }
    m_pTimer->setSingleShot(true);
    connect(m_pTimer, &QTimer::timeout, this, &QWidget::hide);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, [=](int nType) {
        if (nType == 2) {
            m_pMsgLabel->setForegroundRole(DPalette::TextLively);
        } else {
            m_pMsgLabel->setForegroundRole(QPalette::ToolTipText);
        }
    });

}
/**
 * @brief showEvent 重载显示事件函数
 * @param pShowEvent 显示事件
 */
void NotificationWidget::showEvent(QShowEvent *event)
{
    ensurePolished();
    if (m_pMainLayout->indexOf(m_pIconLabel) == -1) {
        resize(m_pMsgLabel->sizeHint().width() + m_pMainLayout->contentsMargins().left()
               + m_pMainLayout->contentsMargins().right(), height());
        adjustSize();
    }
    syncPosition();

    QFrame::showEvent(event);
}
/**
 * @brief resizeEvent 重载大小变化事件函数
 * @param pResizeEvent 大小变化事件
 */
void NotificationWidget::resizeEvent(QResizeEvent *re)
{
    QFrame::resizeEvent(re);
}
/**
 * @brief syncPosition 同步控件位置
 */
void NotificationWidget::syncPosition()
{
    QRect geom = m_pMainWindow->geometry();
    switch (m_pAnchor) {
    case ANCHOR_BOTTOM:
        move(geom.center().x() - size().width() / 2, geom.bottom() - m_nAnchorDist - height());
        break;

    case ANCHOR_NORTH_WEST:
        move(m_anchorPoint);
        break;

    case ANCHOR_NONE:
        move(geom.center().x() - size().width() / 2, geom.center().y() - size().height() / 2);
        break;
    }
}
/**
 * @brief syncPosition 同步控件位置
 * @param rect 控件的位置
 */
void NotificationWidget::syncPosition(QRect rect)
{
    QRect geom = rect;
    switch (m_pAnchor) {
    case ANCHOR_BOTTOM:
        move(geom.center().x() - size().width() / 2, geom.bottom() - m_nAnchorDist - height());
        break;

    case ANCHOR_NORTH_WEST:
        move(m_anchorPoint);
        break;

    case ANCHOR_NONE:
        move(geom.center().x() - size().width() / 2, geom.center().y() - size().height() / 2);
        break;
    }
}
/**
 * @brief popup 显示函数
 * @param msg 传入信息内容
 * @param flag 是否自动隐藏，默认为是
 */
void NotificationWidget::popup(const QString &msg, bool flag)
{
    m_pMainLayout->setContentsMargins(14, 4, 14, 4);
    if (m_pMainLayout->indexOf(m_pMsgLabel) == -1) {
        m_pMainLayout->addWidget(m_pMsgLabel);
    }
    setFixedHeight(30);
    m_pMsgLabel->setText(msg);
    show();
    raise();

    if (flag) {
        m_pTimer->start();
    }
}
/**
 * @brief updateWithMessage 更新显示信息
 * @param newMsg 显示信息
 * @param flag 是否自动隐藏，默认为是
 */
void NotificationWidget::updateWithMessage(const QString &newMsg, bool flag)
{
    if (m_pIconLabel) {
        m_pIconLabel->setVisible(false);
    }

    QFont font;
    font.setPixelSize(12);
    QFontMetrics fontMetrics(font);
    QString sMsg = fontMetrics.elidedText(newMsg, Qt::ElideMiddle, m_pMainWindow->width() - 12 - 12 - 60);

    if (isVisible()) {
        m_pMsgLabel->setText(sMsg);
        resize(m_pMsgLabel->sizeHint().width() + m_pMainLayout->contentsMargins().left()
               + m_pMainLayout->contentsMargins().right(), height());
        adjustSize();

        if (flag) {
            m_pTimer->start();
        }
        syncPosition();
    } else {
        popup(sMsg, flag);
    }
}
/**
 * @brief paintEvent 重载绘制事件函数
 * @param pPaintEvent 绘制事件
 */
void NotificationWidget::paintEvent(QPaintEvent *pPaintEvent)
{
    //参考设计图
    const float fRadius = 8;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool bLight = (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType());

    //按照ui建议，突出文字
    QColor color = QColor(42, 42, 42, 255 * 0.95);
    QColor borderColor = QColor(0, 0, 0, 255 * 0.3);

    if (bLight) {
        color = QColor(247, 247, 247, 255 * 0.95);
        borderColor = QColor(0, 0, 0, 255 * 0.05);
    }
    painter.fillRect(rect(), Qt::transparent);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect(), static_cast<qreal>(fRadius), static_cast<qreal>(fRadius));
        painter.setPen(borderColor);
        painter.drawPath(painterPath);
    }

    QRect viewRect = rect().marginsRemoved(QMargins(1, 1, 1, 1));
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect(), static_cast<qreal>(fRadius), static_cast<qreal>(fRadius));
    painter.fillPath(painterPath, color);

    QFrame::paintEvent(pPaintEvent);
}
/**
 * @brief initMember 初始化成员变量
 */
void NotificationWidget::initMember()
{
    m_pMsgLabel = nullptr;
    m_pIconLabel = nullptr;
    m_pTimer = nullptr;
    m_pFrame = nullptr;
    m_pMainLayout = nullptr;
    m_pAnchor = ANCHOR_NONE;
    m_nAnchorDist = 10;
    m_bIsWheel = true;
}

}
