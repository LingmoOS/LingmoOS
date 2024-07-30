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

#include "tabletdatabase.h"

#include "logging.h"

#include <QStandardPaths>

#include <KConfigGroup>

namespace Wacom
{
/**
 * Private class for the d-pointer.
 */
class TabletDatabasePrivate
{
public:
    QString locaDbFile; //!< the filename (without path) of the local tablet database file
    QString companyFile; //!< the filename (without path) of the company configuration file
    QString dataDirectory; //!< optional path to the data directory, used for unit tests
};
}

using namespace Wacom;

TabletDatabase::TabletDatabase()
    : d_ptr(new TabletDatabasePrivate)
{
    Q_D(TabletDatabase);
    d->locaDbFile = QLatin1String("tabletdblocalrc");
    d->companyFile = QLatin1String("companylist");
}

TabletDatabase::~TabletDatabase()
{
    delete this->d_ptr;
}

TabletDatabase &TabletDatabase::instance()
{
    static TabletDatabase instance;
    return instance;
}

QString TabletDatabase::lookupBackend(const QString &companyId) const
{
    KSharedConfig::Ptr companyConfig;

    if (!openCompanyConfig(companyConfig)) {
        return QString();
    }

    KConfigGroup companyGroup = KConfigGroup(companyConfig, companyId.toLower());

    if (companyGroup.keyList().isEmpty()) {
        qCInfo(COMMON) << QString::fromLatin1("Company with id '%1' could not be found in the tablet information database!").arg(companyId);
        return QString();
    }

    return companyGroup.readEntry("driver");
}

bool TabletDatabase::lookupTablet(const QString &tabletId, const QString &companyId, TabletInformation &tabletInfo) const
{
    KSharedConfig::Ptr companyConfig;

    KConfigGroup companyGroup;
    KConfigGroup tabletGroup;
    QString tabletsConfigFile;

    if (!openCompanyConfig(companyConfig)) {
        return false;
    }

    // get company group section
    companyGroup = KConfigGroup(companyConfig, companyId.toLower());

    // get tablet database configuration file for this company
    tabletsConfigFile = companyGroup.readEntry("listfile");

    if (tabletsConfigFile.isEmpty()) {
        qCWarning(COMMON) << QString::fromLatin1("Company group '%1' does not have a device list file!").arg(companyGroup.name());
        return false;
    }

    // lookup tablet
    if (lookupTabletGroup(tabletsConfigFile, tabletId, tabletGroup)) {
        // found tablet
        getInformation(tabletGroup, tabletId, companyId, companyGroup.readEntry("name"), tabletInfo);
        getButtonMap(tabletGroup, tabletInfo);
        return true;
    }

    return false;
}

bool TabletDatabase::lookupTablet(const QString &tabletId, TabletInformation &tabletInfo) const
{
    KSharedConfig::Ptr companyConfig;

    if (!openCompanyConfig(companyConfig)) {
        return false;
    }

    // find tablet
    KConfigGroup tabletGroup;

    // first check the localdb
    // here we support only Wacom tablet at the moment
    Q_D(const TabletDatabase);
    if (lookupTabletGroup(d->locaDbFile, tabletId, tabletGroup)) {
        // found tablet
        getInformation(tabletGroup, tabletId, QLatin1String("056a"), QLatin1String("Wacom Co., Ltd"), tabletInfo);
        getButtonMap(tabletGroup, tabletInfo);
        return true;
    } else {
        qCInfo(COMMON) << QString::fromLatin1("tablet %1 not in local db").arg(tabletId);
    }

    foreach (const QString &companyId, companyConfig->groupList()) {
        if (lookupTablet(tabletId, companyId, tabletInfo)) {
            return true;
        }
    }

    return false;
}

void TabletDatabase::setDatabase(const QString &dataDirectory, const QString &companyFileName)
{
    Q_D(TabletDatabase);
    d->dataDirectory = dataDirectory;
    d->companyFile = companyFileName;
}

bool TabletDatabase::getButtonMap(const KConfigGroup &deviceGroup, TabletInformation &tabletInfo) const
{
    QMap<QString, QString> buttonMap;
    int buttonNum = 1;
    QString buttonKey = QLatin1String("hwbutton1");

    while (deviceGroup.hasKey(buttonKey)) {
        buttonMap.insert(QString::number(buttonNum), deviceGroup.readEntry(buttonKey));
        buttonKey = QString::fromLatin1("hwbutton%1").arg(++buttonNum);
    }

    if (buttonMap.size() > 0) {
        tabletInfo.setButtonMap(buttonMap);
        return true;
    }

    return false;
}

bool TabletDatabase::getInformation(const KConfigGroup &deviceGroup,
                                    const QString &tabletId,
                                    const QString &companyId,
                                    const QString &companyName,
                                    TabletInformation &tabletInfo) const
{
    // tabletId, companyId & companyName are passed as parameter so all
    // tablet information data is set in one place and not all over this class.

    // get general information
    tabletInfo.set(TabletInfo::TabletId, tabletId.toUpper());
    tabletInfo.set(TabletInfo::CompanyId, companyId.toUpper());
    tabletInfo.set(TabletInfo::CompanyName, companyName);
    tabletInfo.set(TabletInfo::TabletModel, deviceGroup.readEntry("model"));
    tabletInfo.set(TabletInfo::TabletName, deviceGroup.readEntry("name"));
    tabletInfo.set(TabletInfo::ButtonLayout, deviceGroup.readEntry("layout"));
    tabletInfo.set(TabletInfo::NumPadButtons, deviceGroup.readEntry("padbuttons"));
    tabletInfo.set(TabletInfo::StatusLEDs, deviceGroup.readEntry("statusleds", QString::number(0)));
    tabletInfo.set(TabletInfo::TouchSensorId, deviceGroup.readEntry("touchsensorid"));
    tabletInfo.set(TabletInfo::IsTouchSensor, deviceGroup.readEntry("istouchsensor"));

    tabletInfo.setBool(TabletInfo::HasLeftTouchStrip, deviceGroup.readEntry("touchstripl"));
    tabletInfo.setBool(TabletInfo::HasRightTouchStrip, deviceGroup.readEntry("touchstripr"));
    tabletInfo.setBool(TabletInfo::HasTouchRing, deviceGroup.readEntry("touchring"));
    tabletInfo.setBool(TabletInfo::HasWheel, deviceGroup.readEntry("wheel"));

    return true;
}

bool TabletDatabase::lookupTabletGroup(const QString &tabletsConfigFile, const QString &tabletId, KConfigGroup &tabletGroup) const
{
    // open the device list file
    KSharedConfig::Ptr tabletConfig;

    if (!openConfig(tabletsConfigFile, tabletConfig)) {
        return false;
    }

    // lookup device
    tabletGroup = KConfigGroup(tabletConfig, tabletId.toUpper());

    if (tabletGroup.keyList().isEmpty()) {
        return false;
    }

    return true;
}

bool TabletDatabase::openConfig(const QString &configFileName, KSharedConfig::Ptr &configFile) const
{
    Q_D(const TabletDatabase);

    QString configFilePath;

    if (d->dataDirectory.isEmpty()) {
        configFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("wacomtablet/data/%1").arg(configFileName));
    } else {
        configFilePath = QString::fromLatin1("%1/%2").arg(d->dataDirectory).arg(configFileName);
    }
    // try to locate filename in the local config directory
    if (configFilePath.isEmpty()) {
        configFilePath = QStandardPaths::locate(QStandardPaths::ConfigLocation, configFileName);
    }

    if (configFilePath.isEmpty()) {
        qCWarning(COMMON) << QString::fromLatin1("Tablet database configuration file '%1' does not exist or is not accessible!").arg(configFileName);
        return false;
    }

    configFile = KSharedConfig::openConfig(configFilePath, KConfig::SimpleConfig, QStandardPaths::GenericDataLocation);

    if (configFile->groupList().isEmpty()) {
        qCWarning(COMMON) << QString::fromLatin1("Tablet database configuration file '%1' is empty or not readable!").arg(configFilePath);
        return false;
    }

    return true;
}

bool TabletDatabase::openCompanyConfig(KSharedConfig::Ptr &configFile) const
{
    Q_D(const TabletDatabase);

    QString configFileName;

    if (d->companyFile.isEmpty()) {
        configFileName = QLatin1String("companylist");
    } else {
        configFileName = d->companyFile;
    }

    return openConfig(configFileName, configFile);
}
