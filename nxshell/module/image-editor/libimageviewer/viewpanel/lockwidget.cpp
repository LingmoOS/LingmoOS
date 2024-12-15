// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockwidget.h"

#include <QVBoxLayout>
#include <DGuiApplicationHelper>
#include <QGestureEvent>
#include <QDebug>

#include "unionimage/baseutils.h"

#include "accessibility/ac-desktop-define.h"

const QString ICON_PIXMAP_DARK = ":/dark/images/picture damaged_dark.svg";
const QString ICON_PIXMAP_LIGHT = ":/light/images/picture damaged_light.svg";
const QSize THUMBNAIL_SIZE = QSize(151, 151);
LockWidget::LockWidget(const QString &darkFile,
                       const QString &lightFile, QWidget *parent)
    : ThemeWidget(darkFile, lightFile, parent),
      m_picString("")
{
    setMouseTracking(true);
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);
    //修复style问题
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        m_picString = ICON_PIXMAP_DARK;
        m_theme = true;
    } else {
        m_picString = ICON_PIXMAP_LIGHT;
        m_theme = false;
    }
    m_bgLabel = new DLabel(this);
    m_bgLabel->setFixedSize(151, 151);
    m_bgLabel->setObjectName("BgLabel");
#ifdef OPENACCESSIBLE
    setObjectName(Lock_Widget);
    setAccessibleName(Lock_Widget);
    m_bgLabel->setAccessibleName("BgLabel");
#endif
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ]() {
        DGuiApplicationHelper::ColorType themeType =
            DGuiApplicationHelper::instance()->themeType();
        m_picString = "";
        if (themeType == DGuiApplicationHelper::DarkType) {
            m_picString = ICON_PIXMAP_DARK;
            m_theme = true;
        } else {
            m_picString = ICON_PIXMAP_LIGHT;
            m_theme = false;
        }

        QPixmap logo_pix = Libutils::base::renderSVG(m_picString, THUMBNAIL_SIZE);
        if (m_bgLabel) {
            m_bgLabel->setPixmap(logo_pix);
        }
    });
    m_lockTips = new DLabel(this);
    m_lockTips->setObjectName("LockTips");
    m_lockTips->setVisible(false);
    setContentText(tr("You have no permission to view the image"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    QPixmap logo_pix = Libutils::base::renderSVG(m_picString, THUMBNAIL_SIZE);
    m_bgLabel->setPixmap(logo_pix);
    layout->addWidget(m_bgLabel, 0, Qt::AlignHCenter);
    //layout->addSpacing(18);
    //layout->addWidget(m_lockTips, 0, Qt::AlignHCenter);
    layout->addStretch(1);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged, this,
            &LockWidget::onThemeChanged);
}

void LockWidget::setContentText(const QString &text)
{
    m_lockTips->setText(text);
    int textHeight = Libutils::base::stringHeight(m_lockTips->font(),
                                                  m_lockTips->text());
    m_lockTips->setMinimumHeight(textHeight + 2);
}

void LockWidget::handleGestureEvent(QGestureEvent *gesture)
{
    /*    if (QGesture *swipe = gesture->gesture(Qt::SwipeGesture))
            swipeTriggered(static_cast<QSwipeGesture *>(swipe));
        else */if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
}

void LockWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    //非平板才能双击,其他是单击全屏
    if (e->button() == Qt::LeftButton)
        emit showfullScreen();
    ThemeWidget::mouseDoubleClickEvent(e);
}

void LockWidget::mouseReleaseEvent(QMouseEvent *e)
{
#if 0
    //平板单击全屏需求
    if (dApp->isPanelDev()) {
        int xpos = e->globalPos().x() - m_startx;
        if ((QDateTime::currentMSecsSinceEpoch() - m_clickTime) < 200 && abs(xpos) < 50) {
            m_clickTime = QDateTime::currentMSecsSinceEpoch();
            emit showfullScreen();
        }
    }
#endif
    QWidget::mouseReleaseEvent(e);
    if (e->source() == Qt::MouseEventSynthesizedByQt && m_maxTouchPoints == 1) {
        int offset = e->globalPos().x() - m_startx;
        if (qAbs(offset) > 200) {
            if (offset > 0) {
                emit previousRequested();
                qDebug() << "zy------ThumbnailWidget::event previousRequested";
            } else {
                emit nextRequested();
                qDebug() << "zy------ThumbnailWidget::event nextRequested";
            }
        }
    }
    m_startx = 0;
}

void LockWidget::mousePressEvent(QMouseEvent *e)
{
#if 0
    //平板单击全屏需求(暂时屏蔽)
    if (dApp->isPanelDev()) {
        m_clickTime = QDateTime::currentMSecsSinceEpoch();
    }
#endif
    QWidget::mousePressEvent(e);
    m_startx = e->globalPos().x();
}

void LockWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit sigMouseMove();
}

bool LockWidget::event(QEvent *event)
{
    QEvent::Type evType = event->type();
    if (evType == QEvent::TouchBegin || evType == QEvent::TouchUpdate ||
            evType == QEvent::TouchEnd) {
        if (evType == QEvent::TouchBegin) {
            qDebug() << "QEvent::TouchBegin";
            m_maxTouchPoints = 1;
        }
    } else if (event->type() == QEvent::Gesture)
        handleGestureEvent(static_cast<QGestureEvent *>(event));
    return QWidget::event(event);
}

void LockWidget::pinchTriggered(QPinchGesture *gesture)
{
    Q_UNUSED(gesture);
    m_maxTouchPoints = 2;
}

void LockWidget::onThemeChanged(DGuiApplicationHelper::ColorType theme)
{
    ThemeWidget::onThemeChanged(theme);
    update();
}

LockWidget::~LockWidget()
{
    if (m_bgLabel) {
        m_bgLabel->deleteLater();
        m_bgLabel = nullptr;
    }
    if (m_lockTips) {
        m_lockTips->deleteLater();
        m_lockTips = nullptr;
    }
}

void LockWidget::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier)) {
        if (event->delta() > 0) {
            emit previousRequested();
        } else if (event->delta() < 0) {
            emit nextRequested();
        }
        qDebug() << "control++";

    }
}
