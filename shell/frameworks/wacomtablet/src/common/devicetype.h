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

#ifndef DEVICETYPE_H
#define DEVICETYPE_H

#include <QString>

#include "enum.h"
#include "property.h"

namespace Wacom
{

// forward declaration
class DeviceType;
struct DeviceTypeTemplateSpecializationLessFunctor;

/**
 * @brief Helper Typedef! Do not use!
 *
 * This typedef is required by the DeviceType class. It eases maintenance of template parameters.
 */
typedef Enum<DeviceType, QString, DeviceTypeTemplateSpecializationLessFunctor, PropertyKeyEqualsFunctor> DeviceTypeTemplateSpecialization;

/**
 * @brief Helper Class! Do not use!
 *
 * This functor is required by the DeviceType class to sort its instances.
 */
struct DeviceTypeTemplateSpecializationLessFunctor {
    bool operator()(const DeviceTypeTemplateSpecialization *p1, const DeviceTypeTemplateSpecialization *p2)
    {
        return (p1->key() < p2->key());
    }
};

/**
 * @brief An enum of all device types.
 *
 * Device types are tablet components like the stylus, the eraser or the pad itself.
 * This enumeration ensures that only correct device type identifiers are sent via
 * D-Bus by taking care of converting them to a string and providing a way to map
 * these strings back to a device type.
 * <br/>
 * This class also helps to ease maintenance by passing the responsibility for checking
 * for correct device types to the compiler. When using this enumeration it is no
 * longer possible to have method/d-bus calls fail because of typos in the device type
 * identifier. Also refactoring is much easier.
 */
class DeviceType : public DeviceTypeTemplateSpecialization
{
public:
    //! Compare operator which compares the keys case-insensitive.
    bool operator<(const DeviceType &other) const;

    static const DeviceType Cursor;
    static const DeviceType Eraser;
    static const DeviceType Pad;
    static const DeviceType Stylus;
    static const DeviceType Touch;
    /*
     * TODO: Unknown is an erroneous state and eventually should be removed
     * but currently in use in tests a lot due to uninitialized DeviceProfiles
     */
    static const DeviceType Unknown;

private:
    /**
     * Private constructor to initialize the static instances of this class.
     */
    DeviceType(const QString &key)
        : DeviceTypeTemplateSpecialization(this, key)
    {
    }

}; // CLASS

/*
 * Declare static instances-container of the DeviceType template specialization.
 */
template<>
DeviceTypeTemplateSpecialization::Container DeviceTypeTemplateSpecialization::instances;

} // NAMESPACE
#endif // HEADER PROTECTION
