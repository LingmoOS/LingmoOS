// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "keyboardservice.h"
#include <QDebug>
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE
KeyboardService::KeyboardService(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , m_repeatEnabled(RepeatEnabledDefault)
    , m_cursorBlink(CursorBlinkDefault)
    , m_repeatDelay(RepeatDelayDefault)
    , m_repeatInterval(RepeatIntervalDefault)
    , m_service(service)
    , m_path(path)
{
    registerService(m_service, m_path);
}

KeyboardService::~KeyboardService()
{
    unregisterService();
}

bool KeyboardService::registerService(const QString &service, const QString &path)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(service)) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
        return false;
    }
    if (!connection.registerObject(path, this, QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
        return false;
    }
    return true;
}

void KeyboardService::unregisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

bool KeyboardService::RepeatEnabled() const
{
    return m_repeatEnabled;
}

qint32 KeyboardService::CursorBlink() const
{
    return m_cursorBlink;
}

quint32 KeyboardService::RepeatDelay() const
{
    return m_repeatDelay;
}

quint32 KeyboardService::RepeatInterval() const
{
    return m_repeatInterval;
}

void KeyboardService::SetRepeatEnabled(bool enabled)
{
    m_repeatEnabled = enabled;
    Q_EMIT this->RepeatEnabledChanged(enabled);
}

void KeyboardService::SetCursorBlink(qint32 cursorBlink)
{
    m_cursorBlink = cursorBlink;
    Q_EMIT this->CursorBlinkChanged(cursorBlink);
}

void KeyboardService::SetRepeatDelay(quint32 delay)
{
    m_repeatDelay = delay;
    Q_EMIT this->RepeatDelayChanged(delay);
}

void KeyboardService::SetRepeatInterval(quint32 interval)
{
    m_repeatInterval = interval;
    Q_EMIT this->RepeatIntervalChanged(interval);
}

void KeyboardService::Reset()
{
    m_repeatEnabled = RepeatEnabledDefault;
    m_repeatDelay = RepeatIntervalDefault;
    m_repeatInterval = RepeatIntervalDefault;
    m_cursorBlink = CursorBlinkDefault;
}
DDEVICE_END_NAMESPACE
