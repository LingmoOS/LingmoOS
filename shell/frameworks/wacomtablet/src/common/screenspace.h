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

#ifndef SCREENSPACE_H
#define SCREENSPACE_H

#include <QRect>
#include <QString>

namespace Wacom
{

/**
 * @brief Specifies screen area that is used for mapping
 *
 * Stores either "desktop", specific output, arbitrary area
 * or translation matrix (for relative mode).
 * When set as a device property, is converted into translation matrix.
 */

class ScreenSpace
{
public:
    enum class ScreenSpaceType {
        Desktop,
        Output,
        Area,
        ArbitraryTranslationMatrix,
    };

    ScreenSpace();
    ScreenSpace(const QString &screenSpaceString);
    virtual ~ScreenSpace();

    bool operator==(const ScreenSpace &screenSpace) const;

    static const ScreenSpace desktop();

    bool isDesktop() const;

    bool isMonitor() const;

    static const ScreenSpace monitor(QString output);

    static const ScreenSpace matrix(qreal x, qreal y);

    static const ScreenSpace area(QRect area);

    const QString toString() const;

    /**
     * @brief Converts ScreenSpace to corresponding geometry
     *
     * For example, if it's set to output DVI-1, returns geometry of the screen
     * connected to DVI-1
     *
     * Returns empty rectangle on error
     */
    QRect toScreenGeometry() const;

    /**
     * This function allows to cycle through all existing screen spaces in a loop,
     * which means every individual connected screen and "desktop" (whole screen)
     *
     * @return Next ScreenSpace after this
     */
    ScreenSpace next() const;

    ScreenSpaceType getType() const;

    QPointF getSpeed() const;
    QRect getArea() const;

private:
    ScreenSpaceType _type = ScreenSpaceType::Desktop;

    QString _output;
    QRect _area;
    QPointF _speed;
}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
