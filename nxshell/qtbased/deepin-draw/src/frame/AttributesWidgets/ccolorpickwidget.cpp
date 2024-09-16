// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ccolorpickwidget.h"
#include "colorpanel.h"
#include "application.h"

//#include "application.h"
//#include "mainwindow.h"
//#include "toptoolbar.h"
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
const int RADIUS_VALUE = 18;
const int ARROW_WIDTH = 40;
const int ARROW_HEIGHT = 30;
CColorPickWidget::CColorPickWidget(QWidget *parent)
    : DArrowRectangle(ArrowTop, FloatWindow, parent)
{
    setWgtAccesibleName(this, "ColorPickWidget");
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::/*Popup*/Popup);
    this->setAttribute(Qt::WA_TranslucentBackground);
    setRadiusArrowStyleEnable(true);
    this->setArrowWidth(ARROW_WIDTH);
    this->setArrowHeight(ARROW_HEIGHT);
    m_colorPanel = new ColorPanel(this);
    m_colorPanel->setFocusPolicy(Qt::NoFocus);
    this->setFocusPolicy(Qt::NoFocus);
    //this->setContent(m_colorPanel);
    this->hide();

    setRadius(RADIUS_VALUE);
    //connect(m_colorPanel, &ColorPanel::colorChanged, this, &CColorPickWidget::colorChanged);
    connect(m_colorPanel, &ColorPanel::colorChanged, this, [ = ](const QColor & color, EChangedPhase phase) {
        qDebug() << "color ===== " << color << "phase = " << phase;
        if (_color != color || _phase != phase) {
            _color = color;
            _phase = phase;
            emit colorChanged(color, phase);
        }
    });

//    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasBlurWindowChanged, this, [ = ]() {
//        bool hasBlur = DWindowManagerHelper::instance()->hasBlurWindow();

//        if (hasBlur) {
//            m_colorPanel->setWindowFlag(Qt::Widget);
//            m_colorPanel->setParent(this);
//            setContent(m_colorPanel);
//            this->setContent(m_colorPanel);
//        } else {
//            m_colorPanel->setWindowFlag(Qt::Popup);
//            this->hide();
//        }
//    });
}

QColor CColorPickWidget::color()
{
    return _color;
}

ColorPanel *CColorPickWidget::colorPanel()
{
    return m_colorPanel;
}

QWidget *CColorPickWidget::caller()
{
    return _caller;
}

void CColorPickWidget::setCaller(QWidget *pCaller)
{
    _caller = pCaller;
}

void CColorPickWidget::setExpandWidgetVisble(bool visble)
{
    colorPanel()->setExpandWidgetVisble(visble);
}

void CColorPickWidget::show(int x, int y)
{
    //if (parentWidget() == nullptr || isWindowType())
    {
        //DPlatformWindowHandle hander(this);
        //hander.setEnableBlurWindow(false);
        //QPainterPath path; path.addRect(this->rect());
        //hander.setClipPath(path);
        //hander.setFrameMask(QRegion(0, 0, width(), height()));
        //this->setArrowWidth(0);
        //this->setArrowHeight(0);

        if (DWindowManagerHelper::instance()->hasBlurWindow()) {
            m_colorPanel->setWindowFlags(Qt::Widget);
            m_colorPanel->setParent(this);
            setContent(m_colorPanel);
            return DArrowRectangle::show(x, y);
        } else {
            setContent(nullptr);
            this->hide();
            //DPlatformWindowHandle hander(m_colorPanel);
            m_colorPanel->setWindowFlag(Qt::Popup);
//            m_colorPanel->show();
//            QMetaObject::invokeMethod(this, [ = ]() {
//                m_colorPanel->move(x - m_colorPanel->width() / 2, y);
//            }, Qt::QueuedConnection);
            m_colorPanel->move(x - m_colorPanel->sizeHint().width() / 2, y);
            m_colorPanel->show();
            return;
        }

        //Dtk::Widget::DPlatformWindowHandle
    }
    //QPoint pos = this->parentWidget()->mapFromGlobal(QPoint(x, y));
    //DArrowRectangle::show(pos.x(), pos.y());
}

void CColorPickWidget::setColor(const QColor &c)
{
    m_colorPanel->setColor(c, false);
    _color = c;
    _phase = EChanged;
}

void CColorPickWidget::setTheme(int theme)
{
    m_colorPanel->setTheme(theme);
}
void CColorPickWidget::mousePressEvent(QMouseEvent *event)
{
    //event->accept();

    return DArrowRectangle::mousePressEvent(event);
}

bool CColorPickWidget::event(QEvent *e)
{
//    if (e->type() == QEvent::MouseMove) {
//        return  true;
//    }

    if (e->type() == QEvent::Hide) {
        m_colorPanel->hide();
    } else if (e->type() == QEvent::Show) {
        m_colorPanel->show();
    }

    return DArrowRectangle::event(e);
}
