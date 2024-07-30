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

#include "procsystemproperty.h"

using namespace Wacom;

// instances container specialization instantiation
template<>
ProcSystemPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container
    ProcSystemPropertyTemplateSpecialization::PropertySetTemplateSpecialization::instances =
        ProcSystemPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container();

// KEEP THIS LIST IN AN ORDER WHICH CAN BE SET DIRECTLY!
// SOME PROPERTIES NEED TO BE SET BEFORE OTHERS!
//
const ProcSystemProperty ProcSystemProperty::StatusLEDs(Property::StatusLEDs, QLatin1String("Status LEDs for Intuos4/5 and Cintiq"));
const ProcSystemProperty ProcSystemProperty::StatusLEDsBrightness(Property::StatusLEDsBrightness,
                                                                  QLatin1String("Status LED Brightness for Intuos4/5 and Cintiq"));
