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

#include "profilemanager.h"

#include "logging.h"
#include "tabletprofileconfigadaptor.h"

#include <KSharedConfig>
#include <QtGlobal>

using namespace Wacom;

namespace Wacom
{
/**
 * Private class of the ProfileManager for the d-pointer
 *
 */
class ProfileManagerPrivate
{
public:
    QString fileName;
    QString tabletId;
    KConfigGroup tabletGroup;
    KSharedConfig::Ptr config;
};
}

ProfileManager::ProfileManager()
    : d_ptr(new ProfileManagerPrivate)
{
}

ProfileManager::ProfileManager(const QString &filename)
    : d_ptr(new ProfileManagerPrivate)
{
    open(filename);
}

ProfileManager::~ProfileManager()
{
    delete this->d_ptr;
}

void ProfileManager::close()
{
    Q_D(ProfileManager);

    d->tabletId.clear();
    d->tabletGroup = KConfigGroup();
    d->fileName.clear();
    d->config.reset();
}

bool ProfileManager::deleteProfile(const QString &profile)
{
    Q_D(ProfileManager);

    if (!isLoaded()) {
        return false;
    }

    KConfigGroup configGroup = KConfigGroup(&(d->tabletGroup), profile);

    if (configGroup.exists()) {
        configGroup.deleteGroup();
    }

    QStringList profileList = d->tabletGroup.readEntry(QLatin1String("ProfileRotationList"), QStringList());
    if (profileList.contains(profile)) {
        profileList.removeAll(profile);
        d->tabletGroup.writeEntry(QLatin1String("ProfileRotationList"), profileList);
    }

    d->tabletGroup.sync();
    return true;
}

bool ProfileManager::hasIdentifier(const QString &identifier) const
{
    Q_D(const ProfileManager);

    if (!isOpen()) {
        return false;
    }

    return KConfigGroup(d->config, identifier).exists();
}

void ProfileManager::setProfileRotationList(const QStringList &rotationList)
{
    Q_D(ProfileManager);

    if (!isOpen()) {
        return;
    }

    d->tabletGroup.writeEntry(QLatin1String("ProfileRotationList"), rotationList);
}

QStringList ProfileManager::profileRotationList() const
{
    Q_D(const ProfileManager);

    if (!isOpen()) {
        return QStringList();
    }

    return d->tabletGroup.readEntry(QLatin1String("ProfileRotationList"), QStringList());
}

int ProfileManager::currentProfileNumber() const
{
    Q_D(const ProfileManager);

    if (!isOpen()) {
        return -1;
    }

    return d->tabletGroup.readEntry(QLatin1String("CurrentProfileEntry"), -1);
}

int ProfileManager::profileNumber(const QString &profile) const
{
    if (!isOpen()) {
        return -1;
    }

    return profileRotationList().indexOf(profile);
}

void ProfileManager::updateCurrentProfileNumber(const QString &profile)
{
    Q_D(ProfileManager);

    if (!isOpen()) {
        return;
    }

    d->tabletGroup.writeEntry(QLatin1String("CurrentProfileEntry"), profileNumber(profile));
    d->tabletGroup.sync();
}

QString ProfileManager::nextProfile()
{
    Q_D(ProfileManager);

    if (!isOpen()) {
        return QString();
    }

    QStringList profileList = profileRotationList();
    if (profileList.isEmpty()) {
        return QString();
    }

    int curProfileEntry = d->tabletGroup.readEntry(QLatin1String("CurrentProfileEntry"), -1);
    curProfileEntry++;

    if (profileList.size() <= curProfileEntry) {
        curProfileEntry = 0;
    }

    d->tabletGroup.writeEntry(QLatin1String("CurrentProfileEntry"), curProfileEntry);
    d->tabletGroup.sync();
    return profileList.at(curProfileEntry);
}

QString ProfileManager::previousProfile()
{
    Q_D(ProfileManager);

    if (!isOpen()) {
        return QString();
    }

    QStringList profileList = profileRotationList();
    if (profileList.isEmpty()) {
        return QString();
    }

    int curProfileEntry = d->tabletGroup.readEntry(QLatin1String("CurrentProfileEntry"), -1);
    curProfileEntry--;

    if (curProfileEntry < 0) {
        curProfileEntry = profileList.size() - 1;
    }

    d->tabletGroup.writeEntry(QLatin1String("CurrentProfileEntry"), curProfileEntry);
    d->tabletGroup.sync();
    return profileList.at(curProfileEntry);
}

bool ProfileManager::hasProfile(const QString &profileName) const
{
    Q_D(const ProfileManager);

    if (!isLoaded() || profileName.isEmpty()) {
        return false;
    }

    return KConfigGroup(&(d->tabletGroup), profileName).exists();
}

bool ProfileManager::isLoaded() const
{
    Q_D(const ProfileManager);
    return (isOpen() && !d->tabletId.isEmpty());
}

bool ProfileManager::isOpen() const
{
    Q_D(const ProfileManager);
    return (!d->fileName.isEmpty() && d->config);
}

const QStringList ProfileManager::listIdentifiers() const
{
    Q_D(const ProfileManager);

    if (!isOpen()) {
        return QStringList();
    }

    return d->config->groupList();
}

const QStringList ProfileManager::listProfiles() const
{
    Q_D(const ProfileManager);

    if (!isLoaded()) {
        return QStringList();
    }

    return d->tabletGroup.groupList();
}

bool ProfileManager::readProfiles(const QString &tabletIdentifier, const QString &legacyTabletId)
{
    Q_D(ProfileManager);

    if (!isOpen() || tabletIdentifier.isEmpty()) {
        d->tabletId = QString();
        return false;
    }

    d->config->reparseConfiguration();
    d->tabletId = tabletIdentifier;
    d->tabletGroup = KConfigGroup(d->config, d->tabletId);

    if (!d->tabletGroup.exists() && !legacyTabletId.isEmpty()) {
        // Check for old configuration group
        auto legacyGroup = KConfigGroup(d->config, legacyTabletId);
        if (legacyGroup.exists()) {
            qCInfo(COMMON) << "Copying legacy tablet config:" << legacyTabletId << "to" << tabletIdentifier;
            legacyGroup.copyTo(&d->tabletGroup);
        }
    }

    return true;
}

const TabletProfile ProfileManager::loadProfile(const QString &profile) const
{
    Q_D(const ProfileManager);
    TabletProfile tabletProfile(profile);

    if (!isLoaded() || profile.isEmpty()) {
        return tabletProfile;
    }

    KConfigGroup configGroup(&(d->tabletGroup), profile);

    if (!configGroup.exists()) {
        return tabletProfile;
    }

    TabletProfileConfigAdaptor configAdaptor(tabletProfile);
    configAdaptor.loadConfig(configGroup);

    return tabletProfile;
}

void ProfileManager::open(const QString &filename)
{
    Q_D(ProfileManager);

    close();

    if (!filename.isEmpty()) {
        d->fileName = filename;
        d->config = KSharedConfig::openConfig(filename, KConfig::SimpleConfig);
    }
}

void ProfileManager::reload()
{
    Q_D(ProfileManager);

    if (isOpen()) {
        d->config->reparseConfiguration();
    }
}

bool ProfileManager::saveProfile(TabletProfile &tabletProfile)
{
    Q_D(ProfileManager);

    QString profileName = tabletProfile.getName();

    if (!isLoaded() || profileName.isEmpty()) {
        qCWarning(COMMON)
            << QString::fromLatin1("Can not save profile '%1' as it either does not have a name or no configuration file was opened!").arg(profileName);
        return false;
    }

    KConfigGroup configGroup = KConfigGroup(&(d->tabletGroup), profileName);

    if (configGroup.exists()) {
        configGroup.deleteGroup();
    }

    TabletProfileConfigAdaptor configAdaptor(tabletProfile);

    if (!configAdaptor.saveConfig(configGroup)) {
        return false;
    }

    configGroup.sync();

    return true;
}
