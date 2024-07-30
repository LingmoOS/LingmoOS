/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCUPSSERVER_H
#define KCUPSSERVER_H

#include <KCupsConnection.h>
#include <QString>
#include <kcupslib_export.h>

class KCUPSLIB_EXPORT KCupsServer
{
public:
    KCupsServer();

    bool allowRemoteAdmin() const;
    void setAllowRemoteAdmin(bool allow);

    bool allowUserCancelAnyJobs() const;
    void setAllowUserCancelAnyJobs(bool allow);

    bool showSharedPrinters() const;
    void setShowSharedPrinters(bool show);

    bool sharePrinters() const;
    void setSharePrinters(bool share);

    bool allowPrintingFromInternet() const;
    void setAllowPrintingFromInternet(bool allow);

    QVariantMap arguments() const;

protected:
    KCupsServer(const QVariantMap &arguments);

private:
    friend class KCupsRequest;

    QVariantMap m_arguments;
};

Q_DECLARE_METATYPE(KCupsServer)

#endif // KCUPSSERVER_H
