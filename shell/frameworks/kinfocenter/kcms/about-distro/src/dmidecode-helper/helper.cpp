// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

#include "helper.h"

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <array>

#include <KAuth/HelperSupport>

template<typename Output, typename... Input>
auto make_array(Input &&...args) -> std::array<Output, sizeof...(args)> // NB: we need suffix notation here so args is defined
{
    return {std::forward<Input>(args)...};
}

KAuth::ActionReply DMIDecodeHelper::systeminformation(const QVariantMap &args)
{
    Q_UNUSED(args);

    // PATH is super minimal when invoked through dbus
    setenv("PATH", "/usr/sbin:/sbin:/usr/local/sbin", 1);
    const QString dmidecode = QStandardPaths::findExecutable("dmidecode");
    if (dmidecode.isEmpty()) {
        return KAuth::ActionReply::HelperErrorReply();
    }

    // NB: Microsoft also outlines a limited set of DMI values to be required for IOT OEM licensing, as such we
    //   can rely on the same fields to have sound content . Since this only applies to OEMs we still need to filter
    //   out dummy values though and because of that we can grab more fields, since we'll filter them anyway.
    // https://docs.microsoft.com/en-us/windows-hardware/manufacture/iot/license-requirements?view=windows-11#smbios-support

    KAuth::ActionReply reply;
    for (const auto &key : {QStringLiteral("system-manufacturer"),
                            QStringLiteral("system-product-name"),
                            QStringLiteral("system-version"),
                            QStringLiteral("system-serial-number")}) {
        QProcess proc;
        proc.start(dmidecode, {QStringLiteral("--string"), key});
        proc.waitForFinished();
        const QByteArray output = proc.readAllStandardOutput().trimmed();

        if (output.isEmpty() || proc.error() != QProcess::UnknownError || proc.exitStatus() != QProcess::NormalExit) {
            continue;
        }

        // Fairly exhaustive filter list based on a dozen or so samples gathered from reddit and other places.
        // These are values that may appear in the DMI system information but aren't really useful.
        static const auto dummyData = make_array<QString>(QStringLiteral("system version"),
                                                          QStringLiteral("system product name"),
                                                          QStringLiteral("system serial number"),
                                                          QStringLiteral("system manufacturer"),
                                                          QStringLiteral("to be filled by o.e.m."),
                                                          QStringLiteral("standard"), /* sometimes the version is useless */
                                                          QStringLiteral("sku"),
                                                          QStringLiteral("default string"),
                                                          QStringLiteral("not specified"),
                                                          QStringLiteral("not applicable")
                                                          /* may also be empty, but that is filtered above already */);
        if (std::find(dummyData.cbegin(), dummyData.cend(), output.toLower()) != dummyData.cend()) {
            continue;
        }

        reply.addData(key, output);
    }
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kinfocenter.dmidecode", DMIDecodeHelper)

#include "moc_helper.cpp"
