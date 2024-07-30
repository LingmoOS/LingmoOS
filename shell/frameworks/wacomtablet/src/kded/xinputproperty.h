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

#ifndef XINPUTPROPERTY_H
#define XINPUTPROPERTY_H

#include <QString>

#include "property.h"
#include "propertyset.h"

namespace Wacom
{

// forward declarations & typedefs
class XinputProperty;
typedef PropertySet<XinputProperty> XinputPropertyTemplateSpecialization;

/**
 * Properties supported by the XInputAdaptor.
 */
class XinputProperty : public XinputPropertyTemplateSpecialization
{
public:
    static const XinputProperty CursorAccelProfile;
    static const XinputProperty CursorAccelConstantDeceleration;
    static const XinputProperty CursorAccelAdaptiveDeceleration;
    static const XinputProperty CursorAccelVelocityScaling;
    static const XinputProperty InvertScroll;
    static const XinputProperty ScreenSpace;

private:
    XinputProperty(const Property &id, const QString &key)
        : XinputPropertyTemplateSpecialization(this, id, key)
    {
    }

}; // CLASS

// instances container specialization declaration
template<>
XinputPropertyTemplateSpecialization::PropertySetTemplateSpecialization::Container
    XinputPropertyTemplateSpecialization::PropertySetTemplateSpecialization::instances;

} // NAMESPACE
#endif // HEADER PROTECTION
