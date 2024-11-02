/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UPOWERSYSTEMENERY_H
#define UPOWERSYSTEMENERY_H

#include "systemenergy.h"
#include <QDBusInterface>

class UPowerSystemEnery : public SystemEnergy
{
public:
    UPowerSystemEnery();
    ~UPowerSystemEnery() override = default;

    double energy() override;
    double dischargeRate() override;
    int timeToEmpty() override;

private:
    QVariant getUpowerProperty(const QString &propertyName);

private:
    QDBusInterface m_upowerDbusInterface;
};

#endif // UPOWERSYSTEMENERY_H
