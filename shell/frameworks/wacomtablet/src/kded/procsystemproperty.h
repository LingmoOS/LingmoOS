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

#ifndef PROCSYSTEMPROPERTY_H
#define PROCSYSTEMPROPERTY_H

#include <QString>

#include "property.h"
#include "propertyset.h"

namespace Wacom
{

// forward declarations & typedefs
class ProcSystemProperty;
typedef PropertySet<ProcSystemProperty> ProcSystemPropertyTemplateSpecialization;

/**
 * Properties supported by the ProcSystemPropertyAdaptor.
 */
class ProcSystemProperty : public ProcSystemPropertyTemplateSpecialization
{
public:
    static const ProcSystemProperty StatusLEDs;
    static const ProcSystemProperty StatusLEDsBrightness;

private:
    ProcSystemProperty(const Property &id, const QString &key)
        : ProcSystemPropertyTemplateSpecialization(this, id, key)
    {
    }

}; // CLASS

// instances container specialization declaration
template<>
ProcSystemPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container
    ProcSystemPropertyTemplateSpecialization::PropertySetTemplateSpecialization::instances;

} // NAMESPACE
#endif // HEADER PROTECTION
