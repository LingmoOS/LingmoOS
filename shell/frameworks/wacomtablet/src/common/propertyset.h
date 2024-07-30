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

#ifndef PROPERTYSET_H
#define PROPERTYSET_H

#include "enum.h"
#include "property.h"

#include <QDebug>
#include <QList>
#include <QString>

namespace Wacom
{

template<class D>
struct PropertySetTemplateSpecializationLessFunctor {
    bool operator()(const D *d1, const D *d2)
    {
        Q_UNUSED(d1);
        Q_UNUSED(d2);

        if ((d1 == d2) || !(d1->key().compare(d2->key(), Qt::CaseInsensitive))) {
            // FIXME: this template header spans across modules
            // which do not share logging categories
            qWarning() << "Adding the same key or the same element is a severe error";
        }

        // property sets should keep the order they were defined
        return false;
    }
};

/**
 * A typesafe property enumeration template class. It manages property keys,
 * mapping ids and priority handling for different kind of property sets.
 *
 * Different subsystems have different type of property sets and different
 * property identifier keys. Moreover properties have to be set in a specific
 * order or they might overwrite each other. Still all these property sets
 * have to be mapped on subsystem borders.
 *
 * NOTICE
 * This class uses template specialization to store a static set of all instances.
 * Therefore a specialization of the private instances member  'm_instances' has
 * to be declared and instantiated before this template can be used.
 */

template<class D, class L = PropertySetTemplateSpecializationLessFunctor<D>, class E = PropertyKeyEqualsFunctor>
class PropertySet : public Enum<D, QString, L, E>
{
public:
    // typedef for easy maintenance
    // this has to be the same as the specialization of our base class
    typedef Enum<D, QString, L, E> PropertySetTemplateSpecialization;

    /**
     * Maps the given property to an instance of this property set.
     *
     * @param property The property to search for.
     *
     * @return A constant pointer to the property or nullptr if the property is not supported by this set.
     */
    static const D *map(const Property &property)
    {
        typename PropertySetTemplateSpecialization::const_iterator i = PropertySetTemplateSpecialization::begin();

        for (; i != PropertySetTemplateSpecialization::end(); ++i) {
            if (i->id() == property) {
                return &(*i);
            }
        }

        return nullptr;
    }

    /**
     * @return The property identifier of this property instance.
     */
    const Property &id() const
    {
        return *m_id;
    }

    /**
     * @return A list of property ids supported by this set.
     */
    static QList<Property> ids()
    {
        QList<Property> ids;
        for (typename PropertySetTemplateSpecialization::const_iterator i = PropertySetTemplateSpecialization::begin();
             i != PropertySetTemplateSpecialization::end();
             ++i) {
            ids.push_back(i->id());
        }
        return ids;
    }

protected:
    /**
     * Initialization constructor.
     * Used to initialize class-static members. Never make this constructor
     * available to the public! It is only used to instantiate class-static
     * PropertySet instances.
     *
     * @param super
     * @param id The property identifier.
     * @param key The key of this class-static PropertySet instance.
     */
    explicit PropertySet(const D *super, const Property &id, const QString &key)
        : PropertySetTemplateSpecialization(super, key)
    {
        m_id = &id;
    }

private:
    const Property *m_id = nullptr; /**< The property identifier used to map between different property sets */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
