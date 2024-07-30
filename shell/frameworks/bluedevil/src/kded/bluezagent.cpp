/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "bluezagent.h"
#include "bluedevil_kded.h"
#include "helpers/requestauthorization.h"
#include "helpers/requestconfirmation.h"
#include "helpers/requestpin.h"

#include <QDBusObjectPath>
#include <QStandardPaths>

#include <BluezQt/Device>

BluezAgent::BluezAgent(QObject *parent)
    : BluezQt::Agent(parent)
{
}

QDBusObjectPath BluezAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/modules/bluedevil/Agent"));
}

static void processAuthorizationRequest(BluezQt::DevicePtr device, const BluezQt::Request<> &request, RequestAuthorization::Result result)
{
    switch (result) {
    case RequestAuthorization::Accept:
        qCDebug(BLUEDEVIL_KDED_LOG) << "Accepting request";
        request.accept();
        break;

    case RequestAuthorization::AcceptAndTrust:
        qCDebug(BLUEDEVIL_KDED_LOG) << "Accepting request and trusting device";
        request.accept();
        device->setTrusted(true);
        break;

    default:
        qCDebug(BLUEDEVIL_KDED_LOG) << "Rejecting request";
        request.reject();
        break;
    }
}

void BluezAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    // TODO: Show user the Service UUID
    qCDebug(BLUEDEVIL_KDED_LOG) << "AGENT-AuthorizeService" << device->name() << "Service:" << uuid;

    RequestAuthorization *helper = new RequestAuthorization(device, this);
    connect(helper, &RequestAuthorization::done, this, [device, request](RequestAuthorization::Result result) {
        processAuthorizationRequest(device, request, result);
    });
}

void BluezAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "AGENT-RequestPinCode " << device->name();

    RequestPin *helper = new RequestPin(device, false, this);
    connect(helper, &RequestPin::done, this, [request](const QString &result) {
        if (!result.isEmpty()) {
            qCDebug(BLUEDEVIL_KDED_LOG) << "Introducing PIN...";
            request.accept(result);
            return;
        }

        qCDebug(BLUEDEVIL_KDED_LOG) << "No PIN introduced";
        request.reject();
    });
}

void BluezAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "AGENT-RequestPasskey " << device->name();

    RequestPin *helper = new RequestPin(device, true, this);
    connect(helper, &RequestPin::done, this, [request](const QString &result) {
        bool ok;
        quint32 passkey = result.toInt(&ok);
        if (ok) {
            qCDebug(BLUEDEVIL_KDED_LOG) << "Introducing PassKey...";
            request.accept(passkey);
            return;
        }

        qCDebug(BLUEDEVIL_KDED_LOG) << "No PassKey introduced";
        request.reject();
    });
}

void BluezAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "AGENT-RequestConfirmation " << device->name() << passkey;

    RequestConfirmation *helper = new RequestConfirmation(device, passkey, this);
    connect(helper, &RequestConfirmation::done, this, [request](RequestConfirmation::Result result) {
        if (result == RequestConfirmation::Accept) {
            qCDebug(BLUEDEVIL_KDED_LOG) << "Accepting request";
            request.accept();
            return;
        }

        qCDebug(BLUEDEVIL_KDED_LOG) << "Rejecting request";
        request.reject();
    });
}

void BluezAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "AGENT-RequestAuthorization";

    RequestAuthorization *helper = new RequestAuthorization(device, this);
    connect(helper, &RequestAuthorization::done, this, [device, request](RequestAuthorization::Result result) {
        processAuthorizationRequest(device, request, result);
    });
}

void BluezAgent::release()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "AGENT-Release";

    Q_EMIT agentReleased();
}

void BluezAgent::cancel()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "AGENT-Cancel";

    Q_EMIT agentCanceled();
}

#include "moc_bluezagent.cpp"
