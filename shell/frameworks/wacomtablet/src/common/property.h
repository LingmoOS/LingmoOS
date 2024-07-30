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

#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>

#include "enum.h"
#include "logging.h"

namespace Wacom
{

// forward declaration
class Property;
struct PropertyTemplateSpecializationLessFunctor;
struct PropertyKeyEqualsFunctor;

/**
 * Do not use this typedef!
 *
 * It is a helper typedef to ease maintenance of template parameters.
 */
typedef Enum<Property, QString, PropertyTemplateSpecializationLessFunctor, PropertyKeyEqualsFunctor> PropertyTemplateSpecialization;

/**
 * Do not use this functor!
 *
 * It is required by the Property class and can only be used to sort instances
 * of type PropertyTemplateSpecialization.
 */
struct PropertyTemplateSpecializationLessFunctor {
    bool operator()(const PropertyTemplateSpecialization *p1, const PropertyTemplateSpecialization *p2)
    {
        return (p1->key() < p2->key());
    }
};

/**
 * A functor which is used to compare two Property keys from type QString.
 * The keys are compared case insensitive. This functor should be used by
 * all property enumerations.
 *
 * @return True if both keys are equal, else false;
 */
struct PropertyKeyEqualsFunctor {
    bool operator()(const QString &k1, const QString &k2)
    {
        return (k1.compare(k2, Qt::CaseInsensitive) == 0);
    }
};

/**
 * An enum of all supported properties.
 *
 * Different subsystems support different sets of properties, which in
 * turn can be mapped to different key values. This class serves as a
 * global list of supported properties to allow property mapping
 * between subsystems.
 */
class Property : public PropertyTemplateSpecialization
{
public:
    /**
     * The action to which absolute wheel up/down should be mapped.
     * See any button property for the list of valid values.
     *
     * @sa Property::Button1
     */
    static const Property AbsWheel2Down;
    static const Property AbsWheel2Up;
    static const Property AbsWheelDown;
    static const Property AbsWheelUp;

    /**
     * The tablet area to which the screen space gets mapped to.
     * Valid values: A string as returned by TabletArea::toString()
     */
    static const Property Area;

    /**
     * The button shortcut action assigned to this button.
     * This property supports quite a lot of values. Use the \a ButtonShortcut class to parse them.
     * Valid values are:
     *
     * - "key [shortcut]"  : A keyboard shortcut. [shortcut] are some key combinations.
     * - "button [number]" : A mouse button shortcut. [number] is a mouse button number.
     * - "[number]"        : A mouse button shortcut. [number] is a mouse button number.
     * - and a few others  : Qt shortcuts and others.
     *
     * @sa ButtonShortcut
     */
    static const Property Button1;
    static const Property Button2;
    static const Property Button3;
    static const Property Button4;
    static const Property Button5;
    static const Property Button6;
    static const Property Button7;
    static const Property Button8;
    static const Property Button9;
    static const Property Button10;
    static const Property Button11;
    static const Property Button12;
    static const Property Button13;
    static const Property Button14;
    static const Property Button15;
    static const Property Button16;
    static const Property Button17;
    static const Property Button18;

    /**
     * Mouse cursor settings, not yet used.
     */
    static const Property CursorAccelProfile;
    static const Property CursorAccelConstantDeceleration;
    static const Property CursorAccelAdaptiveDeceleration;
    static const Property CursorAccelVelocityScaling;

    /**
     * The cursor distance for proximity-out in distance from the tablet.
     * Valid values are numbers > 0.
     *
     * We do not use this property but it is supported by xsetwacom and
     * can be set manually in our profile configuration file.
     */
    static const Property CursorProximity;

    /**
     * Enables/disables gesture support.
     * Valid values are "on" or "off".
     */
    static const Property Gesture;

    /**
     * Invert the scroll direction of the touch device.
     * Does not work atm.
     */
    static const Property InvertScroll;

    /**
     * @deprecated Use Property::ScreenSpace instead.
     *
     * Maps the tablet/touch to a monitor. Accepts any values which are
     * accepted by xsetwacom.
     *
     * We do not use this property but it is here so the user can add an
     * entry to his tablet profile configuration file which will then be set
     * automatically with xsetwacom. However our configuration dialog
     * ignores this property and we might even override it with
     * our \a ScreenSpace property.
     *
     * @sa Property::ScreenSpace
     */
    static const Property MapToOutput;

    /**
     * The stylus/eraser/touch tracking mode.
     * Valid values are:
     *
     * - "absolute" : Absolute tracking mode.
     * - "relative" : Relative tracking mode.
     */
    static const Property Mode;

    /**
     * Bezier curve for pressure.
     * A string consisting of 4 integer values from 0-100.
     */
    static const Property PressureCurve;

    /**
     * Number of raw data used to filter the points.
     * Valid values are numbers > 0.
     */
    static const Property RawSample;

    /**
     * The action to which relative wheel up/down should be mapped.
     * See any button property for the list of valid values.
     *
     * @sa Property::Button1
     */
    static const Property RelWheelDown;
    static const Property RelWheelUp;

    /**
     * Resets the tablet area to the default calibration.
     * This property does not have a value.
     */
    static const Property ResetArea;

    /**
     * Determines if the tablet should get rotated.
     * Valid values are:
     *
     * - "none"          : The tablet does not get rotated.
     * - "cw"            : The tablet is rotated clockwise.
     * - "ccw"           : The tablet is rotated counterclockwise.
     * - "half"          : The tablet is rotated upside down.
     * - "auto"          : The tablet is auto rotated with the screen.
     * - "auto-inverted" : The tablet is auto rotated with the screen and clockwise/counterclockwise rotation is inverted.
     */
    static const Property Rotate;

    /**
     * Maps the screen spaces to the tablet.
     *
     * Format is:
     *
     *  "SP:x1 y1 x2 y2" (SP = ScreenSpace returned by ScreenSpace::toString()).
     *
     * Multiple of these values can be separated by '|'.
     */
    static const Property ScreenMap;

    /**
     * The current screen space which is being used to map the tablet.
     * Valid values are screen space strings returned by ScreenSpace::toString().
     */
    static const Property ScreenSpace;

    /**
     * Minimum motion before sending a scroll gesture.
     * Valid values are numbers >= 0;
     */
    static const Property ScrollDistance;

    /**
     * Intuos4/5 tablets have 4 LED's that are used to show a "current Mode"
     * Cintiq 21UX2 and Cintiq 24HD have 8 of them (2x4)
     *
     * We use it to switch profiles and show which of the profiles is activated.
     * In the TabletDatabase the number of LEDs must be specified.
     * They will be used via the sys filesystem. The leds also support four
     * brightness levels: off, min, mid, high
     */
    static const Property StatusLEDs;
    static const Property StatusLEDsBrightness;

    /**
     * The action to which left/right strip up/down should be mapped.
     * See any button property for the list of valid values.
     *
     * @sa Property::Button1
     */
    static const Property StripLeftDown;
    static const Property StripLeftUp;
    static const Property StripRightDown;
    static const Property StripRightUp;

    /**
     * Number of points trimmed.
     * Valid values are numbers >= 0.
     */
    static const Property Suppress;

    /**
     * Tap to Click. Valid values are "on" or "off".
     * If  on, the stylus must be in contact with the screen for a stylus side button to work.
     * If off, stylus buttons will work once the stylus is in proximity of the tablet.
     */
    static const Property TabletPcButton;

    /**
     * Minimum time between taps for a right click.
     * Valid values are numbers >= 0.
     */
    static const Property TapTime;

    /**
     * Sets tip/eraser pressure threshold.
     * Valid values are numbers >= 0.
     */
    static const Property Threshold;

    /**
     * Enables/disables touch support.
     * Valid values are either "on" or "off".
     */
    static const Property Touch;

    /**
     * Minimum distance for a zoom gesture.
     * Valid values are numbers > 0.
     */
    static const Property ZoomDistance;

private:
    Property(const QString &key)
        : PropertyTemplateSpecialization(this, key)
    {
    }

}; // CLASS

/*
 * Declare static instances-container of the Property template specialization.
 */
template<>
PropertyTemplateSpecialization::Container PropertyTemplateSpecialization::instances;

} // NAMESPACE
#endif // HEADER PROTECTION
