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

#ifndef XINPUTADAPTOR_H
#define XINPUTADAPTOR_H

#include <QList>
#include <QString>

#include "propertyadaptor.h"

namespace Wacom
{

// Forward Declarations
class XinputProperty;
class XinputAdaptorPrivate;

/**
 * A property adaptor which uses XInput to set properties on a device.
 */
class XinputAdaptor : public PropertyAdaptor
{
public:
    //! Default constructor.
    explicit XinputAdaptor(const QString &deviceName);

    //! Destructor
    ~XinputAdaptor() override;

    /**
     * @sa PropertyAdaptor::getProperties()
     */
    const QList<Property> getProperties() const override;

    /**
     * @sa PropertyAdaptor::getProperty(const Property&)
     */
    const QString getProperty(const Property &property) const override;

    /**
     * @sa PropertyAdaptor::setProperty(const Property&, const QString&)
     */
    bool setProperty(const Wacom::Property &property, const QString &value) override;

    /**
     * @sa PropertyAdaptor::supportsProperty(const Property&)
     */
    bool supportsProperty(const Property &property) const override;

private:
    const QString getProperty(const XinputProperty &property) const;

    const QString getFloatProperty(const XinputProperty &property, long nelements = 1) const;

    const QString getLongProperty(const XinputProperty &property, long nelements = 1) const;

    bool mapTabletToScreen(const QString &screenArea) const;

    template<typename T>
    const QString numbersToString(const QList<T> &values) const;

    bool setProperty(const XinputProperty &property, const QString &value) const;

    Q_DECLARE_PRIVATE(XinputAdaptor)
    XinputAdaptorPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
