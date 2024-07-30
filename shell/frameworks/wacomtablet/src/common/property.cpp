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

#include <QString>

#include "property.h"

using namespace Wacom;

/*
 * Instantiate static instances-container of the Property template specialization.
 * This has to be done here obviously before any instance is created.
 */
template<>
PropertyTemplateSpecialization::Container PropertyTemplateSpecialization::instances = PropertyTemplateSpecialization::Container();

/*
 * Instantiate Properties.
 */
const Property Property::AbsWheel2Down(QLatin1String("AbsWheel2Down"));
const Property Property::AbsWheel2Up(QLatin1String("AbsWheel2Up"));
const Property Property::AbsWheelDown(QLatin1String("AbsWheelDown"));
const Property Property::AbsWheelUp(QLatin1String("AbsWheelUp"));
const Property Property::Area(QLatin1String("Area"));
const Property Property::Button1(QLatin1String("Button1"));
const Property Property::Button2(QLatin1String("Button2"));
const Property Property::Button3(QLatin1String("Button3"));
const Property Property::Button4(QLatin1String("Button4"));
const Property Property::Button5(QLatin1String("Button5"));
const Property Property::Button6(QLatin1String("Button6"));
const Property Property::Button7(QLatin1String("Button7"));
const Property Property::Button8(QLatin1String("Button8"));
const Property Property::Button9(QLatin1String("Button9"));
const Property Property::Button10(QLatin1String("Button10"));
const Property Property::Button11(QLatin1String("Button11"));
const Property Property::Button12(QLatin1String("Button12"));
const Property Property::Button13(QLatin1String("Button13"));
const Property Property::Button14(QLatin1String("Button14"));
const Property Property::Button15(QLatin1String("Button15"));
const Property Property::Button16(QLatin1String("Button16"));
const Property Property::Button17(QLatin1String("Button17"));
const Property Property::Button18(QLatin1String("Button18"));
const Property Property::CursorAccelProfile(QLatin1String("CursorAccelProfile"));
const Property Property::CursorAccelConstantDeceleration(QLatin1String("CursorAccelConstantDeceleration"));
const Property Property::CursorAccelAdaptiveDeceleration(QLatin1String("CursorAccelAdaptiveDeceleration"));
const Property Property::CursorAccelVelocityScaling(QLatin1String("CursorAccelVelocityScaling"));
const Property Property::CursorProximity(QLatin1String("CursorProximity"));
const Property Property::Gesture(QLatin1String("Gesture"));
const Property Property::InvertScroll(QLatin1String("InvertScroll"));
const Property Property::MapToOutput(QLatin1String("MapToOutput"));
const Property Property::Mode(QLatin1String("Mode"));
const Property Property::PressureCurve(QLatin1String("PressureCurve"));
const Property Property::RawSample(QLatin1String("RawSample"));
const Property Property::RelWheelDown(QLatin1String("RelWheelDown"));
const Property Property::RelWheelUp(QLatin1String("RelWheelUp"));
const Property Property::ResetArea(QLatin1String("ResetArea"));
const Property Property::Rotate(QLatin1String("Rotate"));
const Property Property::ScreenMap(QLatin1String("ScreenMap"));
const Property Property::ScreenSpace(QLatin1String("ScreenSpace"));
const Property Property::ScrollDistance(QLatin1String("ScrollDistance"));
const Property Property::StatusLEDs(QLatin1String("StatusLEDs"));
const Property Property::StatusLEDsBrightness(QLatin1String("StatusLEDsBrightness"));
const Property Property::StripLeftDown(QLatin1String("StripLeftDown"));
const Property Property::StripLeftUp(QLatin1String("StripLeftUp"));
const Property Property::StripRightDown(QLatin1String("StripRightDown"));
const Property Property::StripRightUp(QLatin1String("StripRightUp"));
const Property Property::Suppress(QLatin1String("Suppress"));
const Property Property::TabletPcButton(QLatin1String("TabletPcButton"));
const Property Property::TapTime(QLatin1String("TapTime"));
const Property Property::Threshold(QLatin1String("Threshold"));
const Property Property::Touch(QLatin1String("Touch"));
const Property Property::ZoomDistance(QLatin1String("ZoomDistance"));
