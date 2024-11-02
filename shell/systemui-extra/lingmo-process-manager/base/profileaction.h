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

#ifndef PROFILEACTION_H
#define PROFILEACTION_H

#include <string>
#include <memory>
#include <vector>
#include "profileattribute.h"
#include "resourcemanagerinterface.h"

class ProfileAction
{
public:
    virtual ~ProfileAction() = 0;
    virtual std::string name() const = 0;
    virtual std::string cgroupControllerName() const = 0;
    virtual void executeForCgroup(const std::string &path) = 0;
};

class SetAttributeAction : public ProfileAction
{
public:
    SetAttributeAction(
        std::unique_ptr<ProfileAttribute> attribute, const std::string &value,
        const std::shared_ptr<ResourceManagerInterface> &resourceInterface);
    ~SetAttributeAction() = default;
    std::string name() const override;
    std::string cgroupControllerName() const override;
    void executeForCgroup(const std::string &path) override;

private:
    std::unique_ptr<ProfileAttribute> m_attribute;
    std::string m_value;
    std::shared_ptr<ResourceManagerInterface> m_resourceInterface;
};

#endif // PROFILEACTION_H
