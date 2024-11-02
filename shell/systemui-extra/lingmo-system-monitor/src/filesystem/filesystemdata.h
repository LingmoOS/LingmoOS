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


#ifndef FILESYSTEMDATA_H
#define FILESYSTEMDATA_H

#include <QObject>
#include <QString>

class FileSystemData
{
public:
    FileSystemData();
    virtual ~FileSystemData();

    const QString deviceName() const;
    void setDevName(const QString &name);

    const QString mountDir() const;
    const QString diskType() const;
    const QString totalCapacity() const;
    const QString freeCapacity() const;
    const QString availCapacity() const;
    const QString usedCapactiy() const;
    int usedPercentage() const;
    qreal totalCapacityValue() const;
    qreal freeCapacityValue() const;
    qreal avalidCapacityValue() const;
    qreal usedCapacityValue() const;

    void updateDiskInfo(QString mountDir, QString diskType, QString totalCapacity, QString freeCapacity, 
        QString availCapacity, QString usedCapactiy, int percentage, qreal lfTotalCapcity, qreal lfFreeCapcity, 
        qreal lfAvalidCapcity, qreal lfUsedCapcity);

private:
    QString m_devName;
    QString m_mountDir;
    QString m_diskType;
    QString m_totalCapacity;
    QString m_freeCapacity;
    QString m_availCapacity;
    QString m_usedCapactiy;
    int m_percentage = 0;
    qreal m_lfTotalCapcity = 0;
    qreal m_lfFreeCapcity = 0; 
    qreal m_lfAvalidCapcity = 0; 
    qreal m_lfUsedCapcity = 0; 
};

#endif // FILESYSTEMDATA_H

