// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBKEYBOARD_H
#define QXCBKEYBOARD_H

#include "qxcbobject.h"

#include <xcb/xcb_keysyms.h>

#include <xkbcommon/xkbcommon.h>
#if QT_CONFIG(xkb)
#include <xkbcommon/xkbcommon-x11.h>
#endif

#include <QEvent>

QT_BEGIN_NAMESPACE

class QWindow;

class QXcbKeyboard : public QXcbObject
{
public:
    QXcbKeyboard(QXcbConnection *connection);

    ~QXcbKeyboard();

    void handleKeyPressEvent(const xcb_key_press_event_t *event);
    void handleKeyReleaseEvent(const xcb_key_release_event_t *event);

    Qt::KeyboardModifiers translateModifiers(int s) const;
    void updateKeymap(xcb_mapping_notify_event_t *event);
    void updateKeymap();
    QList<int> possibleKeys(const QKeyEvent *e) const;

    // when XKEYBOARD not present on the X server
    void updateXKBMods();
    xkb_mod_mask_t xkbModMask(quint16 state);
    void updateXKBStateFromCore(quint16 state);
#if QT_CONFIG(xinput2)
    void updateXKBStateFromXI(void *modInfo, void *groupInfo);
#endif
#if QT_CONFIG(xkb)
    // when XKEYBOARD is present on the X server
    int coreDeviceId() const { return core_device_id; }
    void updateXKBState(xcb_xkb_state_notify_event_t *state);
#endif
    void handleStateChanges(xkb_state_component changedComponents);

protected:
    void handleKeyEvent(xcb_window_t sourceWindow, QEvent::Type type, xcb_keycode_t code,
                        quint16 state, xcb_timestamp_t time, bool fromSendEvent);

    void resolveMaskConflicts();
    QString lookupString(struct xkb_state *state, xcb_keycode_t code) const;
    QString lookupStringNoKeysymTransformations(xkb_keysym_t keysym) const;
    int keysymToQtKey(xcb_keysym_t keysym, Qt::KeyboardModifiers modifiers,
                      struct xkb_state *state, xcb_keycode_t code) const;

    typedef QMap<xcb_keysym_t, int> KeysymModifierMap;
    struct xkb_keymap *keymapFromCore(const KeysymModifierMap &keysymMods);

    // when XKEYBOARD not present on the X server
    void updateModifiers(const KeysymModifierMap &keysymMods);
    KeysymModifierMap keysymsToModifiers();
    // when XKEYBOARD is present on the X server
    void updateVModMapping();
    void updateVModToRModMapping();

    xkb_keysym_t lookupLatinKeysym(xkb_keycode_t keycode) const;
    void checkForLatinLayout() const;

private:
    bool m_config = false;
    xcb_keycode_t m_autorepeat_code = 0;

    struct _mod_masks {
        uint alt;
        uint altgr;
        uint meta;
        uint super;
        uint hyper;
    };

    _mod_masks rmod_masks;

    // when XKEYBOARD not present on the X server
    xcb_key_symbols_t *m_key_symbols = nullptr;
    struct _xkb_mods {
        xkb_mod_index_t shift;
        xkb_mod_index_t lock;
        xkb_mod_index_t control;
        xkb_mod_index_t mod1;
        xkb_mod_index_t mod2;
        xkb_mod_index_t mod3;
        xkb_mod_index_t mod4;
        xkb_mod_index_t mod5;
    };
    _xkb_mods xkb_mods;
#if QT_CONFIG(xkb)
    // when XKEYBOARD is present on the X server
    _mod_masks vmod_masks;
    int core_device_id;
#endif

    struct XKBStateDeleter {
        void operator()(struct xkb_state *state) const { return xkb_state_unref(state); }
    };
    struct XKBKeymapDeleter {
        void operator()(struct xkb_keymap *keymap) const { return xkb_keymap_unref(keymap); }
    };
    struct XKBContextDeleter {
        void operator()(struct xkb_context *context) const { return xkb_context_unref(context); }
    };
    using ScopedXKBState = std::unique_ptr<struct xkb_state, XKBStateDeleter>;
    using ScopedXKBKeymap = std::unique_ptr<struct xkb_keymap, XKBKeymapDeleter>;
    using ScopedXKBContext = std::unique_ptr<struct xkb_context, XKBContextDeleter>;

    ScopedXKBState m_xkbState;
    ScopedXKBKeymap m_xkbKeymap;
    ScopedXKBContext m_xkbContext;
};

QT_END_NAMESPACE

#endif
