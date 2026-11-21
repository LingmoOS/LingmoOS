// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "coredump.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

class CoredumpJournal;
class CrashDialog;

// Shows a dialog when an application the runtime started or a process the
// session restarts dumps core. One dialog per application or process; a
// repeat crash updates the open one.
class CrashManager
{
public:
    explicit CrashManager(std::function<std::string()> fixedFont);
    ~CrashManager();

    CrashManager(const CrashManager &) = delete;
    CrashManager &operator=(const CrashManager &) = delete;

private:
    void onCoredump(const Coredump &dump);
    CrashDialog *dialogFor(const std::string &key, bool application, const std::string &id);

    std::function<std::string()> m_fixedFont;
    std::unique_ptr<CoredumpJournal> m_journal;
    std::map<std::string, std::unique_ptr<CrashDialog>> m_dialogs;
};
