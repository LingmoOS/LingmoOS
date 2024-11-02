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

#include "resource.h"

#include <QDebug>

namespace resource {

ResourceUrgency intToResourceUrgencyEnum(int urgency)
{
    ResourceUrgency result = ResourceUrgency::Unknown;
    switch(urgency) {
    case 0:
        result = ResourceUrgency::Low;
        break;
    case 1:
        result = ResourceUrgency::Medium;
        break;
    case 2:
        result = ResourceUrgency::High;
        break;
    default:
        qWarning() << "Translate int to ResourceUrgency enmu error,"
                   << "parameter" << urgency << "isn't supported.";
        break;
    }
    return result;
}

int resourceUrgencyEnumToInt(ResourceUrgency urgency) {
    int result;
    switch(urgency) {
    case ResourceUrgency::Low:
        result = 0;
        break;
    case ResourceUrgency::Medium:
        result = 1;
        break;
    case ResourceUrgency::High:
        result = 2;
        break;
    default:
        result = -1;
    }
    return result;
}

bool operator<(ResourceUrgency lhs, ResourceUrgency rhs) {
    return resourceUrgencyEnumToInt(lhs) < resourceUrgencyEnumToInt(rhs);
}

Resource stringToResourceEnum(const std::string& str) {
    Resource result = Resource::Unknown;
    if ("CPU" == str)
        result = Resource::CPU;
    else if ("IO" == str)
        result = Resource::IO;
    else if ("Memory" == str)
        result = Resource::Memory;
    else
        qWarning() << "Translate string to Resource enmu error,"
                   << "parameter" << str.c_str() << "isn't supported.";

    return result;
}

std::string resourceEnumToString(Resource resource) {
    std::string resourceStr;
    switch (resource) {
    case Resource::CPU:
        resourceStr = "CPU";
        break;
    case Resource::IO:
        resourceStr = "IO";
        break;
    case Resource::Memory:
        resourceStr = "Memory";
        break;
    default:
        break;
    }
    return resourceStr;
}
} // namespace resource
