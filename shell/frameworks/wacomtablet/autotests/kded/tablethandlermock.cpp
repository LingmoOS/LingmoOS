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

#include "tablethandlermock.h"

using namespace Wacom;

TabletHandlerMock::TabletHandlerMock()
    : TabletHandlerInterface(nullptr)
{
    m_profile = QLatin1String("default");

    m_profiles.append(QLatin1String("default"));
    m_profiles.append(QLatin1String("myprofile"));
}

TabletHandlerMock::~TabletHandlerMock()
{
}

void TabletHandlerMock::emitProfileChanged(const QString &tabletId, const QString &profile)
{
    emit profileChanged(tabletId, profile);
}

void TabletHandlerMock::emitTabletAdded(const TabletInformation &info)
{
    emit tabletAdded(info);
}

void TabletHandlerMock::emitTabletRemoved(const QString &tabletId)
{
    emit tabletRemoved(tabletId);
}

QString TabletHandlerMock::getProperty(const QString &tabletId, const DeviceType &deviceType, const Property &property) const
{
    Q_UNUSED(tabletId)
    QString prop = property.key();

    if (m_deviceType != deviceType.key()) {
        return QLatin1String("ERROR: DEVICE DOES NOT MATCH!");
    }

    if (m_property.compare(prop, Qt::CaseInsensitive) != 0) {
        return QLatin1String("ERROR: PROPERTY DOES NOT MATCH!");
    }

    return m_propertyValue;
}

QStringList TabletHandlerMock::listProfiles(const QString &tabletId)
{
    Q_UNUSED(tabletId)
    return m_profiles;
}

void TabletHandlerMock::setProfile(const QString &tabletId, const QString &profile)
{
    m_profile = profile;
    emitProfileChanged(tabletId, profile);
}

void TabletHandlerMock::setProperty(const QString &tabletId, const DeviceType &deviceType, const Property &property, const QString &value)
{
    Q_UNUSED(tabletId)
    m_deviceType = deviceType.key();
    m_property = property.key();
    m_propertyValue = value;
}

QStringList TabletHandlerMock::getProfileRotationList(const QString &tabletId)
{
    Q_UNUSED(tabletId)
    return m_rotationList;
}

void TabletHandlerMock::setProfileRotationList(const QString &tabletId, const QStringList &rotationList)
{
    Q_UNUSED(tabletId)
    m_rotationList = rotationList;
}

#include "moc_tablethandlermock.cpp"
