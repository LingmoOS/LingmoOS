/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KGLOBALACCEL_X11_H
#define _KGLOBALACCEL_X11_H

#include "../../kglobalaccel_interface.h"

#include <QAbstractNativeEventFilter>
#include <QObject>

struct xcb_key_press_event_t;
typedef xcb_key_press_event_t xcb_key_release_event_t;
typedef struct _XCBKeySymbols xcb_key_symbols_t;
class QTimer;

/**
 * @internal
 *
 * The KGlobalAccel private class handles grabbing of global keys,
 * and notification of when these keys are pressed.
 */
class KGlobalAccelImpl : public KGlobalAccelInterface, public QAbstractNativeEventFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kglobalaccel5.KGlobalAccelInterface" FILE "xcb.json")
    Q_INTERFACES(KGlobalAccelInterface)

public:
    KGlobalAccelImpl(QObject *parent = nullptr);
    ~KGlobalAccelImpl() override;

public:
    /**
     * This function registers or unregisters a certain key for global capture,
     * depending on \b grab.
     *
     * Before destruction, every grabbed key will be released, so this
     * object does not need to do any tracking.
     *
     * \param key the Qt keycode to grab or release.
     * \param grab true to grab they key, false to release the key.
     *
     * \return true if successful, otherwise false.
     */
    bool grabKey(int key, bool grab) override;

    /// Enable/disable all shortcuts. There will not be any grabbed shortcuts at this point.
    void setEnabled(bool) override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

private:
    void scheduleX11MappingNotify();
    void x11MappingNotify();
    /**
     * Filters X11 events ev for key bindings in the accelerator dictionary.
     * If a match is found the activated activated is emitted and the function
     * returns true. Return false if the event is not processed.
     *
     * This is public for compatibility only. You do not need to call it.
     */
    bool x11KeyPress(xcb_key_press_event_t *event);
    bool x11KeyRelease(xcb_key_press_event_t *event);
    bool x11ButtonPress(xcb_key_press_event_t *event);

    xcb_key_symbols_t *m_keySymbols;
    uint8_t m_xkb_first_event;
    void *m_display;
    unsigned int m_xrecordCookieSequence;
    QTimer *m_remapTimer;
};

#endif // _KGLOBALACCEL_X11_H
