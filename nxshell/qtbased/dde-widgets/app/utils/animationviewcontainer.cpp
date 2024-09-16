// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "animationviewcontainer.h"
#include "utils.h"
#include "global.h"
#include <widgetsinterface.h>

#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QDBusInterface>
#include <QPropertyAnimation>
#include <QPainter>

static QColor outerBorderColor = QColor(0, 0, 0, static_cast<int>(0.15 * 255));
static QColor innerBorderColor = QColor(255, 255, 255, static_cast<int>(0.2 * 255));
#define ALPHA_OFFSET 10

WIDGETS_USE_NAMESPACE
DGUI_USE_NAMESPACE
WIDGETS_FRAME_BEGIN_NAMESPACE
AnimationViewContainer::AnimationViewContainer(QWidget *parent)
    : DBlurEffectWidget (parent)
    , m_currentXAni(new QPropertyAnimation(this, "currentX"))
    , m_windowHandle(new DPlatformWindowHandle(this, this))
{
    resize(0, 0); // Should set size explicitly for region monitor to work correctly.
    setWindowFlags(Qt::Tool);
    m_cornerRadius = m_windowHandle->windowRadius();
    m_themeType= DGuiApplicationHelper::instance()->themeType();
    m_windowHandle->setBorderWidth(1);

    m_currentXAni->setDuration(300);
    m_currentXAni->setEasingCurve(QEasingCurve::OutQuart);

    auto setOuterBorderColor = [this]() {
        auto outerBorderNewColor = outerBorderColor;
        if (m_themeType == DGuiApplicationHelper::ColorType::DarkType) {
            outerBorderNewColor.setAlpha(maskAlpha() + ALPHA_OFFSET * 2);
        }

        m_windowHandle->setBorderColor(outerBorderNewColor);
    };

    connect(this, &DBlurEffectWidget::maskAlphaChanged, [this, setOuterBorderColor]() {
        setOuterBorderColor();
        update();
    });

    connect(m_windowHandle, &DPlatformWindowHandle::windowRadiusChanged, this, [this](){
        if (m_cornerRadius == m_windowHandle->windowRadius())
            return;

        m_cornerRadius = m_windowHandle->windowRadius();
        update();
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [this, setOuterBorderColor](DGuiApplicationHelper::ColorType type){
        if (!m_windowHandle || type == m_themeType)
            return;

        m_themeType = type;
        setOuterBorderColor();
        update();
    });

    setOuterBorderColor();
}

AnimationViewContainer::~AnimationViewContainer()
{
}

void AnimationViewContainer::registerRegion()
{
    if (!m_regionMonitor) {
        m_regionMonitor = new DRegionMonitor(this);
        m_regionMonitor->setCoordinateType(DRegionMonitor::Original);
        connect(m_regionMonitor, &DRegionMonitor::buttonRelease, this, &AnimationViewContainer::regionMonitorHide, Qt::UniqueConnection);
    }

    if (!m_regionMonitor->registered()) {
        m_regionMonitor->registerRegion();
    }
}

void AnimationViewContainer::unRegisterRegion()
{
    if (nullptr == m_regionMonitor)
        return;
    if (m_regionMonitor->registered()) {
        m_regionMonitor->unregisterRegion();
    }
}

void AnimationViewContainer::regionMonitorHide(const QPoint &p, const int flag)
{
    // avoid to being hiden when app still is active.
    if (qApp->activeWindow() || flag >= DRegionMonitor::Wheel_Up)
        return;

    auto m_scale = qApp->primaryScreen()->devicePixelRatio();
    QPoint pScale(int(qreal(p.x() / m_scale)), int(qreal(p.y() / m_scale)));
    if (!geometry().contains(pScale)){
        if (releaseMode()) {
            Q_EMIT outsideAreaReleased();
        }
    }
}

void AnimationViewContainer::showView()
{
    setCurrentX(m_targetRect.left());
    show();
    activateWindow();
    registerRegion();
}

void AnimationViewContainer::hideView()
{
    unRegisterRegion();
    hide();
}

void AnimationViewContainer::refreshView()
{
    if (m_currentXAni && m_currentXAni->state() == QAbstractAnimation::Running) {
        m_currentXAni->stop();
    }
    if (isVisible()) {
        showView();
    } else {
        hideView();
    }
}

void AnimationViewContainer::updateGeometry(const QRect &rect)
{
    m_currentXAni->stop();
    m_currentXAni->setStartValue(m_targetRect.left());
    m_currentXAni->setEndValue(rect.left());
    m_targetRect = rect;
    if (isVisible()) {
        m_currentXAni->start();
    }
}

qreal AnimationViewContainer::currentX() const
{
    return QWidget::x();
}

void AnimationViewContainer::setCurrentX(const qreal x)
{
    auto rect = m_targetRect;
    rect -= QMargins(0, UI::Widget::WindowMargin, 0, UI::Widget::WindowMargin);

    rect.setLeft(x);
    rect.setWidth(m_targetRect.right() - x);
    setGeometry(rect.toRect());
}

void AnimationViewContainer::paintEvent(QPaintEvent *e)
{
    DBlurEffectWidget::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(1);

    auto innerBorderNewColor = innerBorderColor;
    if (m_themeType != DGuiApplicationHelper::DarkType) {
        innerBorderNewColor.setAlpha(maskAlpha() + ALPHA_OFFSET);
    }

    pen.setColor(innerBorderNewColor);
    p.setPen(pen);
    p.drawRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
}

WIDGETS_FRAME_END_NAMESPACE
