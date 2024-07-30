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

#ifndef TABLETINFO_H
#define TABLETINFO_H

#include <QString>

#include "enum.h"
#include "property.h"

namespace Wacom
{

// forward declaration
class TabletInfo;
struct TabletInfoTemplateSpecializationLessFunctor;

/**
 * @brief Helper Typedef! Do not use!
 *
 * This typedef is required by the TabletInfo class. It eases maintenance of template parameters.
 */
typedef Enum<TabletInfo, QString, TabletInfoTemplateSpecializationLessFunctor, PropertyKeyEqualsFunctor> TabletInfoTemplateSpecialization;

/**
 * @brief Helper Class! Do not use!
 *
 * This functor is required by the TabletInfo class to sort its instances.
 */
struct TabletInfoTemplateSpecializationLessFunctor {
    bool operator()(const TabletInfoTemplateSpecialization *p1, const TabletInfoTemplateSpecialization *p2)
    {
        return (p1->key() < p2->key());
    }
};

/**
 * @brief An enum of all supported device information properties.
 *
 * This enumeration ensures that only correct tablet info identifiers are sent via
 * D-Bus by taking care of converting them to a string and providing a way to map
 * these strings back to a tablet info type.
 * <br/>
 * This class also helps to ease maintenance by passing the responsibility for checking
 * for correct tablet info types to the compiler. When using this enumeration it is no
 * longer possible to have method/d-bus calls fail because of typos in the tablet info
 * identifier. Also refactoring is much easier.
 */
class TabletInfo : public TabletInfoTemplateSpecialization
{
public:
    static const TabletInfo ButtonLayout; //!< The button layout.
    static const TabletInfo CompanyId; //!< The vendor identifier.
    static const TabletInfo CompanyName; //!< The vendor name.
    static const TabletInfo HasLeftTouchStrip; //!< Flag if this tablet has a left touch strip.
    static const TabletInfo HasRightTouchStrip; //!< Flag if this tablet has a right touch strip.
    static const TabletInfo HasTouchRing; //!< Flag if this tablet has a touch ring.
    static const TabletInfo HasWheel; //!< Flag if this tablet has a wheel.
    static const TabletInfo NumPadButtons; //!< Number of pad buttons.
    static const TabletInfo StatusLEDs; //!< Number of LED's that can display tablet modes (for Intuos)
    static const TabletInfo TabletId; //!< The tablet identifier as a four digit hex code.
    static const TabletInfo TabletModel; //!< The tablet model.
    static const TabletInfo TabletName; //!< The name of the tablet.
    static const TabletInfo TabletSerial; //!< The tablet serial id as reported by the wacom driver.
    static const TabletInfo TouchSensorId; // parent device stores touch sensor id here
    static const TabletInfo IsTouchSensor; // slave touch device is marked by this and is hidden from the KCM

private:
    /**
     * Private constructor for the static members of this class.
     */
    TabletInfo(const QString &key)
        : TabletInfoTemplateSpecialization(this, key)
    {
    }

}; // CLASS

/*
 * Declare static instances-container of the TabletInfo template specialization.
 */
template<>
TabletInfoTemplateSpecialization::Container TabletInfoTemplateSpecialization::instances;

} // NAMESPACE
#endif // HEADER PROTECTION
