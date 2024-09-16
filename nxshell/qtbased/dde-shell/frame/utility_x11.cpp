// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/utility_x11_p.h"

#include <QLoggingCategory>
#include <QMouseEvent>

#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

DS_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

static X11Utility *instance()
{
    return dynamic_cast<X11Utility*>(Utility::instance());
}

X11Utility::X11Utility()
{
    if (auto x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
        m_display = x11Application->display();
}

_XDisplay *X11Utility::getDisplay()
{
    return m_display;
}

void X11Utility::deliverMouseEvent(uint8_t qMouseButton)
{
    uint8_t mouseButton = XCB_BUTTON_INDEX_1;
    switch (qMouseButton) {
    case Qt::MiddleButton:
        mouseButton = XCB_BUTTON_INDEX_2;
        break;
    case Qt::RightButton:
        mouseButton = XCB_BUTTON_INDEX_3;
        break;
    }

    auto dis = getDisplay();
    if (!dis)
        return;
    XTestFakeRelativeMotionEvent(dis, 0, 0, 0);
    XFlush(dis);
    XTestFakeButtonEvent(dis, mouseButton, true, 0);
    XFlush(dis);
    XTestFakeButtonEvent(dis, mouseButton, false, 0);
    XFlush(dis);
}

bool X11Utility::grabKeyboard(QWindow *target, bool grab)
{
    if (target) {
        qCDebug(dsLog) << "grab keyboard for the window:" << target->winId();
        return target->setKeyboardGrabEnabled(grab);
    }
    return false;
}

bool X11Utility::grabMouse(QWindow *target, bool grab)
{
    if (grab && MouseGrabEventFilter::isMatchingWindow(target)) {
        qCDebug(dsLog) << "tray to grab mouse for the window:" << target->winId();
        auto filter = new MouseGrabEventFilter(target);
        target->installEventFilter(filter);
        QObject::connect(filter, &MouseGrabEventFilter::outsideMousePressed, target, [this, filter, target] () {
            qCDebug(dsLog) << "clicked outside mouse for the window:" << target->winId();
            target->setMouseGrabEnabled(false);
            target->removeEventFilter(filter);
            filter->closeAllWindow();
            filter->deleteLater();
        });
        return filter->tryGrabMouse();
    }
    if (target)
        return target->setMouseGrabEnabled(grab);
    return false;
}

MouseGrabEventFilter::MouseGrabEventFilter(QWindow *target)
    : QObject (target)
    , m_target(target)
{

}

bool MouseGrabEventFilter::tryGrabMouse()
{
    if (isMainWindow())
        return m_target->setMouseGrabEnabled(true);

    return true;
}

bool MouseGrabEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_target)
        return false;
    if (event->type() == QEvent::MouseButtonRelease) {
        mousePressEvent(static_cast<QMouseEvent *>(event));
    } else if (event->type() == QEvent::Close) {
        m_target->removeEventFilter(this);
        this->deleteLater();
    } else if (event->type() == QEvent::Leave) {
        if (!isMainWindow()) {
            m_target->setMouseGrabEnabled(false);
            if (auto parent = mainWindow()) {
                qCDebug(dsLog) << "grab mouse for main window:" << parent->winId();
                parent->setMouseGrabEnabled(true);
            }
        }
    } else if (event->type() == QEvent::MouseMove) {
        if (isMainWindow()) {
            trySelectGrabWindow(static_cast<QMouseEvent *>(event));
        }
    }
    return false;
}

void MouseGrabEventFilter::mousePressEvent(QMouseEvent *e)
{
    const auto bounding = m_target->geometry();
    const auto pos = e->globalPosition();
    if ((e->position().toPoint().isNull() && !pos.isNull()) ||
        !bounding.contains(pos.toPoint())) {
        instance()->deliverMouseEvent(e->button());
        emit outsideMousePressed();
        return;
    }
}

bool MouseGrabEventFilter::trySelectGrabWindow(QMouseEvent *e)
{
    const auto bounding = m_target->geometry();
    auto pos = QPointF(e->globalPosition());
    if ((e->position().toPoint().isNull() && !pos.isNull()) ||
        !bounding.contains(pos.toPoint())) {
        for (auto item : Utility::instance()->allChildrenWindows(m_target)) {
            if (!item->isVisible() || isMainWindow(item))
                continue;
            if (item->geometry().contains(pos.toPoint())) {
                qCDebug(dsLog) << "grab mouse for the window:" << item->winId();
                m_target->setMouseGrabEnabled(false);
                item->setMouseGrabEnabled(true);
                return true;
            }
        }
    }
    return false;
}

QWindow *MouseGrabEventFilter::mainWindow() const
{
    return mainWindow(m_target);
}

bool MouseGrabEventFilter::isMainWindow() const
{
    return isMainWindow(m_target);
}

QWindow *MouseGrabEventFilter::mainWindow(QWindow *target)
{
    return target ? target->property("mainMenuWindow").value<QWindow *>() : nullptr;
}

bool MouseGrabEventFilter::isMainWindow(QWindow *target)
{
    return !mainWindow(target);
}

// distinguish MenuWindow with NormalWindow
bool MouseGrabEventFilter::isMatchingWindow(QWindow *target)
{
    return target && target->property("mainMenuWindow").isValid();
}

void MouseGrabEventFilter::closeAllWindow()
{
    if (auto window = mainWindow()) {
        for (auto item : Utility::instance()->allChildrenWindows(window)) {
            if (item && item->isVisible())
                item->close();
        }
    }
    if (m_target && m_target->isVisible())
        m_target->close();

}

DS_END_NAMESPACE
