/*
    SPDX-FileCopyrightText: 2009 Benjamin K. Stuhl <bks24@cornell.edu>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

extern "C" {
#define LIBUDEV_I_KNOW_THE_API_IS_SUBJECT_TO_CHANGE
#include <libudev.h>
}

class QByteArray;
class QSocketNotifier;

namespace UdevQt
{
class DevicePrivate
{
public:
    explicit DevicePrivate(struct udev_device *udev_, bool ref = true);
    ~DevicePrivate();
    DevicePrivate &operator=(const DevicePrivate &other);

    QString decodePropertyValue(const QByteArray &encoded) const;

    struct udev_device *udev;
};

class Client;
class ClientPrivate
{
public:
    enum ListenToWhat {
        ListenToList,
        ListenToNone,
    };

    explicit ClientPrivate(Client *q_);
    ~ClientPrivate();

    void init(const QStringList &subsystemList, ListenToWhat what);
    void setWatchedSubsystems(const QStringList &subsystemList);
    void _uq_monitorReadyRead(int fd);
    DeviceList deviceListFromEnumerate(struct udev_enumerate *en);

    struct udev *udev;
    struct udev_monitor *monitor;
    Client *q;
    QSocketNotifier *monitorNotifier;
    QStringList watchedSubsystems;
};

inline QStringList listFromListEntry(struct udev_list_entry *list)
{
    QStringList ret;
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, list)
    {
        ret << QString::fromLatin1(udev_list_entry_get_name(entry));
    }
    return ret;
}

}
