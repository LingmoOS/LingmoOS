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

#ifndef PROFILEATTRIBUTE_H
#define PROFILEATTRIBUTE_H

#include <string>

class ProfileAttribute
{
public:
    virtual ~ProfileAttribute() = 0;
    virtual std::string name() const = 0;
    virtual std::string controllerName() const = 0;
    virtual std::string fileName() const = 0;
};

class CgroupProfileAttribute : public ProfileAttribute
{
public:
    CgroupProfileAttribute(const std::string &name, 
                           const std::string &controllerName, 
                           const std::string &fileName);

    virtual ~CgroupProfileAttribute() = default;
    virtual std::string name() const override;
    virtual std::string controllerName() const override;
    virtual std::string fileName() const override;

private:
    std::string m_name;
    std::string m_controllerName;
    std::string m_fileName;
};
#endif // PROFILEATTRIBUTE_H
