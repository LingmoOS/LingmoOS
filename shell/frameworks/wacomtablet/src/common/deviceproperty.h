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

#ifndef DEVICEPROPERTY_H
#define DEVICEPROPERTY_H

#include <QString>

#include "property.h"
#include "propertyset.h"

namespace Wacom
{

// forward declarations & typedefs
class DeviceProperty;
typedef PropertySet<DeviceProperty> DevicePropertyTemplateSpecialization;

/**
 * The property set for configuration files. It defines all properties which
 * can be written to or read from config files.
 */
class DeviceProperty : public DevicePropertyTemplateSpecialization
{
public:
    static const DeviceProperty AbsWheel2Down;
    static const DeviceProperty AbsWheel2Up;
    static const DeviceProperty AbsWheelDown;
    static const DeviceProperty AbsWheelUp;
    static const DeviceProperty Area;
    static const DeviceProperty Button1;
    static const DeviceProperty Button2;
    static const DeviceProperty Button3;
    static const DeviceProperty Button4;
    static const DeviceProperty Button5;
    static const DeviceProperty Button6;
    static const DeviceProperty Button7;
    static const DeviceProperty Button8;
    static const DeviceProperty Button9;
    static const DeviceProperty Button10;
    static const DeviceProperty Button11;
    static const DeviceProperty Button12;
    static const DeviceProperty Button13;
    static const DeviceProperty Button14;
    static const DeviceProperty Button15;
    static const DeviceProperty Button16;
    static const DeviceProperty Button17;
    static const DeviceProperty Button18;
    static const DeviceProperty CursorAccelProfile;
    static const DeviceProperty CursorAccelConstantDeceleration;
    static const DeviceProperty CursorAccelAdaptiveDeceleration;
    static const DeviceProperty CursorAccelVelocityScaling;
    static const DeviceProperty CursorProximity;
    static const DeviceProperty Gesture;
    static const DeviceProperty InvertScroll;
    static const DeviceProperty MapToOutput;
    static const DeviceProperty Mode;
    static const DeviceProperty PressureCurve;
    static const DeviceProperty RawSample;
    static const DeviceProperty RelWheelDown;
    static const DeviceProperty RelWheelUp;
    static const DeviceProperty Rotate;
    static const DeviceProperty ScreenMap;
    static const DeviceProperty ScreenSpace;
    static const DeviceProperty ScrollDistance;
    static const DeviceProperty StripLeftDown;
    static const DeviceProperty StripLeftUp;
    static const DeviceProperty StripRightDown;
    static const DeviceProperty StripRightUp;
    static const DeviceProperty Suppress;
    static const DeviceProperty TabletPcButton;
    static const DeviceProperty TapTime;
    static const DeviceProperty Threshold;
    static const DeviceProperty Touch;
    static const DeviceProperty ZoomDistance;

private:
    DeviceProperty(const Property &id, const QString &key)
        : DevicePropertyTemplateSpecialization(this, id, key)
    {
    }

}; // CLASS

// instances container specialization declaration
template<>
DevicePropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container
    DevicePropertyTemplateSpecialization::PropertySetTemplateSpecialization::instances;

} // NAMESPACE
#endif // HEADER PROTECTION
