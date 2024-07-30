/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Anant Kamath <kamathanant@gmail.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGER_FAKE_DBUS_H
#define MODEMMANAGER_FAKE_DBUS_H

#define DBUS_INTERFACE_PROPS QLatin1String("org.freedesktop.DBus.Properties")
#define DBUS_INTERFACE_INTROSPECT QLatin1String("org.freedesktop.DBus.Introspectable")
#define DBUS_INTERFACE_MANAGER QLatin1String("org.freedesktop.DBus.ObjectManager")

#define MMQT_DBUS_SERVICE "org.kde.fakemodem"
#define MMQT_DBUS_PATH "/org/kde/fakemodem"
/* Prefix for object paths */
#define MMQT_DBUS_MODEM_PREFIX MMQT_DBUS_PATH "/Modem"
#define MMQT_DBUS_BEARER_PREFIX MMQT_DBUS_PATH "/Bearer"
#define MMQT_DBUS_SIM_PREFIX MMQT_DBUS_PATH "/SIM"
#define MMQT_DBUS_SMS_PREFIX MMQT_DBUS_PATH "/SMS"
#define MMQT_DBUS_CALL_PREFIX MMQT_DBUS_PATH "/Call"
/* Interfaces */
#define MMQT_DBUS_INTERFACE "org.kde.fakemodem"
#define MMQT_DBUS_INTERFACE_SIM "org.kde.fakemodem.Sim"
#define MMQT_DBUS_INTERFACE_BEARER "org.kde.fakemodem.Bearer"
#define MMQT_DBUS_INTERFACE_SMS "org.kde.fakemodem.Sms"
#define MMQT_DBUS_INTERFACE_CALL "org.kde.fakemodem.Call"
#define MMQT_DBUS_INTERFACE_MODEM "org.kde.fakemodem.Modem"
#define MMQT_DBUS_INTERFACE_MODEM_SIMPLE "org.kde.fakemodem.Modem.Simple"
#define MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP "org.kde.fakemodem.Modem.Modem3gpp"
#define MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP_USSD "org.kde.fakemodem.Modem.Modem3gpp.Ussd"
#define MMQT_DBUS_INTERFACE_MODEM_MODEMCDMA "org.kde.fakemodem.Modem.ModemCdma"
#define MMQT_DBUS_INTERFACE_MODEM_MESSAGING "org.kde.fakemodem.Modem.Messaging"
#define MMQT_DBUS_INTERFACE_MODEM_LOCATION "org.kde.fakemodem.Modem.Location"
#define MMQT_DBUS_INTERFACE_MODEM_TIME "org.kde.fakemodem.Modem.Time"
#define MMQT_DBUS_INTERFACE_MODEM_FIRMWARE "org.kde.fakemodem.Modem.Firmware"
#define MMQT_DBUS_INTERFACE_MODEM_SIGNAL "org.kde.fakemodem.Modem.Signal"
#define MMQT_DBUS_INTERFACE_MODEM_OMA "org.kde.fakemodem.Modem.Oma"
#define MMQT_DBUS_INTERFACE_MODEM_VOICE "org.kde.fakemodem.Modem.Voice"

#endif
