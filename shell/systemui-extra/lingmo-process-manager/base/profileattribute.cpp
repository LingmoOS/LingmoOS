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

#include "profileattribute.h"

ProfileAttribute::~ProfileAttribute() = default;

CgroupProfileAttribute::CgroupProfileAttribute(
    const std::string &name, const std::string &controllerName,
    const std::string &fileName)
    : m_name(name)
    , m_controllerName(controllerName)
    , m_fileName(fileName)
{
}

std::string CgroupProfileAttribute::name() const
{
    return m_name;
}

std::string CgroupProfileAttribute::controllerName() const
{
    return m_controllerName;
}

std::string CgroupProfileAttribute::fileName() const
{
    return m_fileName;
}
