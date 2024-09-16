// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <functional>

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusError>

class Request : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Request")

public:
    Request(const QDBusObjectPath &handle, const QVariant &data, QObject *parent = nullptr);
    ~Request();

public slots:  // DBus methods
    Q_SCRIPTABLE void Close(const QDBusMessage &message);

signals:
    void closeRequested(const QVariant &data);

private:
    QDBusObjectPath m_handle;
    QVariant m_data;
};
