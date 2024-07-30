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

#include "deviceprofiledefaults.h"

#include "screenrotation.h"

using namespace Wacom;

void setupDefaultPad(DeviceProfile &pad)
{
    pad.setProperty(Property::AbsWheelUp, QLatin1String("4"));
    pad.setProperty(Property::AbsWheelDown, QLatin1String("5"));
}

void setupDefaultStylus(DeviceProfile &stylus)
{
    stylus.setProperty(Property::Button1, QLatin1String("1"));
    stylus.setProperty(Property::Button2, QLatin1String("2"));
    stylus.setProperty(Property::Button3, QLatin1String("3"));
    stylus.setProperty(Property::PressureCurve, QLatin1String("0 0 100 100"));
    stylus.setProperty(Property::RawSample, QLatin1String("4"));
    stylus.setProperty(Property::Suppress, QLatin1String("2"));
    stylus.setProperty(Property::Threshold, QLatin1String("27"));
    stylus.setProperty(Property::TabletPcButton, QLatin1String("off"));

    // Mapping defaults
    stylus.setProperty(Property::Mode, QLatin1String("absolute"));
    stylus.setProperty(Property::Rotate, ScreenRotation::AUTO.key());
}

void setupDefaultTouch(DeviceProfile &touch)
{
    touch.setProperty(Property::Gesture, QLatin1String("off"));
    touch.setProperty(Property::InvertScroll, QLatin1String("off"));
    touch.setProperty(Property::Mode, QLatin1String("absolute"));
    touch.setProperty(Property::Rotate, ScreenRotation::AUTO.key());
    touch.setProperty(Property::ScrollDistance, QLatin1String("20"));
    touch.setProperty(Property::TapTime, QLatin1String("250"));
    touch.setProperty(Property::Touch, QLatin1String("on"));
    touch.setProperty(Property::ZoomDistance, QLatin1String("50"));
}
