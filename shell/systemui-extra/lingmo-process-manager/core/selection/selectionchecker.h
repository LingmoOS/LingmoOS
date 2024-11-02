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

#ifndef SELECTIONCHECKER_H
#define SELECTIONCHECKER_H

#include <vector>
#include <string>

class SelectionChecker
{
public:
    SelectionChecker();
    bool hasSelection(const std::vector<int> &pids) const;

private:
    void checkPlatform();
    std::vector<int> getX11SelectionProcesses() const;
    std::vector<unsigned long> queryX11SelectionOwners() const;
    std::vector<int> getWaylandSelectionProcesses() const;
    int getSelectionPidFromKwin(const std::string &method) const;

private:
    bool m_isWaylandPlatform;
};

#endif // SELECTIONCHECKER_H
