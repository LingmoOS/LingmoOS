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
#include "ddesecurity_interface.h"
#include "display.h"
#include "utils/common.h"

#include <QDebug>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QVector>

#include "ddesecurity_interface_p.h"

namespace KWaylandServer
{

static const int s_version = 1;

DDESecurityInterface::DDESecurityInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new DDESecurityInterfacePrivate(this, display))
{
}

DDESecurityInterface::~DDESecurityInterface() = default;

int DDESecurityInterface::doVerifySecurity(uint32_t types, uint32_t client, uint32_t target)
{
    if (client == target) {
        qCWarning(KWIN_CORE) << "client and target is the same";
        return -1;
    }

    if (d->ddeSecuritySessions.empty()) {
        qCWarning(KWIN_CORE) << "ddeSecuritySessions is empty";
        return -1;
    }

    QVector<SessionState *> verifyingState;
    const uint32_t serial = d->display->nextSerial();

    for (auto it = d->ddeSecuritySessions.constBegin(); it != d->ddeSecuritySessions.constEnd(); it++) {
        DDESecuritySessionInterface *session = *it;
        if (session->sendVerifySecurity(types, client, target, serial)) {
            SessionState *state = new SessionState(session);
            state->timer = new QTimer(state);
            state->timer->setSingleShot(true);
            state->timer->setInterval(s_VerifyTimeout);
            QObject::connect(state->timer, &QTimer::timeout, this, [this, session, types, serial] {
                qWarning() << "do security timeout in " << s_VerifyTimeout << " seconds types " << types << " serial " << serial;
                d->handleSecurityVerified(session, types, Permission::PERMISSION_ALLOW, serial);
            });
            state->timer->start();
            state->pedingSession = session;
            verifyingState << state;
        }
    }

    if (verifyingState.empty()) {
        return -1;
    }

    d->pendingSessions.insert(serial, verifyingState);

    qCWarning(KWIN_CORE) << "do security verify from " << client << " to "
                         << target << " types " << types << " serial " << serial << " sessions " << verifyingState;

    return serial;
}

DDESecurityInterfacePrivate::DDESecurityInterfacePrivate(DDESecurityInterface *q, Display *d)
    : QtWaylandServer::dde_security(*d, s_version)
    , q(q)
    , display(d)
{
}

DDESecurityInterfacePrivate *DDESecurityInterfacePrivate::get(DDESecurityInterface *ddeSecurity)
{
    return ddeSecurity->d.get();
}

void DDESecurityInterfacePrivate::dde_security_get_session(Resource *resource, uint32_t id, uint32_t types)
{
    wl_resource *sessionResource = wl_resource_create(resource->client(), &security_session_interface, resource->version(), id);

    DDESecuritySessionInterface *securitySession = new DDESecuritySessionInterface(q, types, sessionResource);
    ddeSecuritySessions << securitySession;
    QObject::connect(securitySession, &DDESecuritySessionInterface::destroyed, q,
                     [this, securitySession] {
                         ddeSecuritySessions.removeAll(securitySession);
                     });

    QObject::connect(securitySession, &DDESecuritySessionInterface::sessionVerified, q,
                     [this, securitySession](uint32_t types, uint32_t permission, uint32_t serial) {
                         handleSecurityVerified(securitySession, types, permission, serial);
                     });

    securitySession->m_pid = display->getConnection(resource->client())->processId();
    Q_EMIT q->securitySessionCreated(securitySession);
}

void DDESecurityInterfacePrivate::dde_security_get_ace_clients(Resource *resource, uint32_t pid)
{
    Q_UNUSED(pid)

    int count = 0;
    QByteArray pids;
    pids.resize(ddeSecuritySessions.size());
    for (auto it = ddeSecuritySessions.constBegin(); it != ddeSecuritySessions.constEnd(); it++) {
        pids[count++] = (*it)->processId();
    }

    if (resource) {
        send_ace_clients(resource->handle, count, pids);
    }
}

void DDESecurityInterfacePrivate::handleSecurityVerified(DDESecuritySessionInterface *session, uint32_t types, uint32_t permission, uint32_t serial)
{
    qCDebug(KWIN_CORE) << "handle security verified types " << types << " permission " << permission << " serial " << serial;
    QVector<SessionState *> verifyingState = pendingSessions.value(serial);
    if (verifyingState.isEmpty()) {
        qCWarning(KWIN_CORE) << "handle security verified find no sessions";
        return;
    }
    SessionState *pendingState = nullptr;
    for (auto state : qAsConst(verifyingState)) {
        if (state->pedingSession == session) {
            pendingState = state;
            break;
        }
    }
    if (!pendingState) {
        qCWarning(KWIN_CORE) << "handle security verified not contains session";
        return;
    }

    if (pendingState->timer) {
        pendingState->timer->stop();
    }

    if (permission == DDESecurityInterface::Permission::PERMISSION_ALLOW) {
        verifyingState.removeAll(pendingState);
    } else {
        verifyingState.clear();
    }
    pendingSessions[serial] = verifyingState;

    if (verifyingState.isEmpty()) {
        if (types & DDE_SECURITY_TYPES_SEC_CLIPBOARD_COPY) {
            Q_EMIT q->copySecurityVerified(serial, permission);
        }
    }
}

// DDESecuritySessionInterface
DDESecuritySessionInterfacePrivate::DDESecuritySessionInterfacePrivate(DDESecuritySessionInterface *q, DDESecurityInterface *security, uint32_t types, wl_resource *resource)
    : QtWaylandServer::security_session(resource)
    , ddeSecurity(security)
    , support_types(types)
    , q(q)
{
}

void DDESecuritySessionInterfacePrivate::security_session_destroy(Resource *resource)
{
    Q_UNUSED(resource)
}

void DDESecuritySessionInterfacePrivate::security_session_report_security(Resource *resource, uint32_t types, uint32_t permission, uint32_t serial)
{
    Q_UNUSED(resource)
    Q_EMIT q->sessionVerified(types, permission, serial);
}

bool DDESecuritySessionInterfacePrivate::sendVerifySecurity(uint32_t types, uint32_t client, uint32_t target, uint32_t serial)
{
    if (!resource()) {
        return false;
    }
    if (types & support_types != types) {
        return false;
        qDebug() << "The session cat not support this types " << ((types ^ support_types) & types);
    }
    request_types = types;
    request_serial = serial;
    send_verify_security(types, client, target, serial);
    return true;
}

DDESecuritySessionInterface::DDESecuritySessionInterface(DDESecurityInterface *security, uint32_t types, wl_resource *resource)
    : QObject(security)
    , d(new DDESecuritySessionInterfacePrivate(this, security, types, resource))
{
}

DDESecuritySessionInterface::~DDESecuritySessionInterface() = default;

DDESecurityInterface *DDESecuritySessionInterface::ddeSecurity() const
{
    return d->ddeSecurity;
}

bool DDESecuritySessionInterface::sendVerifySecurity(uint32_t types, uint32_t client, uint32_t target, uint32_t serial)
{
    return d->sendVerifySecurity(types, client, target, serial);
}

}

#include "ddesecurity_interface.moc"