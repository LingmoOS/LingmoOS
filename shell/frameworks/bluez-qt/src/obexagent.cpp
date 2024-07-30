/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexagent.h"
#include "debug.h"

namespace BluezQt
{
ObexAgent::ObexAgent(QObject *parent)
    : QObject(parent)
{
}

void ObexAgent::authorizePush(ObexTransferPtr transfer, ObexSessionPtr session, const Request<QString> &request)
{
    Q_UNUSED(transfer)
    Q_UNUSED(session)
    Q_UNUSED(request)
}

void ObexAgent::cancel()
{
}

void ObexAgent::release()
{
}

} // namespace BluezQt

#include "moc_obexagent.cpp"
