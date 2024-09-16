// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnotitlebarwindowhelper_wl.h"
#include "vtablehook.h"

#define protected public
#include <QWindow>
#undef protected
#include <QMouseEvent>
#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>
#include <QMetaProperty>
#include <QScreen>
#include <qpa/qplatformwindow.h>
#include <QtWaylandClientVersion>

#define private public
#include "QtWaylandClient/private/qwaylandintegration_p.h"
#include "QtWaylandClient/private/qwaylandshellsurface_p.h"
#include "QtWaylandClient/private/qwaylandwindow_p.h"
#include "QtWaylandClient/private/qwaylandcursor_p.h"
#undef private

#include <private/qguiapplication_p.h>

DPP_BEGIN_NAMESPACE

QHash<const QWindow*, DNoTitlebarWlWindowHelper*> DNoTitlebarWlWindowHelper::mapped;

DNoTitlebarWlWindowHelper::DNoTitlebarWlWindowHelper(QWindow *window)
    : QObject(window)
    , m_window(window)
{
    // 不允许设置窗口为无边框的
    if (window->flags().testFlag(Qt::FramelessWindowHint)) {
        window->setFlag(Qt::FramelessWindowHint, false);
    }

    mapped[window] = this;

    updateEnableSystemMoveFromProperty();
}

DNoTitlebarWlWindowHelper::~DNoTitlebarWlWindowHelper()
{
    if (VtableHook::hasVtable(m_window)) {
        VtableHook::resetVtable(m_window);
    }

    mapped.remove(qobject_cast<QWindow*>(parent()));

    // TODO
    // if (m_window->handle()) { // 当本地窗口还存在时，移除设置过的窗口属性
    //     //! Utility::clearWindowProperty(m_windowID, Utility::internAtom(_DEEPIN_SCISSOR_WINDOW));
    //     DPlatformIntegration::clearNativeSettings(m_windowID);
    // }
}

void DNoTitlebarWlWindowHelper::setWindowProperty(QWindow *window, const char *name, const QVariant &value)
{
    const QVariant &old_value = window->property(name);

    if (old_value.isValid() && old_value == value)
        return;

    if (value.typeName() == QByteArray("QPainterPath")) {
        const QPainterPath &old_path = qvariant_cast<QPainterPath>(old_value);
        const QPainterPath &new_path = qvariant_cast<QPainterPath>(value);

        if (old_path == new_path) {
            return;
        }
    }

    if(!window)
        return;

    window->setProperty(name, value);

    if(window->handle()) {
        QtWaylandClient::QWaylandWindow *wl_window = static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());

        if (wl_window->shellSurface())
            wl_window->sendProperty(name, value);
    }

    if (DNoTitlebarWlWindowHelper *self = mapped.value(window)) {

        QByteArray name_array(name);
        if (!name_array.startsWith("_d_"))
            return;

        // to upper
        name_array[3] = name_array.at(3) & ~0x20;

        const QByteArray slot_name = "update" + name_array.mid(3) + "FromProperty";

        if (self->metaObject()->indexOfSlot(slot_name + QByteArray("()")) < 0)
            return;

        if (!QMetaObject::invokeMethod(self, slot_name.constData(), Qt::DirectConnection)) {
            qWarning() << "Failed to update property:" << slot_name;
        }
    }
}

void DNoTitlebarWlWindowHelper::requestByWindowProperty(QWindow *window, const char *name)
{
    if (window && window->handle()) {
        QtWaylandClient::QWaylandWindow *wl_window = static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());

        if (wl_window && wl_window->shellSurface())
            wl_window->sendProperty(name, QVariant());
    }
}

void DNoTitlebarWlWindowHelper::popupSystemWindowMenu(quintptr wid)
{
    QWindow *window = fromQtWinId(wid);
    if(!window || !window->handle())
        return;

    QtWaylandClient::QWaylandWindow *wl_window = static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (!wl_window->shellSurface())
        return;

    if (QtWaylandClient::QWaylandShellSurface *s = wl_window->shellSurface()) {
        auto wl_integration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
        s->showWindowMenu(wl_integration->display()->defaultInputDevice());
    }
}


void DNoTitlebarWlWindowHelper::updateEnableSystemMoveFromProperty()
{
    const QVariant &v = m_window->property(enableSystemMove);

    m_enableSystemMove = !v.isValid() || v.toBool();

    if (m_enableSystemMove) {
        HookOverride(m_window, &QWindow::event, &DNoTitlebarWlWindowHelper::windowEvent);
    } else if (VtableHook::hasVtable(m_window)) {
        HookReset(m_window, &QWindow::event);
    }
}

bool DNoTitlebarWlWindowHelper::windowEvent(QWindow *w, QEvent *event)
{
    DNoTitlebarWlWindowHelper *self = mapped.value(w);

    if (!self)
        return VtableHook::callOriginalFun(w, &QWindow::event, event);
    // m_window 的 event 被 override 以后，在 windowEvent 里面获取到的 this 就成 m_window 了，
    // 而不是 DNoTitlebarWlWindowHelper，所以此处 windowEvent 改为 static 并传 self 进来
    {
        // get touch begin position
        static bool isTouchDown = false;
        static QPointF touchBeginPosition;
        if (event->type() == QEvent::TouchBegin) {
            isTouchDown = true;
        }
        if (event->type() == QEvent::TouchEnd || event->type() == QEvent::MouseButtonRelease) {
            isTouchDown = false;
        }
        if (isTouchDown && event->type() == QEvent::MouseButtonPress) {
            touchBeginPosition = static_cast<QMouseEvent*>(event)->globalPos();
        }
        // add some redundancy to distinguish trigger between system menu and system move
        if (event->type() == QEvent::MouseMove) {
            QPointF currentPos = static_cast<QMouseEvent*>(event)->globalPos();
            QPointF delta = touchBeginPosition  - currentPos;
            if (delta.manhattanLength() < QGuiApplication::styleHints()->startDragDistance()) {
                return VtableHook::callOriginalFun(w, &QWindow::event, event);
            }
        }
    }

    bool is_mouse_move = event->type() == QEvent::MouseMove && static_cast<QMouseEvent*>(event)->buttons() == Qt::LeftButton;

    if (event->type() == QEvent::MouseButtonRelease) {
        self->m_windowMoving = false;
    }

    if (!HookCall(w, &QWindow::event, event))
        return false;

    // workaround for kwin: Qt receives no release event when kwin finishes MOVE operation,
    // which makes app hang in windowMoving state. when a press happens, there's no sense of
    // keeping the moving state, we can just reset ti back to normal.
    if (event->type() == QEvent::MouseButtonPress) {
        self->m_windowMoving = false;
    }

    if (is_mouse_move && !event->isAccepted()
            && w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos())) {
        if (!self->m_windowMoving && self->isEnableSystemMove()) {
            self->m_windowMoving = true;

            event->accept();
            startMoveWindow(w);
        }
    }

    return true;
}

bool DNoTitlebarWlWindowHelper::isEnableSystemMove(/*quint32 winId*/)
{
    return m_enableSystemMove;
}

void DNoTitlebarWlWindowHelper::startMoveWindow(QWindow *window)
{
    // QWaylandWindow::startSystemMove
    if (window && window->handle()) {
#if QTWAYLANDCLIENT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                static_cast<QtWaylandClient::QWaylandWindow *>(window->handle())->startSystemMove(QCursor::pos());
#else
                static_cast<QtWaylandClient::QWaylandWindow *>(window->handle())->startSystemMove();
#endif
    }
}

DPP_END_NAMESPACE
