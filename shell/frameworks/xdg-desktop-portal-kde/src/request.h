/*
 * SPDX-FileCopyrightText: 2017 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_REQUEST_H
#define XDG_DESKTOP_PORTAL_KDE_REQUEST_H

#include <QDBusVirtualObject>
#include <QObject>

class QDBusObjectPath;

class Request : public QDBusVirtualObject
{
    Q_OBJECT
public:
    explicit Request(const QDBusObjectPath &handle, QObject *parent = nullptr, const QString &portalName = QString(), const QVariant &data = QVariant());

    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;
    QString introspect(const QString &path) const override;

    template<class T>
    static Request *makeClosableDialogRequest(const QDBusObjectPath &handle, T *dialogAndParent)
    {
        auto request = new Request(handle, dialogAndParent);
        connect(request, &Request::closeRequested, dialogAndParent, &T::reject);
        return request;
    }

Q_SIGNALS:
    void closeRequested();

private:
    const QVariant m_data;
    const QString m_portalName;
};

#endif // XDG_DESKTOP_PORTAL_KDE_REQUEST_H
