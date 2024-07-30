/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tabletarea.h"

#include <QStringList>

using namespace Wacom;

TabletArea::TabletArea()
    : QRect()
{
    // nothing to do
}

TabletArea::TabletArea(const QRect &area)
    : QRect()
{
    operator=(area);
}

TabletArea::TabletArea(const QString &area, const QRect &defaultValue)
{
    fromString(area, defaultValue);
}

TabletArea &TabletArea::operator=(const QRect &rect)
{
    if (rect.x() <= 0 && rect.y() <= 0 && rect.width() <= 0 && rect.height() <= 0) {
        setTopLeft(QPoint(0, 0));
        setSize(QSize(0, 0));
    } else {
        setTopLeft(rect.topLeft());
        setSize(rect.size());
    }

    return *this;
}

bool TabletArea::fromString(const QString &area, const QRect &defaultValue)
{
    // set given default value
    *this = defaultValue;

    // expected format "x1 y1 x2 y2"
    QStringList areaValues = area.split(QLatin1String(" "), Qt::SkipEmptyParts);

    if (areaValues.count() != 4) {
        return false;
    }

    // convert to integers
    bool x1Ok, y1Ok, x2Ok, y2Ok;
    int x1 = areaValues.at(0).toInt(&x1Ok);
    int y1 = areaValues.at(1).toInt(&y1Ok);
    int x2 = areaValues.at(2).toInt(&x2Ok);
    int y2 = areaValues.at(3).toInt(&y2Ok);

    if (!x1Ok || !y1Ok || !x2Ok || !y2Ok) {
        return false;
    }

    // tablet areas can not be all negative or zero
    // this is to keep compatibility with older config files.
    if (x1 <= 0 && x2 <= 0 && y1 <= 0 && y2 <= 0) {
        return false;
    }

    // we got something which looks valid
    setTopLeft(QPoint(x1, y1));
    setWidth(x2 - x1);
    setHeight(y2 - y1);

    return true;
}

const QString TabletArea::toString() const
{
    return QString::fromLatin1("%1 %2 %3 %4").arg(x()).arg(y()).arg(x() + width()).arg(y() + height());
}
