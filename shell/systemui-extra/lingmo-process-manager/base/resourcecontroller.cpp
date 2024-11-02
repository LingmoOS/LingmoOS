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

#include "resourcecontroller.h"
#include <iostream>
#include <algorithm>

ResourceController::ResourceController(const std::string &name)
    : m_name(name)
{

}

std::string ResourceController::name() const
{
    return m_name;
}

std::vector<std::string> ResourceController::cgroupControllerNames() const
{
    std::vector<std::string> controllerNames;
    for (const auto &action : m_actions) {
        auto it = std::find(controllerNames.begin(), controllerNames.end(), action->cgroupControllerName());
        if (it == controllerNames.end()) {
            controllerNames.emplace_back(action->cgroupControllerName());
        }
    }
    return controllerNames;
}

void ResourceController::addAction(std::unique_ptr<ProfileAction> action) 
{
    m_actions.emplace_back(std::move(action));
}

void ResourceController::executeForCgroup(const std::string &path)
{
    for (const auto &action : m_actions) {
        action->executeForCgroup(path);
    }
}
