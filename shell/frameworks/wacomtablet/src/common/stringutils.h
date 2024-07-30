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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QString>

class QRect;

namespace Wacom
{
class StringUtils
{
public:
    /**
     * Trims the input value and checks if it is one of '1', 'true', 'on' or 'yes'.
     *
     * @param value The value to return as bool.
     *
     * @return True if the input string is one of "1/true/on/yes", else false.
     */
    static bool asBool(const QString &value);

    /**
     * Converts a QRect to string in the format "x y width height" or "x1 y1 x2 y2".
     *
     * @param rect The given QRect.
     * @param returnCoordinates Return "x1 y1 x2 y2" instead of "x y width height".
     *
     * @return The rectangle as string.
     */
    static const QString fromQRect(const QRect &rect, bool returnCoordinates = false);

    /**
     * Converts a string to a rectangle. If the conversion fails an empty
     * rectangle is returned. The format of the string is expected to be
     * "x y width height". Multiple whitespaces are ignored.
     *
     * @param value The rectangle as string.
     * @param allowOnlyPositiveValues If this flag is set, only values >= 0 are accepted as valid.
     *
     * @return The string as rectangle.
     */
    static const QRect toQRect(const QString &value, bool allowOnlyPositiveValues = false);

    /**
     * Converts a string to a rectangle. If the conversion fails and empty
     * rectangle is returned. The format of the string is expected to be
     * "x1 y1 x2 y2". Multiple whitespaces are ignored.
     *
     * @param value The value as string.
     * @param allowOnlyPositiveValues If this flag is set, only values >= 0 are accepted as valid.
     *
     * @return The string as rectangle.
     */
    static const QRect toQRectByCoordinates(const QString &value, bool allowOnlyPositiveValues = false);

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
