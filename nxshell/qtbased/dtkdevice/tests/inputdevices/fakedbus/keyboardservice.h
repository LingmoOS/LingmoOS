// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef KEYBOARDSERVICE_H
#define KEYBOARDSERVICE_H
#include "dtkdevice_global.h"
#include <QObject>
DDEVICE_BEGIN_NAMESPACE
class KeyboardService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.InputDevice.Keyboard")
    Q_PROPERTY(bool RepeatEnabled MEMBER m_repeatEnabled READ RepeatEnabled WRITE SetRepeatEnabled NOTIFY RepeatEnabledChanged)
    Q_PROPERTY(qint32 CursorBlink MEMBER m_cursorBlink READ CursorBlink WRITE SetCursorBlink NOTIFY CursorBlinkChanged)
    Q_PROPERTY(quint32 RepeatDelay MEMBER m_repeatDelay READ RepeatDelay WRITE SetRepeatDelay NOTIFY RepeatDelayChanged)
    Q_PROPERTY(
        quint32 RepeatInterval MEMBER m_repeatInterval READ RepeatInterval WRITE SetRepeatInterval NOTIFY RepeatIntervalChanged)

public:
    static inline const bool RepeatEnabledDefault = true;
    static inline const qint32 CursorBlinkDefault = 1200;
    static inline const quint32 RepeatDelayDefault = 100;
    static inline const quint32 RepeatIntervalDefault = 50;

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
    void Reset();

public:
    KeyboardService(const QString &service = QStringLiteral("org.deepin.dtk.InputDevices"),
                    const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/Keyboard"),
                    QObject *parent = nullptr);
    ~KeyboardService() override;

private:
    bool registerService(const QString &service, const QString &path);
    void unregisterService();

private:
    bool m_repeatEnabled;
    qint32 m_cursorBlink;
    quint32 m_repeatDelay;
    quint32 m_repeatInterval;
    QString m_service;
    QString m_path;
};
DDEVICE_END_NAMESPACE
#endif  // KEYBOARDSERVICE_H
