/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#pragma once

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
    void init();

    int readBrightness() const;
    bool writeBrightness(int brightness) const;

    int readFromDevice(const QString &device, const QString &property) const;
    bool writeToDevice(const QString &device, int brightness) const;

    /**
     * The kernel offer from version 2.6.37 the type of the interface, and based on that
     * we can decide which interface is better for us, being the order
     * firmware-platform-raw
     */
    void initUsingBacklightType();
    QStringList getBacklightTypeDevices() const;

    bool m_isSupported = false;
    QString m_sysctlDevice;
    QList<int> m_sysctlBrightnessLevels;
    QList<QPair<QString /*device path*/, int /*max brightness*/>> m_devices;

    QVariantAnimation m_anim;
};
