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

#ifndef RESOURCECONTROLLER_H
#define RESOURCECONTROLLER_H

#include <string>
#include <vector>
#include "profileaction.h"

class ResourceController
{
public:
    ResourceController(const std::string &name);
    ResourceController(const ResourceController &) = delete;
    std::string name() const;
    std::vector<std::string> cgroupControllerNames() const;
    void addAction(std::unique_ptr<ProfileAction> action);
    void executeForCgroup(const std::string &path);

private:
    std::string m_name;
    std::vector<std::unique_ptr<ProfileAction>> m_actions;
};

#endif // RESOURCECONTROLLER_H
