// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "clientmanagement_interface.h"
#include "display.h"
#include "logging.h"
#include "surface_interface.h"
#include "utils.h"
#include "shmclientbuffer.h"

#include <qwayland-server-wayland.h>
#include "qwayland-server-com-deepin-client-management.h"

#define MAX_WINDOWS 100

namespace KWaylandServer
{

static const quint32 s_version = 1;

class ClientManagementInterfacePrivate: public QtWaylandServer::com_deepin_client_management
{
public:

    ClientManagementInterfacePrivate(ClientManagementInterface *q, Display *d);
    ClientManagementInterface *q;

    void updateWindowStates();
    void getWindowStates();
    void captureWindowImage(int windowId, wl_resource *buffer);
    void sendWindowStates(wl_resource *resource);
    void sendWindowCaption(int windowId, bool succeed, wl_resource *buffer);
    void sendSplitChange(const QString& uuid, int splitable);
    void splitWindow(QString uuid, int splitType);

    ClientManagementInterface::WindowState m_windowStates[MAX_WINDOWS];
    uint32_t m_windowCount;

protected:
    void com_deepin_client_management_get_window_states(Resource *resource) override;
    void com_deepin_client_management_capture_window_image(Resource *resource,
        int32_t window_id, struct ::wl_resource *buffer) override;
    void com_deepin_client_management_split_window(Resource *resource,
        const QString &uuid, int32_t splitType) override;
private:
    QString m_splitUuid;
    int     m_splitable = 0;
};

ClientManagementInterfacePrivate::ClientManagementInterfacePrivate(ClientManagementInterface *q, Display *d)
    : QtWaylandServer::com_deepin_client_management(*d, s_version)
    , q(q)
{
}

void ClientManagementInterfacePrivate::com_deepin_client_management_get_window_states(Resource *resource)
{
    Q_UNUSED(resource);

    getWindowStates();
}

void ClientManagementInterfacePrivate::com_deepin_client_management_capture_window_image(Resource *resource, int windowId, wl_resource *buffer)
{
    Q_UNUSED(resource);

    captureWindowImage(windowId, buffer);
}

void ClientManagementInterfacePrivate::com_deepin_client_management_split_window(Resource *resource, const QString &uuid, int32_t splitType)
{
    Q_UNUSED(resource);

    splitWindow(uuid, splitType);
}

void ClientManagementInterfacePrivate::getWindowStates()
{
    Q_EMIT q->windowStatesRequest();
}

void ClientManagementInterfacePrivate::captureWindowImage(int windowId, wl_resource *buffer)
{
    qWarning() << __func__ << ":" << __LINE__ << "ut-gfx-capture: windowId " << windowId;
    Q_EMIT q->captureWindowImageRequest(windowId, buffer);
}

void ClientManagementInterfacePrivate::splitWindow(QString uuid, int splitType)
{
    Q_EMIT q->splitWindowRequest(uuid, splitType);
}

void ClientManagementInterfacePrivate::sendWindowStates(wl_resource *resource)
{
    struct wl_array data;
    auto fillArray = [this](const ClientManagementInterface::WindowState *origin, wl_array *dest) {
        wl_array_init(dest);
        const size_t memLength = sizeof(struct ClientManagementInterface::WindowState) * m_windowCount;
        void *s = wl_array_add(dest, memLength);
        memcpy(s, origin, memLength);
    };
    fillArray(m_windowStates, &data);
    com_deepin_client_management_send_window_states(resource, m_windowCount, &data);
    wl_array_release(&data);
}

void ClientManagementInterfacePrivate::updateWindowStates()
{
    const auto clientResources = resourceMap();
    for (Resource *resource : clientResources) {
        sendWindowStates(resource->handle);
    }
}

void ClientManagementInterfacePrivate::sendWindowCaption(int windowId, bool succeed, wl_resource *buffer)
{
    const auto clientResources = resourceMap();
    for (Resource *resource : clientResources) {
        qWarning() << __func__ << ":" << __LINE__ << "ut-gfx-capture-sendWindowCaption: windowId " << windowId << " resource" << resource->handle;
        com_deepin_client_management_send_capture_callback(resource->handle, windowId, succeed, buffer);
    }
}

void ClientManagementInterfacePrivate::sendSplitChange(const QString& uuid, int splitable)
{
    if (splitable > 0) {
        m_splitUuid = uuid;
        m_splitable = splitable;
        const auto clientResources = resourceMap();
        for (Resource *resource : clientResources) {
            com_deepin_client_management_send_split_change(resource->handle, m_splitUuid.toLatin1().data(), m_splitable);
        }
    }
}

ClientManagementInterface::ClientManagementInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new ClientManagementInterfacePrivate(this, display))
{
    connect(this, &ClientManagementInterface::windowStatesChanged, this, [this] { this->d->updateWindowStates(); });
}

ClientManagementInterface::~ClientManagementInterface() = default;

ClientManagementInterface *ClientManagementInterface::get(wl_resource* native)
{
    if (auto interfacePrivate = resource_cast<ClientManagementInterfacePrivate *>(native)) {
        return interfacePrivate->q;
    }
    return nullptr;
}

void ClientManagementInterface::setWindowStates(QList<WindowState*> &windowStates)
{
    int i = 0;
    for (auto it = windowStates.begin();
        it != windowStates.end() && i < MAX_WINDOWS;
        ++it) {
        memcpy(&d->m_windowStates[i++], *it, sizeof(WindowState));
        d->m_windowCount = i;
    }
    Q_EMIT windowStatesChanged();
}

void ClientManagementInterface::sendWindowCaptionImage(int windowId, wl_resource *buffer, QImage image)
{
    bool succeed = false;
    wl_shm_buffer *shm_buffer = wl_shm_buffer_get(buffer);
    if (shm_buffer && !image.isNull()) {
        wl_shm_buffer_begin_access(shm_buffer);
        void *data = wl_shm_buffer_get_data(shm_buffer);
        if (data) {
            succeed = true;
            memcpy(data, image.bits(), image.sizeInBytes());
        }
        wl_shm_buffer_end_access(shm_buffer);
    }
    d->sendWindowCaption(windowId, succeed, buffer);
}

void ClientManagementInterface::sendWindowCaption(int windowId, wl_resource *buffer, SurfaceInterface* surface)
{
    if (!surface || !surface->buffer()) {
        d->sendWindowCaption(windowId, false, buffer);
        return;
    }

    auto shmClient = qobject_cast<ShmClientBuffer *>(surface->buffer());

    if(!shmClient) {
        return;
    }

    bool succeed = false;
    wl_shm_buffer *shm_buffer = wl_shm_buffer_get(buffer);
    if (shm_buffer) {
        QImage image = shmClient->data();
        void *data = wl_shm_buffer_get_data(shm_buffer);
        if (!image.isNull())
        {
            memcpy(data, image.bits(), image.sizeInBytes());
            succeed = true;
        }
    }
    d->sendWindowCaption(windowId, succeed, buffer);
}

void ClientManagementInterface::sendSplitChange(const QString& uuid, int splitable)
{
    d->sendSplitChange(uuid, splitable);
}

}