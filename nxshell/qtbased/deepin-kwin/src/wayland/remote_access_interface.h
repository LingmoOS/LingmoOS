/*
    SPDX-FileCopyrightText: 2016 Oleg Chernovskiy <kanedias@xaker.ru>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_SERVER_REMOTE_ACCESS_INTERFACE_H
#define KWAYLAND_SERVER_REMOTE_ACCESS_INTERFACE_H

#include "deepin-kwin_export.h"

#include <QObject>

struct wl_resource;

namespace KWaylandServer
{

class RemoteAccessManagerInterfacePrivate;
class Display;
class OutputInterface;
class BufferHandlePrivate;

/**
 * The structure server should fill to use this interface.
 * Lifecycle:
 *  1. BufferHandle is filled and passed to RemoteAccessManager
 *     (stored in manager's sent list)
 *  2. Clients confirm that they wants this buffer, the RemoteBuffer
 *     interfaces are then created and wrapped around BufferHandle.
 *  3. Once all clients are done with buffer (or disconnected),
 *     RemoteBuffer notifies manager and release signal is emitted.
 *
 *     It's the responsibility of your process to delete this BufferHandle
 *     and release its' fd afterwards.
 **/

class KWIN_EXPORT BufferHandle : public QObject
{
    Q_OBJECT
public:
    explicit BufferHandle();
    virtual ~BufferHandle();
    void setFd(qint32 fd);
    void setSize(quint32 width, quint32 height);
    void setStride(quint32 stride);
    void setFormat(quint32 format);
    void setFrame(qint32 frame);

    qint32 fd() const;
    quint32 height() const;
    quint32 width() const;
    quint32 stride() const;
    quint32 format() const;
    qint32 frame() const;
private:
    friend class RemoteAccessManagerInterface;
    friend class RemoteBufferInterface;
    QScopedPointer<BufferHandlePrivate> d;
};

class KWIN_EXPORT RemoteAccessManagerInterface : public QObject
{
    Q_OBJECT
public:
    explicit RemoteAccessManagerInterface(Display *display);
    ~RemoteAccessManagerInterface();

    /**
     * Store buffer in sent list and notify client that we have a buffer for it
     **/
    void sendBufferReady(const OutputInterface *output, BufferHandle *buf);

    /**
     * Increase the rendering sequence
     **/
    void incrementRenderSequence();
    /**
     * Check whether interface has been bound
     **/
    bool isBound() const;
Q_SIGNALS:
    /**
     * Previously sent buffer has been released by client
     */
    void bufferReleased(const BufferHandle *buf);
    void screenRecordStatusChanged(bool isScreenRecording);
    void startRecord(int count);
    void addedClient();

private:
    friend class Display;
    QScopedPointer<RemoteAccessManagerInterfacePrivate> d;
};

}

#endif