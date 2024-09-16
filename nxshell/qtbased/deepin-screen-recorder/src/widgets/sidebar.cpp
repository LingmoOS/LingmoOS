// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebar.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSettings>
#include <QVBoxLayout>
#include <QBitmap>

#include <dgraphicsgloweffect.h>


DWIDGET_USE_NAMESPACE

namespace {

//const QSize TOOLBAR_WIDGET_SIZE = QSize(66, 465);
//不同图形下二级菜单的大小及长度不一样
const QSize TOOLBAR_WIDGET_SIZE1 = QSize(290, 68);
const QSize TOOLBAR_WIDGET_SIZE2 = QSize(370, 68);
const QSize TOOLBAR_WIDGET_SIZE3 = QSize(500, 68);

//const int BUTTON_SPACING = 3;
//const int BTN_RADIUS = 3;
const QSize SPLITTER_SIZE = QSize(3, 30);

}


SideBarWidget::SideBarWidget(MainWindow *pmainwindow, DWidget *parent)
    : DFloatingWidget(parent)
    , m_expanded(false)
{
    m_pMainWindow = pmainwindow;
    initSideBarWidget();
}

SideBarWidget::~SideBarWidget()
{

}

void SideBarWidget::initSideBarWidget()
{

//    this->setAttribute(Qt::WA_StyledBackground, true);
//    this->setStyleSheet("background-color: rgb(255,0, 0)");
    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    if (Utils::themeType == 1) {
        blurBackground()->setMaskColor(QColor(0xF7, 0xF7, 0xF7, 153));
    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }
    //设置侧边栏大小
    //setFixedSize(TOOLBAR_WIDGET_SIZE);

    //分割线
    m_seperator = new DVerticalLine(this);
    //m_seperator->setDisabled(true);
    m_seperator->setFixedSize(SPLITTER_SIZE);

    m_colorTool = new ColorToolWidget(this);
    m_shotTool = new ShotToolWidget(m_pMainWindow, this);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 0, 10, 0);
    hLayout->addWidget(m_shotTool);
    hLayout->addWidget(m_seperator);
    hLayout->addWidget(m_colorTool);
    setLayout(hLayout);

    setMinimumSize(TOOLBAR_WIDGET_SIZE1);

    connect(m_shotTool, &ShotToolWidget::changeArrowAndLine, this, &SideBarWidget::changeArrowAndLineEvent);
    connect(m_colorTool, &ColorToolWidget::colorChecked, m_shotTool, &ShotToolWidget::colorChecked);

}
void SideBarWidget::changeShotToolWidget(const QString &func)
{
    qDebug() << __FUNCTION__ << __LINE__ << func;
    if (func == "effect") {
        m_seperator->hide();
    } else {
        m_seperator->show();
    }

    //不同图形下二级菜单的大小及长度不一样
    if (func == "rectangle" ||
            func == "oval" ||
            func == "line" ||
            func == "arrow" ||
            func == "pen") {
        resize(TOOLBAR_WIDGET_SIZE1);
    } else if (func == "text") {
        resize(TOOLBAR_WIDGET_SIZE2);
    } else if (func == "effect") {
        resize(TOOLBAR_WIDGET_SIZE3);
    }
    m_shotTool->switchContent(func);
    m_colorTool->setFunction(func);
}

int SideBarWidget::getSideBarWidth(const QString &func)
{
    int width = TOOLBAR_WIDGET_SIZE1.width();
    if (func == "rectangle" ||
            func == "oval" ||
            func == "line" ||
            func == "arrow" ||
            func == "pen") {
        resize(TOOLBAR_WIDGET_SIZE1);
        width = TOOLBAR_WIDGET_SIZE1.width();
    } else if (func == "text") {
        width = TOOLBAR_WIDGET_SIZE2.width();
    } else if (func == "effect") {
        width = TOOLBAR_WIDGET_SIZE3.width();
    }
    return width;
}

void SideBarWidget::paintEvent(QPaintEvent *e)
{
    DFloatingWidget::paintEvent(e);
}

void SideBarWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    DFloatingWidget::showEvent(event);
}



SideBar::SideBar(DWidget *parent) : DLabel(parent)
    , m_sidebarWidget(nullptr), m_expanded(false)
{
}

SideBar::~SideBar()
{

}

void SideBar::changeShotToolFunc(const QString &func)
{
    m_sidebarWidget->changeShotToolWidget(func);
    resize(m_sidebarWidget->size());
}

int SideBar::getSideBarWidth(const QString &func)
{
    return m_sidebarWidget->getSideBarWidth(func);
}

bool SideBar::isDraged()
{
    return m_isDrag;
}

bool SideBar::isPressed()
{
    return m_isPress;
}

void SideBar::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();

    move(pos.x(), pos.y());
}

void SideBar::initSideBar(MainWindow *pmainWindow)
{
    m_pMainWindow = pmainWindow;
    m_sidebarWidget = new SideBarWidget(pmainWindow, this);
    resize(m_sidebarWidget->size());

    connect(m_sidebarWidget, &SideBarWidget::changeArrowAndLineEvent, this, &SideBar::changeArrowAndLineToMain);
    connect(m_sidebarWidget, &SideBarWidget::closeSideBar, this, [ = ] { this->hide(); });
}
/*
 * never used
void SideBar::setColorFunc(const QString &func)
{
    m_sidebarWidget->changeShotToolWidget(func);
}
*/
void SideBar::paintEvent(QPaintEvent *e)
{
    DLabel::paintEvent(e);
}

void SideBar::enterEvent(QEvent *e)
{
//    qApp->setOverrideCursor(Qt::ArrowCursor);
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
    DLabel::enterEvent(e);
}

bool SideBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::PaletteChange) {
        qDebug() << obj;
        qDebug() << "--------------";
    }

    return DLabel::eventFilter(obj, event);
}

void SideBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        m_isPress = true;
        //获得鼠标的初始位置
        m_mouseStartPoint = event->globalPos();
        //获得窗口的初始位置
        m_windowStartPoint = this->frameGeometry().topLeft();
        m_pMainWindow->getToolBarStartPressPoint();
    }
}

void SideBar::mouseMoveEvent(QMouseEvent *event)
{
    //判断是否在拖拽移动
    if (m_isPress) {
        m_isDrag = true;
        //获得鼠标移动的距离
        QPoint move_distance = event->globalPos() - m_mouseStartPoint;
        //改变窗口的位置
        if (m_pMainWindow) {
            m_pMainWindow->moveToolBars(m_windowStartPoint, move_distance);
        }
    }
}

void SideBar::mouseReleaseEvent(QMouseEvent *event)
{
    //放下左键即停止移动
    if (event->button() == Qt::LeftButton) {
        m_isPress = false;
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
    }
}
