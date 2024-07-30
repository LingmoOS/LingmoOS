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

#include "screenrotation.h"

using namespace Wacom;

/*
 * Instantiate static instances-container of the ScreenRotation template specialization.
 * This has to be done here obviously before any instance is created.
 */
template<>
ScreenRotationTemplateSpecialization::Container ScreenRotationTemplateSpecialization::instances = ScreenRotationTemplateSpecialization::Container();

/*
 * Instantiate Device Types.
 */
const ScreenRotation ScreenRotation::NONE(QLatin1String("none"));
const ScreenRotation ScreenRotation::CCW(QLatin1String("ccw"));
const ScreenRotation ScreenRotation::HALF(QLatin1String("half"));
const ScreenRotation ScreenRotation::CW(QLatin1String("cw"));

const ScreenRotation ScreenRotation::AUTO(QLatin1String("auto"));
const ScreenRotation ScreenRotation::AUTO_INVERTED(QLatin1String("auto-inverted"));

const ScreenRotation &ScreenRotation::invert() const
{
    if (*this == ScreenRotation::CW) {
        return ScreenRotation::CCW;

    } else if (*this == ScreenRotation::CCW) {
        return ScreenRotation::CW;
    }

    return *this;
}
