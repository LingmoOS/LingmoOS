/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kameleonhelper.h"
#include "kameleonhelper_debug.h"

#include <KAuth/HelperSupport>

#include <QDebug>
#include <QFile>
#include <QFileInfo>

KAuth::ActionReply KameleonHelper::writecolor(const QVariantMap &args)
{
    QStringList devices = args.value(QStringLiteral("devices")).toStringList();
    QStringList colors = args.value(QStringLiteral("colors")).toStringList();
    if (devices.length() != colors.length()) {
        qCWarning(KAMELEONHELPER) << "lists of devices and colors do not match in length";
        return KAuth::ActionReply::HelperErrorReply();
    }
    for (int i = 0; i < devices.length(); ++i) {
        QString device = devices.at(i);
        if (device.contains(QLatin1String("..")) || device.contains(QLatin1Char('/'))) {
            qCWarning(KAMELEONHELPER) << "invalid device name" << device;
            return KAuth::ActionReply::HelperErrorReply();
        }

        QByteArray color = colors.at(i).toUtf8();
        if (!(color.length() >= QByteArray("0 0 0").length() && color.length() <= QByteArray("255 255 255").length())) {
            qCWarning(KAMELEONHELPER) << "invalid RGB color" << color << "for device" << device;
            return KAuth::ActionReply::HelperErrorReply();
        }

        QFile file(LED_SYSFS_PATH + device + LED_RGB_FILE);
        if (!file.open(QIODevice::WriteOnly | QIODevice::ExistingOnly)) {
            qCWarning(KAMELEONHELPER) << "Opening" << file.fileName() << "failed:" << file.error() << file.errorString();
            return KAuth::ActionReply::HelperErrorReply();
        }
        const int bytesWritten = file.write(color);
        if (bytesWritten == -1) {
            qCWarning(KAMELEONHELPER) << "writing to" << file.fileName() << "failed:" << file.error() << file.errorString();
            return KAuth::ActionReply::HelperErrorReply();
        }
        qCDebug(KAMELEONHELPER) << "wrote" << color << "to" << device;
    }

    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.kameleonhelper", KameleonHelper)

#include "moc_kameleonhelper.cpp"
