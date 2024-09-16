// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef KEYBOARDINTERFACE_H
#define KEYBOARDINTERFACE_H
#include "dtkdevice_global.h"
#include <DDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QObject>
DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;
class LIBDTKDEVICESHARED_EXPORT KeyboardInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool RepeatEnabled READ RepeatEnabled WRITE SetRepeatEnabled NOTIFY RepeatEnabledChanged)
    Q_PROPERTY(qint32 CursorBlink READ CursorBlink WRITE SetCursorBlink NOTIFY CursorBlinkChanged)
    Q_PROPERTY(quint32 RepeatDelay READ RepeatDelay WRITE SetRepeatDelay NOTIFY RepeatDelayChanged)
    Q_PROPERTY(quint32 RepeatInterval READ RepeatInterval WRITE SetRepeatInterval NOTIFY RepeatIntervalChanged)

public:
    static inline const char *staticInterfaceName() { return "com.deepin.daemon.InputDevice.Keyboard"; }
    KeyboardInterface(const QString &service = QStringLiteral("com.deepin.daemon.InputDevices"),
                      const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/Keyboard"),
                      QDBusConnection connection = QDBusConnection::sessionBus(),
                      QObject *parent = nullptr);

    bool RepeatEnabled() const;
    qint32 CursorBlink() const;
    quint32 RepeatDelay() const;
    quint32 RepeatInterval() const;

    void SetRepeatEnabled(bool enabled);
    void SetCursorBlink(qint32 cursorBlink);
    void SetRepeatDelay(quint32 delay);
    void SetRepeatInterval(quint32 interval);

Q_SIGNALS:
    void RepeatEnabledChanged(bool enabled);
    void CursorBlinkChanged(qint32 cursorBlink);
    void RepeatDelayChanged(quint32 delay);
    void RepeatIntervalChanged(quint32 interval);

public Q_SLOTS:
    QDBusPendingReply<> Reset();

private:
    DDBusInterface *m_interface;
};
DDEVICE_END_NAMESPACE
#endif  // KEYBOARDINTERFACE_H
