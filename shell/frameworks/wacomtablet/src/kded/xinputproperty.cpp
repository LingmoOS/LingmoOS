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

#include "xinputproperty.h"

using namespace Wacom;

// instances container specialization instantiation
template<>
XinputPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container
    XinputPropertyTemplateSpecialization::PropertySetTemplateSpecialization::instances =
        XinputPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container();

// KEEP THIS LIST IN AN ORDER WHICH CAN BE SET DIRECTLY!
// SOME PROPERTIES NEED TO BE SET BEFORE OTHERS!
//
const XinputProperty XinputProperty::CursorAccelProfile(Property::CursorAccelProfile, QLatin1String("Device Accel Profile"));
const XinputProperty XinputProperty::CursorAccelConstantDeceleration(Property::CursorAccelConstantDeceleration,
                                                                     QLatin1String("Device Accel Constant Deceleration"));
const XinputProperty XinputProperty::CursorAccelAdaptiveDeceleration(Property::CursorAccelAdaptiveDeceleration,
                                                                     QLatin1String("Device Accel Adaptive Deceleration"));
const XinputProperty XinputProperty::CursorAccelVelocityScaling(Property::CursorAccelVelocityScaling, QLatin1String("Device Accel Velocity Scaling"));
const XinputProperty XinputProperty::InvertScroll(Property::InvertScroll, QLatin1String("Invert Scroll"));
const XinputProperty XinputProperty::ScreenSpace(Property::ScreenSpace, QLatin1String("Coordinate Transformation Matrix"));
