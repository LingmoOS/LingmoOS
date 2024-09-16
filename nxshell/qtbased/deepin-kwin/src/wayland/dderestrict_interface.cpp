/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "dderestrict_interface.h"
#include "display.h"
#include "utils.h"

#include <qwayland-server-wayland.h>
#include <qwayland-server-dde-restrict.h>

namespace KWaylandServer
{

class DDERestrictInterfacePrivate : QtWaylandServer::dde_restrict
{
public:
    DDERestrictInterfacePrivate(DDERestrictInterface *q, Display *d);
    // static DDERestrictInterface *get(wl_resource *native);

    void setProhibitScreencast(bool set);
    void setClientWhitelists(const QList<QByteArray> &whitelists);
    void setProtectedWindow(int32_t window);
    void removeProtectedWindow(int32_t window);

    bool m_prohibitScreencast = false;
    QList<QByteArray> m_clientWhitelists;
    QList<int32_t> m_protectedWindowIdLists;

private:
    // void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void dde_restrict_switch_screencast(Resource *resource, uint32_t switch_flag);
    void dde_restrict_client_whitelist(Resource *resource, const char *white_str);
    void dde_restrict_set_protected_window(Resource *resource, int32_t window);
    void dde_restrict_remove_protected_window(Resource *resource, int32_t window);
    // void unbind(wl_resource *resource);
    // Private *cast(wl_resource *r) {
    //     return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    // }

    DDERestrictInterface *q;
    static const struct dde_restrict_interface s_interface;
    static const quint32 s_version;
};

const quint32 DDERestrictInterfacePrivate::s_version = 1;

// #ifndef DOXYGEN_SHOULD_SKIP_THIS
// const struct dde_restrict_interface DDERestrictInterfacePrivate::s_interface = {
//     switchScreencastCallback,
//     clientWhitelistCallback,
//     setProtectedWindowCallback,
//     removeProtectedWindowCallback
// };
// #endif

DDERestrictInterfacePrivate::DDERestrictInterfacePrivate(DDERestrictInterface *q, Display *d)
    : QtWaylandServer::dde_restrict(*d, s_version)
    , q(q)
{
}

// DDERestrictInterface *DDERestrictInterfacePrivate::get(wl_resource *native)
// {
//     if (Private *p = cast(native)) {
//         return p->q;
//     }
//     return nullptr;
// }

// DDERestrictInterfacePrivate *DDERestrictInterface::d_func() const
// {
//     return reinterpret_cast<Private*>(d.data());
// }

void DDERestrictInterfacePrivate::dde_restrict_client_whitelist(Resource *resource, const char *white_str)
{
    Q_UNUSED(resource);

    if (white_str == nullptr)
        return;

    QByteArray byte(white_str);
    QList<QByteArray> white_lists;

    if (byte.contains(","))
        white_lists = byte.split(',');
    else {
        if (!byte.isEmpty()) white_lists.append(byte);
    }

    setClientWhitelists(white_lists);
}

void DDERestrictInterfacePrivate::dde_restrict_switch_screencast(Resource *resource, uint32_t switch_flag)
{
    Q_UNUSED(resource);
    setProhibitScreencast(switch_flag == dde_restrict_switch_flag::DDE_RESTRICT_SWITCH_FLAG_OFF);
}

void DDERestrictInterfacePrivate::dde_restrict_set_protected_window(Resource *resource, int32_t window)
{
    Q_UNUSED(resource);
    setProtectedWindow(window);
}

void DDERestrictInterfacePrivate::dde_restrict_remove_protected_window(Resource *resource, int32_t window)
{
    Q_UNUSED(resource);
    removeProtectedWindow(window);
}

void DDERestrictInterfacePrivate::setProhibitScreencast(bool set)
{
    if(m_prohibitScreencast == set)
        return;

    m_prohibitScreencast = set;
}

void DDERestrictInterfacePrivate::setClientWhitelists(const QList<QByteArray> &whitelists)
{
    if(m_clientWhitelists == whitelists)
        return;

    m_clientWhitelists = whitelists;
}

void DDERestrictInterfacePrivate::setProtectedWindow(int32_t window)
{
    if (m_protectedWindowIdLists.contains(window)) {
        return;
    }
    m_protectedWindowIdLists.append(window);
}

void DDERestrictInterfacePrivate::removeProtectedWindow(int32_t window)
{
    for (int i = 0; i < m_protectedWindowIdLists.length(); i++) {
        if (m_protectedWindowIdLists[i] == window) {
            m_protectedWindowIdLists.removeAt(i);
            break;
        }
    }
}

// void DDERestrictInterfacePrivate::bind(wl_client *client, uint32_t version, uint32_t id)
// {
//     auto c = display->getConnection(client);
//     wl_resource *resource = c->createResource(&dde_restrict_interface, qMin(version, s_version), id);
//     if (!resource) {
//         wl_client_post_no_memory(client);
//         return;
//     }
//     wl_resource_set_implementation(resource, &s_interface, this, unbind);
//     // TODO: should we track?
// }

// void DDERestrictInterfacePrivate::unbind(wl_resource *resource)
// {
//     Q_UNUSED(resource);
// }

DDERestrictInterface::DDERestrictInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new DDERestrictInterfacePrivate(this, display))
{
}

DDERestrictInterface::~DDERestrictInterface() = default;

// DDERestrictInterface *DDERestrictInterface::get(wl_resource* native)
// {
//     return Private::get(native);
// }

bool DDERestrictInterface::prohibitScreencast()
{
    return d->m_prohibitScreencast;
}

QList<QByteArray> DDERestrictInterface::clientWhitelists()
{
    return d->m_clientWhitelists;
}

QList<int32_t> DDERestrictInterface::protectedWindowIdLists()
{
    return d->m_protectedWindowIdLists;
}

void DDERestrictInterface::removeProtectedWindow(int32_t window)
{
    d->removeProtectedWindow(window);
}

}

