// SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LGPL-3.0-only

#ifndef DXCBXSETTINGS_H
#define DXCBXSETTINGS_H

#include "dplatformsettings.h"

#include <QByteArray>
#include <QByteArrayList>

#include <xcb/xcb.h>

QT_BEGIN_NAMESPACE
class QVariant;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DXcbXSettingsPrivate;

class DXcbXSettings : public DPlatformSettings
{
    Q_DECLARE_PRIVATE(DXcbXSettings)
public:
    DXcbXSettings(xcb_connection_t *connection, const QByteArray &property = QByteArray());
    DXcbXSettings(xcb_connection_t *connection, xcb_window_t setting_window, const QByteArray &property = QByteArray());
    DXcbXSettings(xcb_window_t setting_window, const QByteArray &property = QByteArray());
    ~DXcbXSettings();

    static xcb_window_t getOwner(xcb_connection_t *conn = nullptr, int screenNumber = 0);
    bool initialized() const override;
    bool isEmpty() const override;

    bool contains(const QByteArray &property) const override;
    QVariant setting(const QByteArray &property) const override;
    void setSetting(const QByteArray &property, const QVariant &value) override;
    QByteArrayList settingKeys() const override;

    typedef void (*PropertyChangeFunc)(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle);
    void registerCallback(PropertyChangeFunc func, void *handle);
    void registerCallbackForProperty(const QByteArray &property, PropertyChangeFunc func, void *handle);
    void removeCallbackForHandle(const QByteArray &property, void *handle);
    void removeCallbackForHandle(void *handle);
    typedef void (*SignalFunc)(xcb_connection_t *connection, const QByteArray &signal, qint32 data1, qint32 data2, void *handle);
    void registerSignalCallback(SignalFunc func, void *handle);
    void removeSignalCallback(void *handle);
    void emitSignal(const QByteArray &signal, qint32 data1, qint32 data2) override;

    static void emitSignal(xcb_connection_t *conn, xcb_window_t window, xcb_atom_t type, const QByteArray &signal, qint32 data1, qint32 data2);
    static bool handlePropertyNotifyEvent(const xcb_property_notify_event_t *event);
    static bool handleClientMessageEvent(const xcb_client_message_event_t *event);

    static void clearSettings(xcb_window_t setting_window);
private:
    DXcbXSettingsPrivate *d_ptr;
    friend class DXcbXSettingsPrivate;
};

DPP_END_NAMESPACE

#endif // DXCBXSETTINGS_H
