/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "placesitem.h"
#include <QDebug>

#include <Solid/OpticalDisc>
#include <solid_version.h>

PlacesItem::PlacesItem(const QString &displayName,
                       QUrl url,
                       QObject *parent)
    : QObject(parent)
    , m_displayName(displayName)
    , m_url(url)
    , m_category("")
    , m_isOpticalDisc(false)
    , m_isAccessible(false)
{
}

QString PlacesItem::displayName() const
{
    return m_displayName;
}

void PlacesItem::setDisplayName(const QString &name)
{
    m_displayName = name;
}

QString PlacesItem::iconName() const
{
    return m_iconName;
}

void PlacesItem::setIconName(const QString &name)
{
    m_iconName = name;
}

QString PlacesItem::iconPath() const
{
    return m_iconPath;
}

void PlacesItem::setIconPath(const QString &path)
{
    m_iconPath = path;
}

QUrl PlacesItem::url() const
{
    return m_url;
}

void PlacesItem::setUrl(const QUrl &url)
{
    m_url = url;
}

QString PlacesItem::path() const
{
    if (m_url.isValid())
        return m_url.toString(QUrl::PreferLocalFile);

    return QString();
}

QString PlacesItem::udi() const
{
    return m_udi;
}

void PlacesItem::setUdi(const QString &udi)
{
    m_udi = udi;
    updateDeviceInfo(m_udi);
}

bool PlacesItem::isDevice()
{
    return !m_udi.isEmpty() && m_device.isValid();
}

bool PlacesItem::setupNeeded()
{
    if (m_access) {
        return !m_isAccessible;
    }

    return false;
}

void PlacesItem::updateDeviceInfo(const QString &udi)
{
    m_device = Solid::Device(udi);

    if (m_access)
        m_access->disconnect(this);

    if (m_device.isValid()) {
        m_access = m_device.as<Solid::StorageAccess>();
        m_iconName = m_device.icon();
        m_iconPath = QString("%1.svg").arg(m_iconName);

#if SOLID_VERSION_MAJOR >= 5 && SOLID_VERSION_MINOR >= 71
        m_displayName = m_device.displayName();
#else
        m_displayName = m_device.description();
#endif

        if (m_device.is<Solid::OpticalDisc>()) {
            m_isOpticalDisc = true;
            emit itemChanged(this);
        }

        if (m_access) {
            m_url = QUrl::fromLocalFile(m_access->filePath());
            connect(m_access.data(), &Solid::StorageAccess::accessibilityChanged, this, &PlacesItem::onAccessibilityChanged);
            onAccessibilityChanged(m_access->isAccessible());
        }
    } else {
        m_access = nullptr;
    }
}

void PlacesItem::onAccessibilityChanged(bool isAccessible)
{
    m_isAccessible = isAccessible;

    emit itemChanged(this);
}

QString PlacesItem::category() const
{
    return m_category;
}

void PlacesItem::setCategory(const QString &category)
{
    m_category = category;
}

bool PlacesItem::isOpticalDisc() const
{
    return m_isOpticalDisc;
}
