/********************************************************************
Copyright 2018 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "globals.h"

#include <QStandardPaths>
#include <QStringBuilder>

namespace Globals
{

QString s_dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) %
                                                     QStringLiteral("/kscreen/");

QString dirPath() {
    return s_dirPath;
}

void setDirPath(const QString &path)
{
    s_dirPath = path;
    if (!s_dirPath.endsWith(QLatin1Char('/'))) {
        s_dirPath += QLatin1Char('/');
    }
}

}
