/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#include "fonts.h"

Fonts::Fonts(QObject *parent) : QObject(parent)
{
    init();
}

QStringList Fonts::families() const
{
    return m_families;
}

void Fonts::init()
{
    m_families.clear();

    for (const QString &family : m_fontDatabase.families()) {
        if (m_fontDatabase.isFixedPitch(family)) {
            m_families << family;
        }
    }

    emit familiesChanged();
}
