/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "history.h"

using namespace KateVi;

namespace
{
const int HISTORY_SIZE_LIMIT = 100;
}

void History::append(const QString &historyItem)
{
    if (historyItem.isEmpty()) {
        return;
    }

    m_items.removeAll(historyItem);

    if (m_items.size() == HISTORY_SIZE_LIMIT) {
        m_items.removeFirst();
    }

    m_items.append(historyItem);
}

void History::clear()
{
    m_items.clear();
}
