/*
    SPDX-FileCopyrightText: 2016-2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "coreconfig_p.h"

#include <QFile>

#include <config-kcrash.h>
namespace KCrash
{
CoreConfig::CoreConfig(const QString &path)
{
#if !KCRASH_CORE_PATTERN_RAISE
    return; // Leave everything false unless enabled.
#endif
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    char first = 0;
    if (!file.getChar(&first)) {
        return;
    }
    m_supported = true;
    m_process = first == '|';

    if (file.readLine().contains(QByteArrayLiteral("systemd-coredump"))) {
        m_coredumpd = true;
    }
}

bool CoreConfig::isProcess() const
{
    return m_supported && m_process;
}

bool CoreConfig::isCoredumpd() const
{
    return m_coredumpd;
}

} // namespace KCrash
