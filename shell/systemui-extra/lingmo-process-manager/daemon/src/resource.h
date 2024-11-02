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

#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>
#include <string>

namespace resource {

enum class Resource {
    Unknown = -1,
    CPU,
    IO,
    Memory,
};

enum class ResourceUrgency {
    Unknown = -1,
    Low = 1,
    Medium = 2,
    High = 3,
};

//! \keyword ResourceUrgency, \value threshold
typedef std::map<ResourceUrgency, int> ResourceUrgencyThreshold;

std::string resourceEnumToString(Resource resource);
Resource stringToResourceEnum(const std::string& str);

int resourceUrgencyEnumToInt(ResourceUrgency urgency);
ResourceUrgency intToResourceUrgencyEnum(int urgency);
bool operator<(ResourceUrgency lhs, ResourceUrgency rhs);

} // namespace resource

#endif // RESOURCE_H
