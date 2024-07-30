/*
    SPDX-FileCopyrightText: 2024 Natalie Clarius <natalie.clarius@kde.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef _KAMELEONHELPER_H_
#define _KAMELEONHELPER_H_

#include <QObject>

#include <KAuth/ActionReply>

#define LED_SYSFS_PATH "/sys/class/leds/"
#define LED_RGB_FILE "/multi_intensity"

class KameleonHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    KAuth::ActionReply writecolor(const QVariantMap &args);
};

#endif // _KameleonHELPER_H_
