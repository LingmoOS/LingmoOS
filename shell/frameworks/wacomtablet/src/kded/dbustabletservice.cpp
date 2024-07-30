/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbustabletservice.h"

#include "dbustabletinterface.h"
#include "devicetype.h"
#include "logging.h"
#include "property.h"
#include "tabletinfo.h"
#include "wacomadaptor.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMetaType>

using namespace Wacom;

namespace Wacom
{
class DBusTabletServicePrivate
{
public:
    WacomAdaptor *wacomAdaptor = nullptr;
    TabletHandlerInterface *tabletHandler = nullptr;
    QHash<QString, TabletInformation> tabletInformationList; //!< Information of all currently connected tablets.
    QHash<QString, QString> currentProfileList; //!< Currently active profile for each tablet.
}; // CLASS
} // NAMESPACE

DBusTabletService::DBusTabletService(TabletHandlerInterface &tabletHandler)
    : QObject()
    , d_ptr(new DBusTabletServicePrivate)
{
    Q_D(DBusTabletService);

    d->tabletHandler = &tabletHandler;

    DBusTabletInterface::registerMetaTypes();

    d->wacomAdaptor = new WacomAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/Tablet"), this);
    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.Wacom"));
}

DBusTabletService::~DBusTabletService()
{
    QDBusConnection::sessionBus().unregisterService(QLatin1String("org.kde.Wacom"));
    QDBusConnection::sessionBus().unregisterObject(QLatin1String("/Tablet"));
    delete d_ptr->wacomAdaptor;

    delete d_ptr;
}

const QStringList DBusTabletService::getTabletList() const
{
    Q_D(const DBusTabletService);
    return d->tabletInformationList.keys();
}

const QStringList DBusTabletService::getDeviceList(const QString &tabletId) const
{
    Q_D(const DBusTabletService);
    return d->tabletInformationList.value(tabletId).getDeviceList();
}

QString DBusTabletService::getDeviceName(const QString &tabletId, const QString &device) const
{
    Q_D(const DBusTabletService);

    static const QString unknown;

    const DeviceType *type = DeviceType::find(device);

    if (!type) {
        qCWarning(KDED) << QString::fromLatin1("Unsupported device type '%1'!").arg(device);
        return unknown;
    }

    return d->tabletInformationList.value(tabletId).getDeviceName(*type);
}

QString DBusTabletService::getInformation(const QString &tabletId, const QString &info) const
{
    Q_D(const DBusTabletService);

    static const QString unknown;

    const TabletInfo *devinfo = TabletInfo::find(info);

    if (!devinfo) {
        qCWarning(KDED) << QString::fromLatin1("Can not get unsupported tablet information '%1'!").arg(info);
        return unknown;
    }

    return d->tabletInformationList.value(tabletId).get(*devinfo);
}

QString DBusTabletService::getProfile(const QString &tabletId) const
{
    Q_D(const DBusTabletService);
    return d->currentProfileList.value(tabletId);
}

QString DBusTabletService::getProperty(const QString &tabletId, const QString &deviceType, const QString &property) const
{
    Q_D(const DBusTabletService);

    const DeviceType *type = DeviceType::find(deviceType);

    if (!type) {
        qCWarning(KDED) << QString::fromLatin1("Can not get property '%1' from invalid device '%2'!").arg(property).arg(deviceType);
        return QString();
    }

    const Property *prop = Property::find(property);

    if (!prop) {
        qCWarning(KDED) << QString::fromLatin1("Can not get invalid property '%1' from device '%2'!").arg(property).arg(deviceType);
        return QString();
    }

    return d->tabletHandler->getProperty(tabletId, *type, *prop);
}

bool DBusTabletService::hasPadButtons(const QString &tabletId) const
{
    Q_D(const DBusTabletService);
    return d->tabletInformationList.value(tabletId).hasButtons();
}

bool DBusTabletService::isAvailable(const QString &tabletId) const
{
    Q_D(const DBusTabletService);
    return d->tabletInformationList.contains(tabletId);
}

QStringList DBusTabletService::listProfiles(const QString &tabletId)
{
    Q_D(const DBusTabletService);
    return d->tabletHandler->listProfiles(tabletId);
}

void DBusTabletService::setProfile(const QString &tabletId, const QString &profile)
{
    Q_D(DBusTabletService);
    d->tabletHandler->setProfile(tabletId, profile);
}

void DBusTabletService::setProperty(const QString &tabletId, const QString &deviceType, const QString &property, const QString &value)
{
    Q_D(DBusTabletService);

    const DeviceType *type = DeviceType::find(deviceType);

    if (!type) {
        qCWarning(KDED) << QString::fromLatin1("Can not set property '%1' on invalid device '%2' to '%3'!").arg(property).arg(deviceType).arg(value);
        return;
    }

    const Property *prop = Property::find(property);

    if (!prop) {
        qCWarning(KDED) << QString::fromLatin1("Can not set invalid property '%1' on device '%2' to '%3'!").arg(property).arg(deviceType).arg(value);
        return;
    }

    d->tabletHandler->setProperty(tabletId, *type, *prop, value);
}

QStringList DBusTabletService::getProfileRotationList(const QString &tabletId)
{
    Q_D(DBusTabletService);
    return d->tabletHandler->getProfileRotationList(tabletId);
}

void DBusTabletService::setProfileRotationList(const QString &tabletId, const QStringList &rotationList)
{
    Q_D(DBusTabletService);
    d->tabletHandler->setProfileRotationList(tabletId, rotationList);
}

QString DBusTabletService::getTouchSensorId(const QString &tabletId)
{
    Q_D(DBusTabletService);
    return d->tabletInformationList.value(tabletId).get(TabletInfo::TouchSensorId);
}

bool DBusTabletService::isTouchSensor(const QString &tabletId)
{
    Q_D(DBusTabletService);
    return d->tabletInformationList.value(tabletId).getBool(TabletInfo::IsTouchSensor);
}

void DBusTabletService::onProfileChanged(const QString &tabletId, const QString &profile)
{
    Q_D(DBusTabletService);

    d->currentProfileList.insert(tabletId, profile);

    emit profileChanged(tabletId, profile);
}

void DBusTabletService::onTabletAdded(const TabletInformation &info)
{
    Q_D(DBusTabletService);

    d->tabletInformationList.insert(info.get(TabletInfo::TabletId), info);

    emit tabletAdded(info.get(TabletInfo::TabletId));
}

void DBusTabletService::onTabletRemoved(const QString &tabletId)
{
    Q_D(DBusTabletService);

    d->currentProfileList.remove(tabletId);
    d->tabletInformationList.remove(tabletId);

    emit tabletRemoved(tabletId);
}

#include "moc_dbustabletservice.cpp"
