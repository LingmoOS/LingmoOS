/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pathhistory.h"
#include <QUrl>

PathHistory::PathHistory(QObject *parent)
    : QObject(parent)
{

}

void PathHistory::append(const QUrl &path)
{
    m_prevHistory.append(path);
}

QUrl PathHistory::first()
{
    return m_prevHistory.first();
}

QUrl PathHistory::last()
{
    return m_prevHistory.last();
}

QUrl PathHistory::at(int i)
{
    return m_prevHistory.at(i);
}

int PathHistory::count()
{
    return m_prevHistory.count();
}

bool PathHistory::isEmpty()
{
    return m_prevHistory.isEmpty();
}

QUrl PathHistory::posteriorPath()
{
    if (m_postHistory.isEmpty())
        return QUrl();

    return m_postHistory.takeLast();
}

QUrl PathHistory::previousPath()
{
    if (m_prevHistory.isEmpty())
        return QUrl();

    if (m_prevHistory.length() < 2)
        return m_prevHistory.at(0);

    m_postHistory.append(m_prevHistory.takeLast());
    return m_prevHistory.takeLast();
}

