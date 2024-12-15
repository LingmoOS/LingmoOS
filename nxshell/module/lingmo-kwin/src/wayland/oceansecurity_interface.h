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
#ifndef WAYLAND_SERVER_OCEAN_SECURITY_INTERFACE_H
#define WAYLAND_SERVER_OCEAN_SECURITY_INTERFACE_H

#include <QObject>

#include "lingmo-kwin_export.h"

#include "qwayland-server-ocean-security.h"

#include <memory>

struct wl_resource;

namespace KWaylandServer
{

class Display;
class OCEANSecuritySessionInterface;
class OCEANSecurityInterfacePrivate;
class OCEANSecuritySessionInterfacePrivate;

/** @class OCEANSecurityInterface
 *
 *
 * @see OCEANSecurityInterface
 * @since 5.5
 */
class KWIN_EXPORT OCEANSecurityInterface : public QObject
{
    Q_OBJECT
public:
    OCEANSecurityInterface(Display *display, QObject *parent = nullptr);
    virtual ~OCEANSecurityInterface();

    static const quint32 s_VerifyTimeout = 30000;

    enum SecurityType {
	    SEC_CLIPBOARD_COPY = QtWaylandServer::ocean_security::types::types_sec_clipboard_copy,
    };

    enum Permission {
        PERMISSION_ALLOW = QtWaylandServer::ocean_security::permissions::permissions_permission_allow,
        PERMISSION_DENY = QtWaylandServer::ocean_security::permissions::permissions_permission_deny,
    };

    int doVerifySecurity(uint32_t types, uint32_t client, uint32_t target);

Q_SIGNALS:
    void securitySessionCreated(OCEANSecuritySessionInterface* session);
    void copySecurityVerified(uint32_t serial, uint32_t permission);

private:
    std::unique_ptr<OCEANSecurityInterfacePrivate> d;
    friend class OCEANSecurityInterfacePrivate;
};

/**
 * @brief Resource for the ocean_security interface.
 *
 * OCEANSecuritySessionInterface gets created by OCEANSecurityInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT OCEANSecuritySessionInterface : public QObject
{
    Q_OBJECT
public:
    OCEANSecuritySessionInterface(OCEANSecurityInterface *oceanSecurity, uint32_t types, wl_resource *resource);
    virtual ~OCEANSecuritySessionInterface();

    OCEANSecurityInterface *oceanSecurity() const;

    static OCEANSecuritySessionInterface *get(wl_resource *native);

    bool sendVerifySecurity(uint32_t types, uint32_t client, uint32_t target, uint32_t serial);

    uint32_t processId()
    {
        return m_pid;
    }

    uint32_t m_pid = 0;

Q_SIGNALS:
    void sessionVerified(uint32_t types, uint32_t permission, uint32_t serial);

private:
    std::unique_ptr<OCEANSecuritySessionInterfacePrivate> d;
    friend class OCEANSecuritySessionInterfacePrivate;
};

}

#endif
