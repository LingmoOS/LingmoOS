/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filesystemdata.h"
#include <QDebug>

FileSystemData::FileSystemData()
{
}

FileSystemData::~FileSystemData()
{
}

const QString FileSystemData::deviceName() const
{
    return m_devName;
}

void FileSystemData::setDevName(const QString &name)
{
    if (name != m_devName)
        m_devName = name;
}

const QString FileSystemData::mountDir() const
{
    return m_mountDir;
}

const QString FileSystemData::diskType() const
{
    return m_diskType;
}

const QString FileSystemData::totalCapacity() const
{
    return m_totalCapacity;
}

const QString FileSystemData::freeCapacity() const
{
    return m_freeCapacity;
}

const QString FileSystemData::availCapacity() const
{
    return m_availCapacity;
}

const QString FileSystemData::usedCapactiy() const
{
    return m_usedCapactiy;
}

int FileSystemData::usedPercentage() const
{
    return m_percentage;
}

qreal FileSystemData::totalCapacityValue() const
{
    return m_lfTotalCapcity;
}

qreal FileSystemData::freeCapacityValue() const
{
    return m_lfFreeCapcity;
}

qreal FileSystemData::avalidCapacityValue() const
{
    return m_lfAvalidCapcity;
}

qreal FileSystemData::usedCapacityValue() const
{
    return m_lfUsedCapcity;
}

void FileSystemData::updateDiskInfo(QString mountDir, QString diskType, QString totalCapacity, QString freeCapacity, 
        QString availCapacity, QString usedCapactiy, int percentage, qreal lfTotalCapcity, qreal lfFreeCapcity, 
        qreal lfAvalidCapcity, qreal lfUsedCapcity)
{
    if (mountDir != m_mountDir)
        m_mountDir = mountDir;
    if (diskType != m_diskType)
        m_diskType = diskType;
    if (totalCapacity != m_totalCapacity)
        m_totalCapacity = totalCapacity;
    if (freeCapacity != m_freeCapacity)
        m_freeCapacity = freeCapacity;
    if (availCapacity != m_availCapacity)
        m_availCapacity = availCapacity;
    if (usedCapactiy != m_usedCapactiy)
        m_usedCapactiy = usedCapactiy;
    if (percentage != m_percentage)
        m_percentage = percentage;
    m_lfTotalCapcity = lfTotalCapcity;
    m_lfFreeCapcity = lfFreeCapcity;
    m_lfAvalidCapcity = lfAvalidCapcity;
    m_lfUsedCapcity = lfUsedCapcity;
}
