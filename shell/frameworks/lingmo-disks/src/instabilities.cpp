// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#include "instabilities.h"

#include <KLocalizedString>

#include <QMetaEnum>

#include "smartdata.h"

static QString smartCtlDataFailureToInstabilityString(SMART::Failure failure)
{
    switch (failure) {
    case SMART::Failure::None:
    case SMART::Failure::CmdLineParse:
    case SMART::Failure::DeviceOpen:
    case SMART::Failure::InternalCommand:
        // These are kind of internal failures that the user cannot really
        // do anything about and they aren't necessarily indicative of
        // anything wrong with the drive.
        return {};
    case SMART::Failure::Disk:
        // This is reflected as failure.
        return {};
    case SMART::Failure::Prefail:
        return i18nc("@label", "Prefail attributes <= threshold.");
    case SMART::Failure::PastPrefail:
        return i18nc(
            "@label",
            "SMART status check returned 'DISK OK' but we found that some (usage or prefail) attributes have been <= threshold at some time in the past.");
    case SMART::Failure::ErrorsRecorded:
        return i18nc("@label", "The device error log contains records of errors.");
    case SMART::Failure::SelfTestErrors:
        return i18nc(
            "@label",
            "The device self-test log contains records of errors. [ATA only] Failed self-tests outdated by a newer successful extended self-test are ignored.");
    }
    Q_UNREACHABLE();
    return {};
}

QStringList Instabilities::from(const SMARTData &data)
{
    QStringList list;
    const SMARTCtlData ctlData = data.m_smartctl;
    const auto failureEnum = QMetaEnum::fromType<SMART::Failure>();
    for (auto i = 0; i < failureEnum.keyCount(); ++i) {
        const auto fail = static_cast<SMART::Failure>(failureEnum.value(i));
        const bool flagSet = ctlData.failure().testFlag(fail);
        if (!flagSet) {
            continue;
        }
        const QString instability = smartCtlDataFailureToInstabilityString(fail);
        if (!instability.isEmpty()) {
            list << instability;
        }
    }
    return list;
}
