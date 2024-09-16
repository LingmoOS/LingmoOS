/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2016 Oleg Chernovskiy <kanedias@xaker.ru>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef REMOTEACCESSMANAGER_H
#define REMOTEACCESSMANAGER_H

// DWayland
#include "wayland/remote_access_interface.h"

// Qt
#include <QObject>

struct gbm_bo;
struct gbm_surface;

namespace KWin
{

class DrmOutput;
class DrmBuffer;
class DrmGpuBuffer;
class Display;
class Output;
class DrmDumbBuffer;
class GbmBuffer;

using KWaylandServer::RemoteAccessManagerInterface;
using KWaylandServer::BufferHandle;

class RemoteAccessManager : public QObject
{
    Q_OBJECT
public:
    explicit RemoteAccessManager(QObject *parent = nullptr);
    ~RemoteAccessManager() override;

    void passBuffer(Output *output, DrmGpuBuffer *buffer);
    void passGbmBuffer(Output *output, GbmBuffer *buffer);
    void passDumBuffer(Output *output, DrmDumbBuffer *buffer);
    void passProhibitBuffer(Output *output, DrmGpuBuffer *buffer);
    void incrementRenderSequence();

Q_SIGNALS:
    void bufferNoLongerNeeded(qint32 gbm_handle);
    void screenRecordStatusChanged(bool isScreenRecording);
    void addedClient();

private:
    void releaseBuffer(const BufferHandle *buf);

    RemoteAccessManagerInterface *m_interface = nullptr;
    Output *m_removedOutput = nullptr;
};

} // KWin namespace

#endif // REMOTEACCESSMANAGER_H
