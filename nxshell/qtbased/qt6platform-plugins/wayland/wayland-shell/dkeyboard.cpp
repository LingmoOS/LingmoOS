// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dkeyboard.h"

#include <private/qxkbcommon_p.h>
#include <private/qguiapplication_p.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformcursor.h>

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(dkeyboard, "dkeyboard.wayland.plugin" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(dkeyboard, "dkeyboard.wayland.plugin");
#endif

//#if QT_CONFIG(xkbcommon)
static QXkbCommon::ScopedXKBKeymap mXkbKeymap;
static QXkbCommon::ScopedXKBState mXkbState;
static uint32_t mNativeModifiers = 0;

namespace QtWaylandClient {

// from qtwayland...
static void handleKey(QWindow *window, ulong timestamp, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
               quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers,
               const QString &text, bool autorepeat = false, ushort count = 1)
{
    QPlatformInputContext *inputContext = QGuiApplicationPrivate::platformIntegration()->inputContext();
    bool filtered = false;

    bool usingInputContextFromCompositor = true;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QWaylandDisplay *display = static_cast<QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration())->display();
    usingInputContextFromCompositor = display && !display->usingInputContextFromCompositor();
#endif
    if (inputContext && usingInputContextFromCompositor) {
        QKeyEvent event(type, key, modifiers, nativeScanCode, nativeVirtualKey,
                        nativeModifiers, text, autorepeat, count);
        event.setTimestamp(timestamp);
        filtered = inputContext->filterEvent(&event);
    }

    if (!filtered) {
        if (type == QEvent::KeyPress && key == Qt::Key_Menu) {
            auto cursor = window->screen()->handle()->cursor();
            if (cursor) {
                const QPoint globalPos = cursor->pos();
                const QPoint pos = window->mapFromGlobal(globalPos);
                QWindowSystemInterface::handleContextMenuEvent(window, false, pos, globalPos, modifiers);
            }
        }

        QWindowSystemInterface::handleExtendedKeyEvent(window, timestamp, type, key, modifiers,
                nativeScanCode, nativeVirtualKey, nativeModifiers, text, autorepeat, count);
    }
}

static bool createDefaultKeymap()
{
    auto ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx)
        return false;

    struct xkb_rule_names names;
    names.rules = "evdev";
    names.model = "pc105";
    names.layout = "us";
    names.variant = "";
    names.options = "";

    mXkbKeymap.reset(xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS));
    if (mXkbKeymap)
        mXkbState.reset(xkb_state_new(mXkbKeymap.get()));

    if (!mXkbKeymap || !mXkbState) {
        qCWarning(dkeyboard) <<  "failed to create default keymap";
        return false;
    }

    return true;
}
// #endif //QT_CONFIG(xkbcommon)


DKeyboard::DKeyboard(QObject *parent)
    : QObject(parent)
{

}

DKeyboard::~DKeyboard()
{

}

void DKeyboard::handleKeyEvent(quint32 key, DDEKeyboard::KeyState state, quint32 time)
{
    QWaylandWindow *window = dynamic_cast<QWaylandWindow *>(parent());

    if (!window || !window->window() || window->isActive())
        return;

//#if QT_CONFIG(xkbcommon)
    if ((!mXkbKeymap || !mXkbState) && !createDefaultKeymap())
        return;

    QEvent::Type type = state == DDEKeyboard::KeyState::Pressed ? QEvent::KeyPress : QEvent::KeyRelease;
//    qCDebug(dkeyboard) << __func__ << " key " << key << " state " << int(state) << " time " << time;

    auto code = key + 8; // map to wl_keyboard::keymap_format::keymap_format_xkb_v1
    xkb_keysym_t sym = xkb_state_key_get_one_sym(mXkbState.get(), code);
    Qt::KeyboardModifiers modifiers = QXkbCommon::modifiers(mXkbState.get());
    QString text = QXkbCommon::lookupString(mXkbState.get(), code);

    int qtkey = QXkbCommon::keysymToQtKey(sym, modifiers, mXkbState.get(), code);
     qCDebug(dkeyboard) << __func__ << "type" << type << "qtkey" << qtkey <<
                           "mNativeModifiers" << mNativeModifiers <<
                           "modifiers" << modifiers << "text" << text;
    handleKey(window->window(), time, type, qtkey, modifiers, code, sym, mNativeModifiers, text);
//#endif
}

void DKeyboard::handleModifiersChanged(quint32 depressed, quint32 latched, quint32 locked, quint32 group)
{
    qCDebug(dkeyboard) << __func__ << " depressed " << depressed <<
                          " latched " << latched <<
                          " locked " << locked <<
                          " group " << group;
    if (mXkbState)
        xkb_state_update_mask(mXkbState.get(),
                              depressed, latched, locked,
                              0, 0, group);
    mNativeModifiers = depressed | latched | locked;
}

}
