/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "backend_p.h"

#include "backends/cryfs/cryfsbackend.h"
#include "backends/encfs/encfsbackend.h"
#include "backends/gocryptfs/gocryptfsbackend.h"

#include <KLocalizedString>

namespace PlasmaVault
{
Backend::Backend()
{
}

Backend::~Backend()
{
}

QStringList Backend::availableBackends()
{
    return {QStringLiteral("encfs"), QStringLiteral("cryfs"), QStringLiteral("gocryptfs")};
}

Backend::Ptr Backend::instance(const QString &backend)
{
    return backend == QLatin1String("encfs")    ? PlasmaVault::EncFsBackend::instance()
        : backend == QLatin1String("cryfs")     ? PlasmaVault::CryFsBackend::instance()
        : backend == QLatin1String("gocryptfs") ? PlasmaVault::GocryptfsBackend::instance()
                                                :
                                                /* unknown backend */ nullptr;
}

QString Backend::formatMessageLine(const QString &command, const QPair<bool, QString> &result) const
{
    const auto valid = result.first;
    const auto message = result.second;

    QString htmlMessage = (valid ? QString() : QStringLiteral("<b>")) + message + (valid ? QString() : QStringLiteral("</b>")) + "<br />\n";

    return i18nc("formatting the message for a command, as in encfs: not found", "%1: %2", command, htmlMessage);
}

} // namespace PlasmaVault
