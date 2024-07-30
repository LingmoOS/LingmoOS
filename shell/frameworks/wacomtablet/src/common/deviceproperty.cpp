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

#include "deviceproperty.h"

using namespace Wacom;

// instances container specialization instantiation
template<>
DevicePropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container
    DevicePropertyTemplateSpecialization::PropertySetTemplateSpecialization::instances =
        DevicePropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container();

// config property instances
const DeviceProperty DeviceProperty::AbsWheel2Down(Property::AbsWheel2Down, QLatin1String("AbsWheel2Down"));
const DeviceProperty DeviceProperty::AbsWheel2Up(Property::AbsWheel2Up, QLatin1String("AbsWheel2Up"));
const DeviceProperty DeviceProperty::AbsWheelDown(Property::AbsWheelDown, QLatin1String("AbsWheelDown"));
const DeviceProperty DeviceProperty::AbsWheelUp(Property::AbsWheelUp, QLatin1String("AbsWheelUp"));
const DeviceProperty DeviceProperty::Area(Property::Area, QLatin1String("Area"));
const DeviceProperty DeviceProperty::Button1(Property::Button1, QLatin1String("Button1"));
const DeviceProperty DeviceProperty::Button2(Property::Button2, QLatin1String("Button2"));
const DeviceProperty DeviceProperty::Button3(Property::Button3, QLatin1String("Button3"));
const DeviceProperty DeviceProperty::Button4(Property::Button4, QLatin1String("Button4"));
const DeviceProperty DeviceProperty::Button5(Property::Button5, QLatin1String("Button5"));
const DeviceProperty DeviceProperty::Button6(Property::Button6, QLatin1String("Button6"));
const DeviceProperty DeviceProperty::Button7(Property::Button7, QLatin1String("Button7"));
const DeviceProperty DeviceProperty::Button8(Property::Button8, QLatin1String("Button8"));
const DeviceProperty DeviceProperty::Button9(Property::Button9, QLatin1String("Button9"));
const DeviceProperty DeviceProperty::Button10(Property::Button10, QLatin1String("Button10"));
const DeviceProperty DeviceProperty::Button11(Property::Button11, QLatin1String("Button11"));
const DeviceProperty DeviceProperty::Button12(Property::Button12, QLatin1String("Button12"));
const DeviceProperty DeviceProperty::Button13(Property::Button13, QLatin1String("Button13"));
const DeviceProperty DeviceProperty::Button14(Property::Button14, QLatin1String("Button14"));
const DeviceProperty DeviceProperty::Button15(Property::Button15, QLatin1String("Button15"));
const DeviceProperty DeviceProperty::Button16(Property::Button16, QLatin1String("Button16"));
const DeviceProperty DeviceProperty::Button17(Property::Button17, QLatin1String("Button17"));
const DeviceProperty DeviceProperty::Button18(Property::Button18, QLatin1String("Button18"));
const DeviceProperty DeviceProperty::CursorAccelProfile(Property::CursorAccelProfile, QLatin1String("CursorAccelProfile"));
const DeviceProperty DeviceProperty::CursorAccelConstantDeceleration(Property::CursorAccelConstantDeceleration,
                                                                     QLatin1String("CursorAccelConstantDeceleration"));
const DeviceProperty DeviceProperty::CursorAccelAdaptiveDeceleration(Property::CursorAccelAdaptiveDeceleration,
                                                                     QLatin1String("CursorAccelAdaptiveDeceleration"));
const DeviceProperty DeviceProperty::CursorAccelVelocityScaling(Property::CursorAccelVelocityScaling, QLatin1String("CursorAccelVelocityScaling"));
const DeviceProperty DeviceProperty::CursorProximity(Property::CursorProximity, QLatin1String("CursorProximity"));
const DeviceProperty DeviceProperty::Gesture(Property::Gesture, QLatin1String("Gesture"));
const DeviceProperty DeviceProperty::InvertScroll(Property::InvertScroll, QLatin1String("InvertScroll"));
const DeviceProperty DeviceProperty::MapToOutput(Property::MapToOutput, QLatin1String("MapToOutput"));
const DeviceProperty DeviceProperty::Mode(Property::Mode, QLatin1String("Mode"));
const DeviceProperty DeviceProperty::PressureCurve(Property::PressureCurve, QLatin1String("PressureCurve"));
const DeviceProperty DeviceProperty::RawSample(Property::RawSample, QLatin1String("RawSample"));
const DeviceProperty DeviceProperty::RelWheelDown(Property::RelWheelDown, QLatin1String("RelWheelDown"));
const DeviceProperty DeviceProperty::RelWheelUp(Property::RelWheelUp, QLatin1String("RelWheelUp"));
const DeviceProperty DeviceProperty::Rotate(Property::Rotate, QLatin1String("Rotate"));
const DeviceProperty DeviceProperty::ScreenMap(Property::ScreenMap, QLatin1String("ScreenMap"));
const DeviceProperty DeviceProperty::ScreenSpace(Property::ScreenSpace, QLatin1String("ScreenSpace"));
const DeviceProperty DeviceProperty::ScrollDistance(Property::ScrollDistance, QLatin1String("ScrollDistance"));
const DeviceProperty DeviceProperty::StripLeftDown(Property::StripLeftDown, QLatin1String("StripLeftDown"));
const DeviceProperty DeviceProperty::StripLeftUp(Property::StripLeftUp, QLatin1String("StripLeftUp"));
const DeviceProperty DeviceProperty::StripRightDown(Property::StripRightDown, QLatin1String("StripRightDown"));
const DeviceProperty DeviceProperty::StripRightUp(Property::StripRightUp, QLatin1String("StripRightUp"));
const DeviceProperty DeviceProperty::Suppress(Property::Suppress, QLatin1String("Suppress"));
const DeviceProperty DeviceProperty::TabletPcButton(Property::TabletPcButton, QLatin1String("TabletPcButton"));
const DeviceProperty DeviceProperty::TapTime(Property::TapTime, QLatin1String("TapTime"));
const DeviceProperty DeviceProperty::Threshold(Property::Threshold, QLatin1String("Threshold"));
const DeviceProperty DeviceProperty::Touch(Property::Touch, QLatin1String("Touch"));
const DeviceProperty DeviceProperty::ZoomDistance(Property::ZoomDistance, QLatin1String("ZoomDistance"));
