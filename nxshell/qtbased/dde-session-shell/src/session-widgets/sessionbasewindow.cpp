// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionbasewindow.h"
#include "constants.h"

#include <QDebug>
#include <QResizeEvent>
#include <QApplication>

using namespace DDESESSIONCC;

SessionBaseWindow::SessionBaseWindow(QWidget *parent)
    : QFrame(parent)
    , m_TopFrame(new QFrame(this))
    , m_centerFrame(new QFrame(this))
    , m_bottomFrame(new QFrame(this))
    , m_fakeWindowLayer(new FakeWindowLayer(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_topLayout(new QHBoxLayout(this))
    , m_centerLayout(new QVBoxLayout(this))
    , m_leftBottomLayout(new QHBoxLayout(this))
    , m_centerBottomLayout(new QHBoxLayout(this))
    , m_rightBottomLayout(new QHBoxLayout(this))
    , m_topWidget(nullptr)
    , m_centerWidget(nullptr)
    , m_leftBottomWidget(nullptr)
    , m_centerBottomWidget(nullptr)
    , m_rightBottomWidget(nullptr)
    , m_centerSpacerItem(new QSpacerItem(0, 0))
{
    initUI();
}

void SessionBaseWindow::setLeftBottomWidget(QWidget * const widget)
{
    if (!widget)
        return;

    if (m_leftBottomWidget) {
        m_leftBottomLayout->removeWidget(m_leftBottomWidget);
        // TODO 是否要将m_leftBottomWidget的visible设置为false
    }

    m_leftBottomLayout->addWidget(widget, 0, Qt::AlignBottom);
    m_leftBottomWidget = widget;

#ifdef CONTRAST_BACKGROUND
    m_leftBottomWidget->setStyleSheet("background-color: darkRed");
#endif
}

void SessionBaseWindow::setCenterBottomWidget(QWidget *const widget)
{
    if (!widget)
        return;

    if (m_centerBottomWidget) {
        m_centerBottomLayout->removeWidget(m_centerBottomWidget);
    }

    m_centerBottomLayout->addWidget(widget, 0, Qt::AlignBottom | Qt::AlignHCenter);
    m_centerBottomWidget = widget;

#ifdef CONTRAST_BACKGROUND
    m_centerBottomWidget->setStyleSheet("background-color: darkGreen");
#endif
}

void SessionBaseWindow::setRightBottomWidget(QWidget * const widget)
{
    if (!widget)
        return;

    if (m_rightBottomWidget) {
        m_rightBottomLayout->removeWidget(m_rightBottomWidget);
    }

    m_rightBottomLayout->addWidget(widget, 0, Qt::AlignBottom);
    m_rightBottomWidget = widget;

#ifdef CONTRAST_BACKGROUND
    m_rightBottomWidget->setStyleSheet("background-color: darkBlue");
#endif
}

void SessionBaseWindow::setCenterContent(QWidget * const widget, Qt::Alignment align, int spacerHeight)
{
    if (!widget || m_centerWidget == widget) {
        return;
    }

    if (m_centerWidget) {
        m_centerLayout->removeWidget(m_centerWidget);
        m_centerWidget->hide();
    }

    m_centerSpacerItem->changeSize(0, spacerHeight);
    m_centerLayout->setAlignment(align);
    m_centerLayout->addWidget(widget);
    m_centerLayout->invalidate();
    m_centerLayout->update();

    m_centerWidget = widget;
    widget->show();

    setFocusProxy(widget);
    setFocus();
}

void SessionBaseWindow::setTopWidget(QWidget *const widget)
{
    if (!widget)
        return;

    if (m_topWidget) {
        m_topLayout->removeWidget(m_topWidget);
    }

    m_topLayout->addStretch();
    m_topLayout->addWidget(widget, 0, Qt::AlignTop);
    m_topLayout->addStretch();
    m_topWidget = widget;
}

void SessionBaseWindow::initUI()
{
    //整理代码顺序，让子部件层级清晰明了,
    //同时方便计算中间区域的大小,使用QFrame替换了QScrollArea
    m_topLayout->setMargin(0);
    m_topLayout->setSpacing(0);

    m_TopFrame->setAccessibleName("CenterTopFrame");
    m_TopFrame->setLayout(m_topLayout);
    m_TopFrame->setFixedHeight(autoScaledSize(LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT));
    m_TopFrame->setAutoFillBackground(false);

    m_centerLayout->setMargin(0);
    m_centerLayout->setSpacing(0);
    m_centerLayout->addSpacerItem(m_centerSpacerItem);
    m_centerLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    m_centerFrame->setAccessibleName("CenterFrame");
    m_centerFrame->setLayout(m_centerLayout);
    m_centerFrame->setAutoFillBackground(false);

    m_leftBottomLayout->setMargin(0);
    m_leftBottomLayout->setSpacing(0);
    m_centerBottomLayout->setMargin(0);
    m_centerBottomLayout->setSpacing(0);
    m_rightBottomLayout->setMargin(0);
    m_rightBottomLayout->setSpacing(0);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setMargin(0);
    bottomLayout->setSpacing(0);
    bottomLayout->addLayout(m_leftBottomLayout, 3);
    bottomLayout->addLayout(m_centerBottomLayout, 2);
    bottomLayout->addLayout(m_rightBottomLayout, 3);

    m_bottomFrame->setAccessibleName("BottomFrame");
    m_bottomFrame->setLayout(bottomLayout);
    m_bottomFrame->setFixedHeight(LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT);
    m_bottomFrame->setAutoFillBackground(false);

    m_mainLayout->setContentsMargins(getMainLayoutMargins());
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_TopFrame);
    m_mainLayout->addWidget(m_centerFrame);
    m_mainLayout->addWidget(m_bottomFrame);

#ifdef CONTRAST_BACKGROUND
    m_TopFrame->setStyleSheet("background-color: darkGray");
    m_bottomFrame->setStyleSheet("background-color: gray");
#endif
    setLayout(m_mainLayout);
    m_fakeWindowLayer->setFixedSize(size());
    m_fakeWindowLayer->setAccessibleName("FakeWindowLayer");
}

QSize SessionBaseWindow::getCenterContentSize()
{
    //计算中间区域的大小
    int w = width() - m_mainLayout->contentsMargins().left() - m_mainLayout->contentsMargins().right();

    int h = height() - m_mainLayout->contentsMargins().top() - m_mainLayout->contentsMargins().bottom();
    if (m_TopFrame->isVisible()) {
        h = h - m_TopFrame->height();
    }
    if (m_bottomFrame->isVisible()) {
        h = h - m_bottomFrame->height();
    }

    return QSize(w, h);
}

void SessionBaseWindow::changeCenterSpaceSize(int w, int h)
{
    m_centerSpacerItem->changeSize(w, h);

    m_centerLayout->invalidate();
    m_centerLayout->update();
}

void SessionBaseWindow::resizeEvent(QResizeEvent *event)
{
    m_mainLayout->setContentsMargins(getMainLayoutMargins());
    m_TopFrame->setFixedHeight(autoScaledSize(LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT));
    m_bottomFrame->setFixedHeight(LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT);
    m_fakeWindowLayer->setFixedSize(size());

    QFrame::resizeEvent(event);
}

void SessionBaseWindow::setTopFrameVisible(bool visible)
{
    m_TopFrame->setVisible(visible);
}

void SessionBaseWindow::setBottomFrameVisible(bool visible)
{
    m_bottomFrame->setVisible(visible);
}

void SessionBaseWindow::showPopup(QPoint globalPos, QWidget *popup)
{
    m_fakeWindowLayer->setContent(popup);
    QPoint localPos = m_fakeWindowLayer->mapFromGlobal(globalPos);
    popup->move(localPos);
    m_fakeWindowLayer->raise();
    m_fakeWindowLayer->show();
}

void SessionBaseWindow::hidePopup()
{
    m_fakeWindowLayer->hide();
}

void SessionBaseWindow::togglePopup(QPoint globalPos, QWidget *popup)
{
    if (m_fakeWindowLayer->isVisible())
        hidePopup();
    else
        showPopup(globalPos, popup);
}

/**
 * @brief SessionBaseWindow::autoScaledSize
 * @param height
 * @return 自动根据当前屏幕的高度，对height进行缩放，但返回值不会大于height
 */
int SessionBaseWindow::autoScaledSize(const int height) const
{
    int h = static_cast<int>(((double) height / (double) BASE_SCREEN_HEIGHT) * topLevelWidget()->geometry().height());
    return qMin(h, height);
}

/**
 * @brief SessionBaseWindow::getMainLayoutMargins
 * @return 返回主界面布局的边距信息
 */
QMargins SessionBaseWindow::getMainLayoutMargins() const
{
    // margin占高度的33/1080,且最大为33
    int margin = static_cast<int>(((double) LOCK_CONTENT_CENTER_LAYOUT_MARGIN / (double) BASE_SCREEN_HEIGHT) * topLevelWidget()->geometry().height());
    margin = qMin(margin, LOCK_CONTENT_CENTER_LAYOUT_MARGIN);

    return QMargins(0, margin, 0, margin);
}
