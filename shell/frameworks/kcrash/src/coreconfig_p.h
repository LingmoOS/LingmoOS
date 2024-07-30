/*
    SPDX-FileCopyrightText: 2016-2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCRASH_CORECONFIG_H
#define KCRASH_CORECONFIG_H

#include <QString>

namespace KCrash
{
class CoreConfig
{
public:
    CoreConfig(const QString &path = QStringLiteral("/proc/sys/kernel/core_pattern"));

    bool isProcess() const;
    // should this need expansion please refactor to enum. could also store cmdline and compare in kcrash.cpp
    bool isCoredumpd() const;

private:
    bool m_supported = false;
    bool m_process = false;
    bool m_coredumpd = false;
};

} // namespace KCrash

#endif // KCRASH_CORECONFIG_H
