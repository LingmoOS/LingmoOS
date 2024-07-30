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

#ifndef PROPERTYADAPTOR_H
#define PROPERTYADAPTOR_H

#include <QList>
#include <QString>

#include "property.h"

namespace Wacom
{

class PropertyAdaptorPrivate;

/**
 * The interface class for property adaptors.
 *
 * These adaptors are used to map properties between different subsystems.
 * They are also used to convert property values between the internal storage
 * format of the adapted object and the global data format.
 */
class PropertyAdaptor
{
public:
    /**
     * Default destructor
     */
    virtual ~PropertyAdaptor();

    /**
     * Gets a list of properties which can be set or get.
     * The default implementation tries to get a list of properties from the
     * adapted object or returns an empty list if no object is set.
     *
     * @return List of available config properties as string.
     */
    virtual const QList<Property> getProperties() const;

    /**
     * Gets a property value. The default implementation tries to get the value
     * from the adapted object and returns it as-is or returns an empty string
     * of no object is set.
     *
     * @param property The property to get.
     *
     * @return The property value.
     */
    virtual const QString getProperty(const Property &property) const;

    /**
     * Gets a property value as boolean. If the conversion to boolean fails,
     * false is returned.
     *
     * @param property The property to get.
     *
     * @return The property value as boolean or false if the value can not be converted.
     */
    virtual bool getPropertyAsBool(const Property &property) const;

    /**
     * Sets a property value. The default implementation passes the value to the
     * adapted object as-is or does nothing if no object is set.
     *
     * @param property The property.
     * @param value    The new value to set.
     *
     * @return True if the value was set, else false.
     */
    virtual bool setProperty(const Wacom::Property &property, const QString &value);

    /**
     * Checks if a property is supported by the managed object. The default
     * implementation tries to query the managed object or parses the output
     * of getProperties() to determine if the given property is supported.
     *
     * @return True if the property is supported, else false.
     */
    virtual bool supportsProperty(const Property &property) const;

protected:
    /**
     * Protected default constructor.
     */
    PropertyAdaptor();

    /**
     * Constructor.
     *
     * @param adaptee The object to adapt (possibly NULL).
     */
    PropertyAdaptor(PropertyAdaptor *adaptee);

    /**
     * Gets the adaptee managed by this instance.
     */
    PropertyAdaptor *getAdaptee();

    /**
     * Gets the adaptee managed by this instance.
     */
    const PropertyAdaptor *getAdaptee() const;

private:
    Q_DECLARE_PRIVATE(PropertyAdaptor)
    PropertyAdaptorPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
