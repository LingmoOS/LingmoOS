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

#ifndef XSETWACOMADAPTOR_H
#define XSETWACOMADAPTOR_H

#include <QMap>
#include <QString>

#include "propertyadaptor.h"

namespace Wacom
{

// Forward Declarations
class XsetwacomProperty;
class XsetwacomAdaptorPrivate;

/**
 * A property adaptor which uses xsetwacom to set properties on a tablet.
 */
class XsetwacomAdaptor : public PropertyAdaptor
{
public:
    //! Default constructor.
    explicit XsetwacomAdaptor(const QString &deviceName);

    XsetwacomAdaptor(const QString &deviceName, const QMap<QString, QString> &buttonMap);

    //! Destructor
    ~XsetwacomAdaptor() override;

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
    /**
     * Does any necessary conversion of the xsetwacom parameter.
     *
     * @param param The parameter to convert.
     *
     * @return The converted parameter.
     */
    const QString convertParameter(const XsetwacomProperty &param) const;

    /**
     * Makes sure a button shortcut is in the correct format. The result of the
     * conversion will be stored in the \a value parameter. If the property is
     * not a button shortcut, the value is left untouched.
     *
     * @param value The shortcut to validate.
     */
    void convertButtonShortcut(const XsetwacomProperty &property, QString &value) const;

    /**
     * Converts a value from xsetwacom format to internal format.
     *
     * @param property The property this value belongs to.
     * @param value    The value of the property. This will also contain the result of the conversion.
     */
    void convertFromXsetwacomValue(const XsetwacomProperty &property, QString &value) const;

    /**
     * Converts a value from internal format to xsetwacom format.
     *
     * @param property The property the value belongs to.
     * @param value    The value of the property. This will also contain the result of the conversion.
     */
    void convertToXsetwacomValue(const XsetwacomProperty &property, QString &value) const;

    /**
     * Gets a parameter using the xsetwacom command line tool. All parameters to this
     * method as well as the device name have to be in a format which is understood
     * by xsetwacom.
     *
     * @param device The device to get the parameter from.
     * @param param  The parameter to get.
     */
    const QString getParameter(const QString &device, const QString &param) const;

    /**
     * Sets the usable tablet area.
     */
    bool setArea(const QString &value);

    /**
     * Sets the tablet rotation.
     */
    bool setRotation(const QString &value);

    /**
     * Sets a parameter using the xsetwacom command line tool. All parameters, values and
     * device names passed to this method have to be in a format which is understood by
     * xsetwacom.
     *
     * @param device The device to set the parameter for.
     * @param param  The parameter to set.
     * @param value  The new value of the parameter.
     */
    bool setParameter(const QString &device, const QString &param, const QString &value) const;

    Q_DECLARE_PRIVATE(XsetwacomAdaptor)
    XsetwacomAdaptorPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
