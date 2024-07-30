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

#include "xsetwacomproperty.h"

using namespace Wacom;

// instances container specialization instantiation
template<>
XsetwacomPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container
    XsetwacomPropertyTemplateSpecialization::PropertySetTemplateSpecialization::instances =
        XsetwacomPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container();

// ALWAYS MAKE SURE THIS LIST HAS AN ORDER WHICH CAN BE USED DIRECTLY WITH XSETWACOM!
// SOME PROPERTIES NEED TO BE SET BEFORE OTHERS!
//
const XsetwacomProperty XsetwacomProperty::Button1(Property::Button1, QLatin1String("Button 1"));
const XsetwacomProperty XsetwacomProperty::Button2(Property::Button2, QLatin1String("Button 2"));
const XsetwacomProperty XsetwacomProperty::Button3(Property::Button3, QLatin1String("Button 3"));
const XsetwacomProperty XsetwacomProperty::Button4(Property::Button4, QLatin1String("Button 4"));
const XsetwacomProperty XsetwacomProperty::Button5(Property::Button5, QLatin1String("Button 5"));
const XsetwacomProperty XsetwacomProperty::Button6(Property::Button6, QLatin1String("Button 6"));
const XsetwacomProperty XsetwacomProperty::Button7(Property::Button7, QLatin1String("Button 7"));
const XsetwacomProperty XsetwacomProperty::Button8(Property::Button8, QLatin1String("Button 8"));
const XsetwacomProperty XsetwacomProperty::Button9(Property::Button9, QLatin1String("Button 9"));
const XsetwacomProperty XsetwacomProperty::Button10(Property::Button10, QLatin1String("Button 10"));
const XsetwacomProperty XsetwacomProperty::Button11(Property::Button11, QLatin1String("Button 11"));
const XsetwacomProperty XsetwacomProperty::Button12(Property::Button12, QLatin1String("Button 12"));
const XsetwacomProperty XsetwacomProperty::Button13(Property::Button13, QLatin1String("Button 13"));
const XsetwacomProperty XsetwacomProperty::Button14(Property::Button14, QLatin1String("Button 14"));
const XsetwacomProperty XsetwacomProperty::Button15(Property::Button15, QLatin1String("Button 15"));
const XsetwacomProperty XsetwacomProperty::Button16(Property::Button16, QLatin1String("Button 16"));
const XsetwacomProperty XsetwacomProperty::Button17(Property::Button17, QLatin1String("Button 17"));
const XsetwacomProperty XsetwacomProperty::Button18(Property::Button18, QLatin1String("Button 18"));
const XsetwacomProperty XsetwacomProperty::CursorProximity(Property::CursorProximity, QLatin1String("CursorProximity"));
const XsetwacomProperty XsetwacomProperty::Gesture(Property::Gesture, QLatin1String("Gesture"));
const XsetwacomProperty XsetwacomProperty::Mode(Property::Mode, QLatin1String("Mode"));
const XsetwacomProperty XsetwacomProperty::PressureCurve(Property::PressureCurve, QLatin1String("PressureCurve"));
const XsetwacomProperty XsetwacomProperty::RawSample(Property::RawSample, QLatin1String("RawSample"));
const XsetwacomProperty XsetwacomProperty::RelWheelDown(Property::RelWheelDown, QLatin1String("RelWheelDown"));
const XsetwacomProperty XsetwacomProperty::RelWheelUp(Property::RelWheelUp, QLatin1String("RelWheelUp"));
const XsetwacomProperty XsetwacomProperty::Rotate(Property::Rotate, QLatin1String("Rotate"));
const XsetwacomProperty XsetwacomProperty::MapToOutput(Property::MapToOutput, QLatin1String("MapToOutput")); // needs to be applied after rotation
const XsetwacomProperty XsetwacomProperty::Area(Property::Area, QLatin1String("Area")); // needs to be applied after rotation
const XsetwacomProperty XsetwacomProperty::ResetArea(Property::ResetArea, QLatin1String("ResetArea")); // special property which takes precedence over "Area".
const XsetwacomProperty XsetwacomProperty::ScrollDistance(Property::ScrollDistance, QLatin1String("ScrollDistance"));
const XsetwacomProperty XsetwacomProperty::StripLeftDown(Property::StripLeftDown, QLatin1String("StripLeftDown"));
const XsetwacomProperty XsetwacomProperty::StripLeftUp(Property::StripLeftUp, QLatin1String("StripLeftUp"));
const XsetwacomProperty XsetwacomProperty::StripRightDown(Property::StripRightDown, QLatin1String("StripRightDown"));
const XsetwacomProperty XsetwacomProperty::StripRightUp(Property::StripRightUp, QLatin1String("StripRightUp"));
const XsetwacomProperty XsetwacomProperty::Suppress(Property::Suppress, QLatin1String("Suppress"));
const XsetwacomProperty XsetwacomProperty::TabletPcButton(Property::TabletPcButton, QLatin1String("TabletPcButton"));
const XsetwacomProperty XsetwacomProperty::TapTime(Property::TapTime, QLatin1String("TapTime"));
const XsetwacomProperty XsetwacomProperty::Threshold(Property::Threshold, QLatin1String("Threshold"));
const XsetwacomProperty XsetwacomProperty::Touch(Property::Touch, QLatin1String("Touch"));
const XsetwacomProperty XsetwacomProperty::ZoomDistance(Property::ZoomDistance, QLatin1String("ZoomDistance"));
const XsetwacomProperty XsetwacomProperty::AbsWheel2Down(Property::AbsWheel2Down, QLatin1String("AbsWheel2Down"));
const XsetwacomProperty XsetwacomProperty::AbsWheel2Up(Property::AbsWheel2Up, QLatin1String("AbsWheel2Up"));
const XsetwacomProperty XsetwacomProperty::AbsWheelDown(Property::AbsWheelDown, QLatin1String("AbsWheelDown"));
const XsetwacomProperty XsetwacomProperty::AbsWheelUp(Property::AbsWheelUp, QLatin1String("AbsWheelUp"));
