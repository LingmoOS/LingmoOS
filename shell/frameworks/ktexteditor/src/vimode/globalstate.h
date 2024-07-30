/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_GLOBAL_STATE_H
#define KATEVI_GLOBAL_STATE_H

#include <KSharedConfig>
#include <ktexteditor_export.h>

namespace KateVi
{
class History;
class Macros;
class Mappings;
class Registers;

class GlobalState
{
public:
    explicit GlobalState();
    ~GlobalState();
    GlobalState(const GlobalState &) = delete;
    GlobalState &operator=(const GlobalState &) = delete;

    KTEXTEDITOR_EXPORT void writeConfig(KConfig *config) const;
    KTEXTEDITOR_EXPORT void readConfig(const KConfig *config);

    inline Macros *macros() const
    {
        return m_macros;
    }
    inline Mappings *mappings() const
    {
        return m_mappings;
    }
    inline Registers *registers() const
    {
        return m_registers;
    }

    inline History *searchHistory() const
    {
        return m_searchHistory;
    }
    inline History *commandHistory() const
    {
        return m_commandHistory;
    }
    inline History *replaceHistory() const
    {
        return m_replaceHistory;
    }

private:
    static KSharedConfigPtr config();

private:
    Macros *m_macros;
    Mappings *m_mappings;
    Registers *m_registers;

    History *m_searchHistory;
    History *m_commandHistory;
    History *m_replaceHistory;
};
}

#endif
