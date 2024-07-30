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

#ifndef TABLETAREA_H
#define TABLETAREA_H

#include <QRect>
#include <QString>

namespace Wacom
{

/**
 * A rectangle which represents a tablet area selection.
 */
class TabletArea : public QRect
{
public:
    /**
     * Creates a new empty tablet area.
     */
    TabletArea();

    /**
     * Creates a new tablet area with the given area selected. If the
     * area string can not be parsed or is invalid, the default value is set.
     *
     * @param area The area string as returned by TabletArea::toString()
     * @param defaultValue A fallback area which is used if the area string is not parseable.
     */
    explicit TabletArea(const QString &area, const QRect &defaultValue = QRect(0, 0, 0, 0));

    /**
     * Creates a new tablet area from the given rectangle.
     * If the rectangle is invalid, (0,0,0,0) is used instead.
     *
     * @param area The area to select.
     */
    explicit TabletArea(const QRect &area);

    /**
     * Copy operators.
     *
     * @param rect The area to copy.
     *
     * @return A reference to this instance.
     */
    TabletArea &operator=(const QRect &rect);

    /**
     * Parses an area string as returned by TabletArea::toString().
     * If the string can not be parsed or the given area is invalid,
     * the default value will be used.
     *
     * @param area The area string to parse.
     * @param defaultValue A fallback value if the given area is not parseable.
     *
     * @return True if the area was successfully parsed, false if the default value was applied.
     */
    bool fromString(const QString &area, const QRect &defaultValue = QRect(0, 0, 0, 0));

    /**
     * Converts the current area to a string. The format is "x1 y1 x2 y2".
     *
     * @return The current area as string.
     */
    const QString toString() const;

}; // CLASS TabletArea
} // NAMESPACE Wacom
#endif // TABLETAREA_H
