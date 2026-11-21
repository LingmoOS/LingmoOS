// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "processrecord.h"
#include "base/signal.h"

#include <cstdint>
#include <string>
#include <vector>

// The processes of one sample in PID order, and the rows shown of them:
// filtered by owner and search text, sorted by a column.
class ProcessList
{
public:
    ProcessList();

    // Takes processes sorted by PID.
    void apply(std::vector<ProcessRecord> processes);
    const ProcessRecord* find(int pid) const;
    const std::vector<ProcessRecord>& processes() const { return m_processes; }

    const std::string& search() const { return m_search; }
    void setSearch(std::string search);
    bool allUsers() const { return m_allUsers; }
    void setAllUsers(bool allUsers);
    const std::string& sortColumn() const { return m_sortColumn; }
    bool ascending() const { return m_ascending; }
    // A column sorts names and PIDs up and the rest down; again, the other way.
    void sortBy(const std::string& column);
    void setSorting(const std::string& column, bool ascending);

    size_t count() const { return m_rows.size(); }
    // Null past the end.
    const ProcessRecord* at(size_t row) const;
    // -1 when the process is not shown.
    int indexOf(int pid) const;

    // A process's own program, as named among an application's processes.
    static std::string programName(const ProcessRecord& process);

    // After the rows or their data changed.
    Lingmo::base::Signal<> changed;
    Lingmo::base::Signal<> sortingChanged;

private:
    bool accepts(const ProcessRecord& process) const;
    bool lessThan(const ProcessRecord& left, const ProcessRecord& right) const;
    void update();

    std::vector<ProcessRecord> m_processes;
    std::vector<size_t> m_rows;
    std::string m_search;
    bool m_allUsers = false;
    std::string m_sortColumn = "cpu";
    bool m_ascending = false;
    uint32_t m_uid;
};
