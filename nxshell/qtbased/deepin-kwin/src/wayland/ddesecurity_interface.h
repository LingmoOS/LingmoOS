/********************************************************************
Copyright 2022  luochaojiang <luochaojiang@uniontech.com>

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
#ifndef WAYLAND_SERVER_DDE_SECURITY_INTERFACE_H
#define WAYLAND_SERVER_DDE_SECURITY_INTERFACE_H

#include <QObject>

#include "deepin-kwin_export.h"

#include "qwayland-server-dde-security.h"

#include <memory>

struct wl_resource;

namespace KWaylandServer
{

class Display;
class DDESecuritySessionInterface;
class DDESecurityInterfacePrivate;
class DDESecuritySessionInterfacePrivate;

/** @class DDESecurityInterface
 *
 *
 * @see DDESecurityInterface
 * @since 5.5
 */
class KWIN_EXPORT DDESecurityInterface : public QObject
{
    Q_OBJECT
public:
    DDESecurityInterface(Display *display, QObject *parent = nullptr);
    virtual ~DDESecurityInterface();

    static const quint32 s_VerifyTimeout = 30000;

    enum SecurityType {
	    SEC_CLIPBOARD_COPY = QtWaylandServer::dde_security::types::types_sec_clipboard_copy,
    };

    enum Permission {
        PERMISSION_ALLOW = QtWaylandServer::dde_security::permissions::permissions_permission_allow,
        PERMISSION_DENY = QtWaylandServer::dde_security::permissions::permissions_permission_deny,
    };

    int doVerifySecurity(uint32_t types, uint32_t client, uint32_t target);

Q_SIGNALS:
    void securitySessionCreated(DDESecuritySessionInterface* session);
    void copySecurityVerified(uint32_t serial, uint32_t permission);

private:
    std::unique_ptr<DDESecurityInterfacePrivate> d;
    friend class DDESecurityInterfacePrivate;
};

/**
 * @brief Resource for the dde_security interface.
 *
 * DDESecuritySessionInterface gets created by DDESecurityInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT DDESecuritySessionInterface : public QObject
{
    Q_OBJECT
public:
    DDESecuritySessionInterface(DDESecurityInterface *ddeSecurity, uint32_t types, wl_resource *resource);
    virtual ~DDESecuritySessionInterface();

    DDESecurityInterface *ddeSecurity() const;

    static DDESecuritySessionInterface *get(wl_resource *native);

    bool sendVerifySecurity(uint32_t types, uint32_t client, uint32_t target, uint32_t serial);

    uint32_t processId()
    {
        return m_pid;
    }

    uint32_t m_pid = 0;

Q_SIGNALS:
    void sessionVerified(uint32_t types, uint32_t permission, uint32_t serial);

private:
    std::unique_ptr<DDESecuritySessionInterfacePrivate> d;
    friend class DDESecuritySessionInterfacePrivate;
};

}

#endif
