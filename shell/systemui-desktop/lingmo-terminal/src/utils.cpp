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

#include "utils.h"
#include <QApplication>
#include <QClipboard>

static Utils *SELF = nullptr;

Utils *Utils::self()
{
    if (!SELF)
        SELF = new Utils;

    return SELF;
}

Utils::Utils(QObject *parent)
    : QObject(parent)
{

}

QString Utils::text() const
{
    return qApp->clipboard()->text();
}
