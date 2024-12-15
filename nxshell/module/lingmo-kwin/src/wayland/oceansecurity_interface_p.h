/*
    SPDX-FileCopyrightText: 2023 luochaojiang <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include "oceansecurity_interface.h"

#include "qwayland-server-ocean-security.h"

namespace KWaylandServer
{
class SessionState : public QObject
{
    Q_OBJECT
public:
    SessionState(QObject *parent)
        : QObject(parent)
    {
    }
    uint32_t serial;
    QTimer *timer = nullptr;
    OCEANSecuritySessionInterface *pedingSession;
};

class OCEANSecurityInterfacePrivate : public QtWaylandServer::ocean_security
{
public:
    OCEANSecurityInterfacePrivate(OCEANSecurityInterface *q, Display *d);
    static OCEANSecurityInterfacePrivate *get(OCEANSecurityInterface *oceanSecurity);

    QVector<OCEANSecuritySessionInterface *> oceanSecuritySessions;
    QMap<uint32_t, QVector<SessionState *>> pendingSessions;

    void handleSecurityVerified(OCEANSecuritySessionInterface *session, uint32_t types, uint32_t permission, uint32_t serial);

    OCEANSecurityInterface *q;
    Display *display;

private:
    // interfaces
    void ocean_security_get_session(Resource *resource, uint32_t id, uint32_t types) override;
    void ocean_security_get_ace_clients(Resource *resource, uint32_t pid) override;
};

class OCEANSecuritySessionInterfacePrivate : public QtWaylandServer::security_session
{
public:
    OCEANSecuritySessionInterfacePrivate(OCEANSecuritySessionInterface *q, OCEANSecurityInterface *oceanSecurity, uint32_t types, wl_resource *parentResource);
    static OCEANSecuritySessionInterfacePrivate *get(OCEANSecuritySessionInterface *session);

    bool sendVerifySecurity(uint32_t types, uint32_t client, uint32_t target, uint32_t serial);

    OCEANSecurityInterface *oceanSecurity;

    uint32_t request_types;
    uint32_t request_serial;

    QMetaObject::Connection destroyConnection;

private:
    void reportSecurity(wl_client *client, wl_resource *resource, uint32_t types, uint32_t permission, uint32_t serial);

    void security_session_destroy(Resource *resource);
    void security_session_report_security(Resource *resource, uint32_t types, uint32_t permission, uint32_t serial);

    static const struct security_session_interface s_interface;
    uint32_t support_types;
    OCEANSecuritySessionInterface *q;
};

}