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

#ifndef SCREENROTATION_H
#define SCREENROTATION_H

#include <QString>

#include "enum.h"
#include "property.h"

namespace Wacom
{

class ScreenRotation;
struct ScreenRotationTemplateSpecializationLessFunctor;

/**
 * @brief Helper Typedef! Do not use!
 *
 * This typedef is required by the ScreenRotation class. It eases maintenance of template parameters.
 */
typedef Enum<ScreenRotation, QString, ScreenRotationTemplateSpecializationLessFunctor, PropertyKeyEqualsFunctor> ScreenRotationTemplateSpecialization;

/**
 * @brief Helper Class! Do not use!
 *
 * This functor is required by the ScreenRotation class to sort its instances.
 */
struct ScreenRotationTemplateSpecializationLessFunctor {
    bool operator()(const ScreenRotationTemplateSpecialization *p1, const ScreenRotationTemplateSpecialization *p2)
    {
        return (p1->key() < p2->key());
    }
};

class ScreenRotation : public ScreenRotationTemplateSpecialization
{
public:
    static const ScreenRotation NONE;
    static const ScreenRotation CCW;
    static const ScreenRotation HALF;
    static const ScreenRotation CW;

    static const ScreenRotation AUTO;
    static const ScreenRotation AUTO_INVERTED;

    const ScreenRotation &invert() const;

private:
    ScreenRotation(const QString &key)
        : ScreenRotationTemplateSpecialization(this, key)
    {
    }

}; // CLASS

/*
 * Declare static instances-container of the template specialization.
 */
template<>
ScreenRotationTemplateSpecialization::Container ScreenRotationTemplateSpecialization::instances;

} // NAMESPACE
#endif // HEADER PROTECTION
