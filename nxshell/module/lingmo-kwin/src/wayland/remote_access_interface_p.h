/*
    SPDX-FileCopyrightText: 2016 Oleg Chernovskiy <kanedias@xaker.ru>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_SERVER_REMOTE_ACCESS_INTERFACE_P_H
#define KWAYLAND_SERVER_REMOTE_ACCESS_INTERFACE_P_H

struct wl_resource;

#include <QObject>

namespace KWaylandServer
{

class BufferHandle;
class RemoteAccessManagerInterface;
class RemoteBufferInterfacePrivate;
/**
 * @class RemoteBufferInterface
 * @brief Internally used class. Holds data of passed buffer and client resource. Also controls buffer lifecycle.
 * @see RemoteAccessManagerInterface
 */
class RemoteBufferInterface : public QObject
{
    Q_OBJECT
public:
    ~RemoteBufferInterface();

    /**
     * Sends GBM fd to the client.
     * Note that server still has to close mirror fd from its side.
     **/
    int sendGbmHandle();

private:
    explicit RemoteBufferInterface(const BufferHandle *buf, wl_resource *resource);
    friend class RemoteAccessManagerInterfacePrivate;

    QScopedPointer<RemoteBufferInterfacePrivate> d;
};

}

#endif // KWAYLAND_SERVER_REMOTE_ACCESS_P_H