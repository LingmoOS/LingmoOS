/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "kynmsystemdbus.h"
#include "lingmoipv4arping.h"
#include "lingmoipv6arping.h"

#define LOG_FLAG "[KynmSystemDbus]"

KynmSystemDbus::KynmSystemDbus(QObject *parent) : QObject(parent)
{

}

KynmSystemDbus::~KynmSystemDbus()
{

}

bool KynmSystemDbus::checkIpv4IsConflict(const QString devName, const QString ipv4Address, QStringList macList)
{
    bool isConflict = false;
    KyIpv4Arping* ipv4Arping = new KyIpv4Arping(devName, ipv4Address);
    if (ipv4Arping->ipv4ConflictCheck() >= 0) {
        isConflict =  ipv4Arping->ipv4IsConflict();
        if (isConflict && !macList.isEmpty()) {
            QString macAddress = ipv4Arping->getMacAddress();
            for (const auto mac : macList) {
                if (macAddress == mac) {
                    qDebug() << LOG_FLAG << "IPv4 conflict mac" << mac;
                    isConflict = false;
                    break;
                }
            }
        }
    } else {
        qWarning() << LOG_FLAG << "checkIpv4Conflict internal error";
    }

    delete ipv4Arping;
    ipv4Arping = nullptr;

    return isConflict;
}

bool KynmSystemDbus::checkIpv6IsConflict(const QString devName, const QString ipv6Address)
{
    bool isConflict = false;
    KyIpv6Arping* ipv6rping = new KyIpv6Arping(devName, ipv6Address);
    if (ipv6rping->ipv6ConflictCheck() >= 0) {
        isConflict =  ipv6rping->ipv6IsConflict();
    } else {
        qWarning() << LOG_FLAG << "checkIpv6Conflict internal error";
    }

    delete ipv6rping;
    ipv6rping = nullptr;

    return isConflict;
}
