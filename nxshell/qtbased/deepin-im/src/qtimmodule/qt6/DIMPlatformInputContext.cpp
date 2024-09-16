// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "DIMPlatformInputContext.h"

#include "Keyboard.h"
#include "QWaylandTextInputInterface.h"
#include "TextInputV3.h"
#include "XdgSurface.h"
#include "XdgToplevel.h"
#include "wl/client/Compositor.h"
#include "wl/client/Connection.h"
#include "wl/client/ConnectionRaw.h"
#include "wl/client/Seat.h"
#include "wl/client/Shm.h"
#include "wl/client/XdgWmBase.h"
#include "wl/client/ZwpTextInputManagerV3.h"

#include <private/qxkbcommon_p.h>

#include <QLoggingCategory>
#include <QSocketNotifier>
#include <QThread>
#include <QtCore/QVarLengthArray>
#include <QtGui/QGuiApplication>
#include <QtGui/QTextCharFormat>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include <locale.h>

Q_LOGGING_CATEGORY(qLcQpaInputMethods, "qt.qpa.input.methods")

DIMPlatformInputContext::DIMPlatformInputContext()
    : m_XkbContext(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
{
    if (QGuiApplication::platformName().contains("wayland")) {
        QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
        struct wl_display *wl_dpy =
            (struct wl_display *)native->nativeResourceForWindow("display", NULL);

        wl_.reset(new wl::client::ConnectionRaw(wl_dpy));
    } else {
        QByteArray waylandDisplay = qgetenv("DIM_WAYLAND_DISPLAY");
        const auto displayName = waylandDisplay.toStdString();
        if (displayName.empty()) {
            qWarning() << "failed to get display env";
            return;
        }
        auto *wl = new wl::client::Connection(displayName);
        if (wl->display() == nullptr) {
            return;
        }
        wl_.reset(wl);
        auto *notifier = new QSocketNotifier(wl->getFd(), QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, [wl]() {
            wl->dispatch();
        });

        QAbstractEventDispatcher *dispatcher = QThread::currentThread()->eventDispatcher();
        QObject::connect(dispatcher, &QAbstractEventDispatcher::aboutToBlock, this, [this]() {
            wl_->flush();
        });

        auto shm = wl_->getGlobal<wl::client::Shm>();
        auto compositor = wl_->getGlobal<wl::client::Compositor>();
        auto *surface = compositor->create_surface();
        auto xdgWmBase = wl_->getGlobal<wl::client::XdgWmBase>();
        xdgSurface_ =
            std::make_shared<XdgSurface>(xdg_wm_base_get_xdg_surface(xdgWmBase->get(), surface),
                                         surface,
                                         shm);
        xdgToplevel_ = std::make_shared<XdgToplevel>(xdg_surface_get_toplevel(xdgSurface_->get()));
        wl_surface_commit(surface);
    }

    auto seat = wl_->getGlobal<wl::client::Seat>();
    keyboard_ = std::make_shared<Keyboard>(seat->get_keyboard());

    auto tiManager = wl_->getGlobal<wl::client::ZwpTextInputManagerV3>();
    if (tiManager) {
        textInput_ = std::make_shared<TextInputV3>(tiManager->get_text_input(seat));
    }
    wl_->flush();

    QXkbCommon::setXkbContext(this, m_XkbContext.get());
}

DIMPlatformInputContext::~DIMPlatformInputContext() { }

bool DIMPlatformInputContext::isValid() const
{
    // #if QT_WAYLAND_TEXT_INPUT_V4_WIP
    //     return mDisplay->textInputManagerv2() != nullptr || mDisplay->textInputManagerv1() !=
    //     nullptr || mDisplay->textInputManagerv4() != nullptr;
    // #else //  QT_WAYLAND_TEXT_INPUT_V4_WIP
    //     return mDisplay->textInputManagerv2() != nullptr || mDisplay->textInputManagerv1() !=
    //     nullptr;
    // #endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
    if (!textInput_) {
        return false;
    }
    return true;
}

void DIMPlatformInputContext::reset()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;
    if (m_composeState) {
        xkb_compose_state_reset(m_composeState);
    }

    QPlatformInputContext::reset();

    if (!textInput()) {
        return;
    }

    textInput()->reset();
}

void DIMPlatformInputContext::commit()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput()) {
        return;
    }

    textInput()->commit();
}

void DIMPlatformInputContext::update(Qt::InputMethodQueries queries)
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO << queries;

    if (!QGuiApplication::focusObject() || !textInput()) {
        return;
    }

    textInput()->updateState(queries,
                             QtWaylandClient::QWaylandTextInputInterface::update_state_change);
}

void DIMPlatformInputContext::invokeAction(QInputMethod::Action action, int cursorPostion)
{
    if (!textInput()) {
        return;
    }

    if (action == QInputMethod::Click) {
        // textInput()->setCursorInsidePreedit(cursorPostion);
    }
}

void DIMPlatformInputContext::showInputPanel()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput()) {
        return;
    }

    textInput()->showInputPanel();
}

void DIMPlatformInputContext::hideInputPanel()
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput()) {
        return;
    }

    textInput()->hideInputPanel();
}

bool DIMPlatformInputContext::isInputPanelVisible() const
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    return QPlatformInputContext::isInputPanelVisible();
}

QRectF DIMPlatformInputContext::keyboardRect() const
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    if (!textInput()) {
        return QPlatformInputContext::keyboardRect();
    }

    return textInput()->keyboardRect();
}

void DIMPlatformInputContext::setFocusObject(QObject *object)
{
    qCDebug(qLcQpaInputMethods) << Q_FUNC_INFO;

    m_focusObject = object;

    if (!textInput()) {
        return;
    }

    QWindow *window = QGuiApplication::focusWindow();

    if (window && window->handle() && inputMethodAccepted()) {
        textInput()->updateState(Qt::ImQueryAll,
                                 QtWaylandClient::QWaylandTextInputInterface::update_state_enter);
    }
}

const std::shared_ptr<QtWaylandClient::QWaylandTextInputInterface> &
DIMPlatformInputContext::textInput() const
{
    // return mDisplay->defaultInputDevice() ? mDisplay->defaultInputDevice()->textInput() :
    // nullptr;
    return textInput_;
}

void DIMPlatformInputContext::ensureInitialized()
{
    if (m_initialized) {
        return;
    }

    if (!m_XkbContext) {
        qCWarning(qLcQpaInputMethods)
            << "error: xkb context has not been set on" << metaObject()->className();
        return;
    }

    m_initialized = true;
    const char *const locale = setlocale(LC_CTYPE, nullptr);
    qCDebug(qLcQpaInputMethods) << "detected locale (LC_CTYPE):" << locale;

    m_composeTable =
        xkb_compose_table_new_from_locale(m_XkbContext.get(), locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (m_composeTable) {
        m_composeState = xkb_compose_state_new(m_composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
    }

    if (!m_composeTable) {
        qCWarning(qLcQpaInputMethods, "failed to create compose table");
        return;
    }
    if (!m_composeState) {
        qCWarning(qLcQpaInputMethods, "failed to create compose state");
        return;
    }
}

bool DIMPlatformInputContext::filterEvent(const QEvent *event)
{
    auto keyEvent = static_cast<const QKeyEvent *>(event);
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease) {
        return false;
    }

    if (!inputMethodAccepted()) {
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (!keyboard_->sameDevice(keyEvent->device())) {
        return true;
    }
#endif

    // lazy initialization - we don't want to do this on an app startup
    ensureInitialized();

    if (!m_composeTable || !m_composeState) {
        return false;
    }

    xkb_compose_state_feed(m_composeState, keyEvent->nativeVirtualKey());

    switch (xkb_compose_state_get_status(m_composeState)) {
    case XKB_COMPOSE_COMPOSING:
        return true;
    case XKB_COMPOSE_CANCELLED:
        reset();
        return false;
    case XKB_COMPOSE_COMPOSED: {
        const int size = xkb_compose_state_get_utf8(m_composeState, nullptr, 0);
        QVarLengthArray<char, 32> buffer(size + 1);
        xkb_compose_state_get_utf8(m_composeState, buffer.data(), buffer.size());
        QString composedText = QString::fromUtf8(buffer.constData());

        QInputMethodEvent event;
        event.setCommitString(composedText);

        if (!m_focusObject && qApp)
            m_focusObject = qApp->focusObject();

        if (m_focusObject)
            QCoreApplication::sendEvent(m_focusObject, &event);
        else
            qCWarning(qLcQpaInputMethods, "no focus object");

        reset();
        return true;
    }
    case XKB_COMPOSE_NOTHING:
        return false;
    default:
        Q_UNREACHABLE();
        return false;
    }
}
