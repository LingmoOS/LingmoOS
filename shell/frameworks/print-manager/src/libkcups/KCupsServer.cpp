/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsServer.h"

#include <cups/adminutil.h>
#include <cups/cups.h>

#include <config.h>

KCupsServer::KCupsServer()
{
}

KCupsServer::KCupsServer(const QVariantMap &arguments)
    : m_arguments(arguments)
{
}

bool KCupsServer::allowRemoteAdmin() const
{
    return m_arguments[QLatin1String(CUPS_SERVER_REMOTE_ADMIN)].toBool();
}

void KCupsServer::setAllowRemoteAdmin(bool allow)
{
    m_arguments[QLatin1String(CUPS_SERVER_REMOTE_ADMIN)] = allow ? QLatin1String("1") : QLatin1String("0");
}

bool KCupsServer::allowUserCancelAnyJobs() const
{
    return m_arguments[QLatin1String(CUPS_SERVER_USER_CANCEL_ANY)].toBool();
}

void KCupsServer::setAllowUserCancelAnyJobs(bool allow)
{
    m_arguments[QLatin1String(CUPS_SERVER_USER_CANCEL_ANY)] = allow ? QLatin1String("1") : QLatin1String("0");
}

bool KCupsServer::showSharedPrinters() const
{
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    return m_arguments.value(CUPS_SERVER_REMOTE_PRINTERS).toBool();
#else
    return false;
#endif
}

void KCupsServer::setShowSharedPrinters(bool show)
{
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    m_arguments[CUPS_SERVER_REMOTE_PRINTERS] = show ? QLatin1String("1") : QLatin1String("0");
#else
    Q_UNUSED(show)
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
}

bool KCupsServer::sharePrinters() const
{
    return m_arguments[QLatin1String(CUPS_SERVER_SHARE_PRINTERS)].toBool();
}

void KCupsServer::setSharePrinters(bool share)
{
    m_arguments[QLatin1String(CUPS_SERVER_SHARE_PRINTERS)] = share ? QLatin1String("1") : QLatin1String("0");
}

bool KCupsServer::allowPrintingFromInternet() const
{
    return m_arguments[QLatin1String(CUPS_SERVER_REMOTE_ANY)].toBool();
}

void KCupsServer::setAllowPrintingFromInternet(bool allow)
{
    m_arguments[QLatin1String(CUPS_SERVER_REMOTE_ANY)] = allow ? QLatin1String("1") : QLatin1String("0");
}

QVariantMap KCupsServer::arguments() const
{
    return m_arguments;
}
