/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2012 Alberto Villa <avilla@FreeBSD.org>
 *    SPDX-FileCopyrightText: 2023 Serenity Cyber Security LLC <license@futurecrew.ru>
 *                       Author: Gleb Popov <arrowd@FreeBSD.org>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#ifndef BACKLIGHTHELPER_H
#define BACKLIGHTHELPER_H

#include <QObject>
#include <QVariantAnimation>

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>
#include <kauth_version.h>

using namespace KAuth;

class BacklightHelper : public QObject
{
    Q_OBJECT
public:
    explicit BacklightHelper(QObject *parent = nullptr);

public Q_SLOTS:
    ActionReply brightness(const QVariantMap &args);
    ActionReply brightnessmax(const QVariantMap &args);
    ActionReply setbrightness(const QVariantMap &args);
    ActionReply syspath(const QVariantMap &args);

private:
    int readBrightness() const;
    bool writeBrightness(int brightness) const;

    int readFromDevice(const QString &device) const;
    bool writeToDevice(const QString &device, int brightness) const;

    /**
     * FreeBSD (and other BSDs) can control backlight via acpi_video(4)
     */
    void initUsingSysctl();
    /**
     * FreeBSD 13+ has a dedicated command line utility backlight(8)
     */
    void initUsingFreeBSDBacklight();

    bool m_isSupported = false;
    QString m_sysctlDevice;
    QList<int> m_sysctlBrightnessLevels;
    QList<QPair<QString /*device path*/, int /*max brightness*/>> m_devices;

    QVariantAnimation m_anim;
};

#endif // BACKLIGHTHELPER_H
