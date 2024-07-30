/*
    SPDX-FileCopyrightText: 2009 Benjamin K. Stuhl <bks24@cornell.edu>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevqtdevice.h"
#include "udevqt_p.h"

#include <QByteArray>

namespace UdevQt
{
DevicePrivate::DevicePrivate(struct udev_device *udev_, bool ref)
    : udev(udev_)
{
    if (ref) {
        udev_device_ref(udev);
    }
}

DevicePrivate::~DevicePrivate()
{
    udev_device_unref(udev);
}

DevicePrivate &DevicePrivate::operator=(const DevicePrivate &other)
{
    udev_device_unref(udev);
    udev = udev_device_ref(other.udev);
    return *this;
}

QString DevicePrivate::decodePropertyValue(const QByteArray &encoded) const
{
    QByteArray decoded;
    const int len = encoded.length();

    for (int i = 0; i < len; i++) {
        quint8 ch = encoded.at(i);

        if (ch == '\\') {
            if (i + 1 < len && encoded.at(i + 1) == '\\') {
                decoded.append('\\');
                i++;
                continue;
            } else if (i + 3 < len && encoded.at(i + 1) == 'x') {
                QByteArray hex = encoded.mid(i + 2, 2);
                bool ok;
                int code = hex.toInt(&ok, 16);
                if (ok) {
                    decoded.append(char(code));
                }
                i += 3;
                continue;
            }
        } else {
            decoded.append(ch);
        }
    }
    return QString::fromUtf8(decoded);
}

Device::Device()
    : d(nullptr)
{
}

Device::Device(const Device &other)
{
    if (other.d) {
        d = new DevicePrivate(other.d->udev);
    } else {
        d = nullptr;
    }
}

Device::Device(DevicePrivate *devPrivate)
    : d(devPrivate)
{
}

Device::~Device()
{
    delete d;
}

Device &Device::operator=(const Device &other)
{
    if (this == &other) {
        return *this;
    }
    if (!other.d) {
        delete d;
        d = nullptr;
        return *this;
    }
    if (!d) {
        d = new DevicePrivate(other.d->udev);
    } else {
        *d = *other.d;
    }

    return *this;
}

bool Device::isValid() const
{
    return (d != nullptr);
}

QString Device::subsystem() const
{
    if (!d) {
        return QString();
    }

    return QString::fromLatin1(udev_device_get_subsystem(d->udev));
}

QString Device::devType() const
{
    if (!d) {
        return QString();
    }

    return QString::fromLatin1(udev_device_get_devtype(d->udev));
}

QString Device::name() const
{
    if (!d) {
        return QString();
    }

    return QString::fromLatin1(udev_device_get_sysname(d->udev));
}

QString Device::sysfsPath() const
{
    if (!d) {
        return QString();
    }

    return QString::fromLatin1(udev_device_get_syspath(d->udev));
}

int Device::sysfsNumber() const
{
    if (!d) {
        return -1;
    }

    QString value = QString::fromLatin1(udev_device_get_sysnum(d->udev));
    bool success = false;
    int number = value.toInt(&success);
    if (success) {
        return number;
    }
    return -1;
}

QString Device::driver() const
{
    if (!d) {
        return QString();
    }

    return QString::fromLatin1(udev_device_get_driver(d->udev));
}

QString Device::primaryDeviceFile() const
{
    if (!d) {
        return QString();
    }

    return QString::fromLatin1(udev_device_get_devnode(d->udev));
}

QStringList Device::alternateDeviceSymlinks() const
{
    if (!d) {
        return QStringList();
    }

    return listFromListEntry(udev_device_get_devlinks_list_entry(d->udev));
}

QStringList Device::deviceProperties() const
{
    if (!d) {
        return QStringList();
    }

    return listFromListEntry(udev_device_get_properties_list_entry(d->udev));
}

#ifdef UDEV_HAVE_GET_SYSATTR_LIST_ENTRY
QStringList Device::sysfsProperties() const
{
    if (!d) {
        return QStringList();
    }

    return listFromListEntry(udev_device_get_sysattr_list_entry(d->udev));
}
#endif

Device Device::parent() const
{
    if (!d) {
        return Device();
    }

    struct udev_device *p = udev_device_get_parent(d->udev);

    if (!p) {
        return Device();
    }

    return Device(new DevicePrivate(p));
}

QVariant Device::deviceProperty(const QString &name) const
{
    if (!d) {
        return QVariant();
    }

    QByteArray propName = name.toLatin1();
    QString propValue = QString::fromLatin1(udev_device_get_property_value(d->udev, propName.constData()));
    if (!propValue.isEmpty()) {
        return QVariant::fromValue(propValue);
    }
    return QVariant();
}

QString Device::decodedDeviceProperty(const QString &name) const
{
    if (!d) {
        return QString();
    }

    QByteArray propName = name.toLatin1();
    return d->decodePropertyValue(udev_device_get_property_value(d->udev, propName.constData()));
}

QVariant Device::sysfsProperty(const QString &name) const
{
    if (!d) {
        return QVariant();
    }

    QByteArray propName = name.toLatin1();
    QString propValue = QString::fromLatin1(udev_device_get_sysattr_value(d->udev, propName.constData()));
    if (!propValue.isEmpty()) {
        return QVariant::fromValue(propValue);
    }
    return QVariant();
}

Device Device::ancestorOfType(const QString &subsys, const QString &devtype) const
{
    if (!d) {
        return Device();
    }

    struct udev_device *p = udev_device_get_parent_with_subsystem_devtype(d->udev, subsys.toLatin1().constData(), devtype.toLatin1().constData());

    if (!p) {
        return Device();
    }

    return Device(new DevicePrivate(p));
}

}
