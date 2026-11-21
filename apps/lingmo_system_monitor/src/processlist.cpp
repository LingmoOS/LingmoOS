// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processlist.h"

#include "fileutil.h"
#include "procreader.h"
#include "textutil.h"

#include "base/i18n.h"
#include "base/locale.h"

#include <algorithm>
#include <unistd.h>

ProcessList::ProcessList()
    : m_uid(getuid())
{
}

void ProcessList::apply(std::vector<ProcessRecord> processes)
{
    m_processes = std::move(processes);
    update();
}

const ProcessRecord* ProcessList::find(int pid) const
{
    const auto position = std::lower_bound(m_processes.cbegin(), m_processes.cend(), pid,
                                           [](const ProcessRecord& process, int value) { return process.pid < value; });
    return position != m_processes.cend() && position->pid == pid ? &*position : nullptr;
}

void ProcessList::setSearch(std::string search)
{
    if (m_search == search)
        return;
    m_search = std::move(search);
    update();
}

void ProcessList::setAllUsers(bool allUsers)
{
    if (m_allUsers == allUsers)
        return;
    m_allUsers = allUsers;
    update();
}

void ProcessList::sortBy(const std::string& column)
{
    setSorting(column, m_sortColumn == column ? !m_ascending : column == "name" || column == "pid");
}

void ProcessList::setSorting(const std::string& column, bool ascending)
{
    if (column != "name" && column != "cpu" && column != "memory" && column != "read" && column != "write"
        && column != "pid")
        return;
    m_ascending = ascending;
    m_sortColumn = column;
    update();
    sortingChanged();
}

const ProcessRecord* ProcessList::at(size_t row) const
{
    return row < m_rows.size() ? &m_processes[m_rows[row]] : nullptr;
}

int ProcessList::indexOf(int pid) const
{
    for (size_t row = 0; row < m_rows.size(); ++row) {
        if (m_processes[m_rows[row]].pid == pid)
            return int(row);
    }
    return -1;
}

std::string ProcessList::programName(const ProcessRecord& process)
{
    // The kernel cuts a process name to 15 bytes; the program or script it
    // runs names it in full. A login shell's "-bash" names nothing.
    std::string executable = process.executable;
    if (executable.ends_with(" (deleted)"))
        executable.resize(executable.size() - 10);
    std::string name = process.name;
    for (const std::string& path : {process.program, executable}) {
        const std::string candidate = fileName(path);
        if (!candidate.empty() && !candidate.starts_with('-')) {
            name = candidate;
            break;
        }
    }

    if (process.helperType.empty())
        return name;
    return Lingmo::i18n::format(Lingmo::i18n::tr("{name} ({helper})"),
                                  {{"name", name}, {"helper", ProcReader::helperName(process.helperType)}});
}

static bool matches(const ProcessRecord& process, const std::string& query)
{
    return textutil::containsIgnoringCase(process.name, query)
           || textutil::containsIgnoringCase(process.displayName, query)
           || textutil::containsIgnoringCase(process.command, query)
           || textutil::containsIgnoringCase(process.user, query)
           || std::to_string(process.pid).find(query) != std::string::npos;
}

bool ProcessList::accepts(const ProcessRecord& process) const
{
    if (!m_allUsers && process.uid != m_uid)
        return false;

    const std::string query = textutil::trimmed(m_search);
    if (query.empty())
        return true;
    return matches(process, query)
           || std::any_of(process.members.cbegin(), process.members.cend(),
                          [&query](const ProcessRecord& member) { return matches(member, query); });
}

bool ProcessList::lessThan(const ProcessRecord& left, const ProcessRecord& right) const
{
    if (m_sortColumn == "name") {
        if (left.displayName == right.displayName)
            return left.pid < right.pid;
        return Lingmo::base::Locale::system().compare(left.displayName, right.displayName) < 0;
    }
    if (m_sortColumn == "pid")
        return left.pid < right.pid;

    const auto value = [this](const ProcessRecord& process) -> double {
        if (m_sortColumn == "cpu")
            return process.cpu;
        if (m_sortColumn == "memory")
            return double(process.memory);
        if (m_sortColumn == "read")
            return process.readRate;
        return process.writeRate;
    };
    const double first = value(left);
    const double second = value(right);
    if (first == second)
        return left.pid < right.pid;
    return first < second;
}

void ProcessList::update()
{
    m_rows.clear();
    for (size_t index = 0; index < m_processes.size(); ++index) {
        if (accepts(m_processes[index]))
            m_rows.push_back(index);
    }
    // Descending order is ascending order turned around, ties included.
    std::stable_sort(m_rows.begin(), m_rows.end(), [this](size_t left, size_t right) {
        return m_ascending ? lessThan(m_processes[left], m_processes[right])
                           : lessThan(m_processes[right], m_processes[left]);
    });
    changed();
}
