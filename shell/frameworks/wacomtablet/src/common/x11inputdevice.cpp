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

#include "x11inputdevice.h"
#include "logging.h"

#include <QStringList>

#include "private/qtx11extras_p.h"

#include <xcb/xinput.h>

using namespace Wacom;

/**
 * Class for private members.
 */
namespace Wacom
{
class X11InputDevicePrivate
{
public:
    QString name;
    uint8_t deviceid;
};
}

X11InputDevice::X11InputDevice()
    : d_ptr(new X11InputDevicePrivate)
{
    Q_D(X11InputDevice);
    d->deviceid = 0;
}

X11InputDevice::X11InputDevice(X11InputDevice::XID id, const QString &name)
    : d_ptr(new X11InputDevicePrivate)
{
    Q_D(X11InputDevice);
    d->deviceid = 0;

    open(id, name);
}

X11InputDevice::X11InputDevice(const X11InputDevice &device)
    : d_ptr(new X11InputDevicePrivate)
{
    Q_D(X11InputDevice);
    d->deviceid = 0;

    operator=(device);
}

X11InputDevice::~X11InputDevice()
{
    close();
    delete d_ptr;
}

X11InputDevice &X11InputDevice::operator=(const X11InputDevice &that)
{
    // close current device
    close();

    // connect new device
    if (that.d_ptr->deviceid) {
        open(that.d_ptr->deviceid, that.d_ptr->name);
    }

    return *this;
}

bool X11InputDevice::close()
{
    Q_D(X11InputDevice);

    if (d->deviceid == 0) {
        qCWarning(COMMON) << "d->name.isEmpty?" << d->name.isEmpty();
        return false;
    }

    xcb_input_close_device(QX11Info::connection(), d->deviceid);

    d->deviceid = 0;
    d->name.clear();

    return true;
}

bool X11InputDevice::getAtomProperty(const QString &property, QList<long int> &values, long int nelements) const
{
    return getProperty<long>(property, XCB_ATOM_ATOM, nelements, values);
}

const QVector<uint8_t> X11InputDevice::getDeviceButtonMapping() const
{
    Q_D(const X11InputDevice);

    QVector<uint8_t> buttonMap;

    if (!isOpen()) {
        return buttonMap;
    }

    int buttonCount = 0;

    xcb_input_get_device_button_mapping_cookie_t cookie = xcb_input_get_device_button_mapping(QX11Info::connection(), d->deviceid);
    xcb_input_get_device_button_mapping_reply_t *reply = xcb_input_get_device_button_mapping_reply(QX11Info::connection(), cookie, nullptr);

    if (!reply) {
        return buttonMap; // the device has no buttons
    }

    uint8_t *map_return = xcb_input_get_device_button_mapping_map(reply);
    buttonCount = xcb_input_get_device_button_mapping_map_length(reply);

    for (int i = 0; i < buttonCount; ++i) {
        buttonMap.append(map_return[i]);
    }

    free(reply);

    return buttonMap;
}

long X11InputDevice::getDeviceId() const
{
    Q_D(const X11InputDevice);

    if (!isOpen()) {
        return 0;
    }

    return d->deviceid;
}

bool X11InputDevice::getFloatProperty(const QString &property, QList<float> &values, long int nelements) const
{
    if (!isOpen()) {
        return false;
    }

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), false, strlen("FLOAT"), "FLOAT");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(QX11Info::connection(), cookie, nullptr);

    xcb_atom_t expectedType = XCB_ATOM_NONE;

    if (reply) {
        expectedType = reply->atom;
        free(reply);
    }

    if (expectedType == XCB_ATOM_NONE) {
        qCWarning(COMMON) << QLatin1String("Float values are unsupported by this XInput implementation!");
        return false;
    }

    return getProperty<float>(property, expectedType, nelements, values);
}

bool X11InputDevice::getLongProperty(const QString &property, QList<long int> &values, long int nelements) const
{
    return getProperty<long>(property, XCB_ATOM_INTEGER, nelements, values);
}

const QString &X11InputDevice::getName() const
{
    Q_D(const X11InputDevice);
    return d->name;
}

bool X11InputDevice::getStringProperty(const QString &property, QList<QString> &values, long int nelements) const
{
    // get property data & values
    unsigned long nitems = 0;
    int expectedFormat = 8;

    xcb_input_get_device_property_reply_t *reply = getPropertyData(property, XCB_ATOM_STRING, expectedFormat, nelements);
    if (!reply) {
        return false;
    }

    unsigned char *strData = static_cast<unsigned char *>(xcb_input_get_device_property_items(reply));
    nitems = reply->num_items;

    for (unsigned long i = 0; i < nitems; ++i) {
        // add first string value up to '\0'
        QString value = QLatin1String((const char *)strData);
        values.append(value);

        // ++i will jump over '\0'
        i += value.length();
        strData += value.length();
    }

    free(reply);
    return true;
}

bool X11InputDevice::hasProperty(const QString &property) const
{
    Q_D(const X11InputDevice);

    if (!isOpen()) {
        // some devices like the virtual core keyboard/pointer can not be opened
        qCDebug(COMMON) << QString::fromLatin1("Can not check property '%1' on a device which is not open!").arg(property);
        return false;
    }

    Atom atom;
    if (!lookupProperty(property, atom)) {
        return false;
    }

    bool found = false;

    xcb_input_list_device_properties_cookie_t cookie = xcb_input_list_device_properties(QX11Info::connection(), d->deviceid);
    xcb_input_list_device_properties_reply_t *reply = xcb_input_list_device_properties_reply(QX11Info::connection(), cookie, nullptr);

    if (reply) {
        xcb_atom_t *atoms = xcb_input_list_device_properties_atoms(reply);

        for (int i = 0; i < reply->num_atoms; ++i) {
            if (atoms[i] == atom) {
                found = true;
                break;
            }
        }
        free(reply);
    }

    return found;
}

bool X11InputDevice::isOpen() const
{
    Q_D(const X11InputDevice);
    return (d->deviceid != 0);
}

bool X11InputDevice::isTabletDevice()
{
    return hasProperty(QLatin1String("Wacom Tool Type"));
}

bool X11InputDevice::open(X11InputDevice::XID id, const QString &name)
{
    Q_D(X11InputDevice);

    if (isOpen()) {
        close();
    }

    if (id == 0) {
        qCWarning(COMMON) << QString::fromLatin1("Unable to open device '%1' as invalid parameters were provided!").arg(name);
        return false;
    }

    xcb_input_open_device_cookie_t cookie = xcb_input_open_device(QX11Info::connection(), id);
    xcb_input_open_device_reply_t *reply = xcb_input_open_device_reply(QX11Info::connection(), cookie, nullptr);

    if (reply == nullptr) {
        // some virtual devices can not be opened
        qCDebug(COMMON) << QString::fromLatin1("XOpenDevice failed on device id '%1'!").arg(id);
        return false;
    }
    free(reply);

    d->deviceid = id;
    d->name = name;

    return true;
}

bool X11InputDevice::setDeviceButtonMapping(const QVector<uint8_t> &buttonMap) const
{
    Q_D(const X11InputDevice);

    if (!isOpen() || buttonMap.count() == 0) {
        return false;
    }

    xcb_input_set_device_button_mapping_cookie_t cookie =
        xcb_input_set_device_button_mapping(QX11Info::connection(), d->deviceid, static_cast<uint8_t>(buttonMap.size()), buttonMap.data());
    xcb_input_set_device_button_mapping_reply_t *reply = xcb_input_set_device_button_mapping_reply(QX11Info::connection(), cookie, nullptr);

    uint8_t result = 1;

    if (reply) {
        result = reply->status;
        free(reply);
    }

    return (result == 0);
}

bool X11InputDevice::setFloatProperty(const QString &property, const QString &values) const
{
    QStringList valueList = values.split(QLatin1String(" "));

    bool ok;
    QString svalue;
    float fvalue;
    QList<float> fvalues;

    for (int i = 0; i < valueList.size(); ++i) {
        svalue = valueList.at(i);

        if (svalue.isEmpty()) {
            continue;
        }

        fvalue = svalue.toFloat(&ok);

        if (!ok) {
            qCWarning(COMMON) << QString::fromLatin1("Could not convert value '%1' to float!").arg(svalue);
            return false;
        }

        fvalues.append(fvalue);
    }

    return setFloatProperty(property, fvalues);
}

bool X11InputDevice::setFloatProperty(const QString &property, const QList<float> &values) const
{
    if (!isOpen()) {
        return false;
    }

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), false, strlen("FLOAT"), "FLOAT");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(QX11Info::connection(), cookie, nullptr);

    xcb_atom_t expectedType = XCB_ATOM_NONE;

    if (reply) {
        expectedType = reply->atom;
        free(reply);
    }

    if (expectedType == XCB_ATOM_NONE) {
        qCWarning(COMMON) << QLatin1String("Float values are unsupported by this XInput implementation!");
        return false;
    }

    return setProperty<float>(property, expectedType, values);
}

bool X11InputDevice::setLongProperty(const QString &property, const QString &values) const
{
    QStringList valueList = values.split(QLatin1String(" "));

    bool ok;
    QString svalue;
    long lvalue = 0;
    QList<long> lvalues;

    for (int i = 0; i < valueList.size(); ++i) {
        svalue = valueList.at(i);

        if (svalue.isEmpty()) {
            continue;
        }

        lvalue = svalue.toLong(&ok, 10);

        if (!ok) {
            qCWarning(COMMON) << QString::fromLatin1("Could not convert value '%1' to long!").arg(svalue);
            return false;
        }

        lvalues.append(lvalue);
    }

    return setLongProperty(property, lvalues);
}

bool X11InputDevice::setLongProperty(const QString &property, const QList<long int> &values) const
{
    return setProperty<long>(property, XCB_ATOM_INTEGER, values);
}

template<typename T>
bool X11InputDevice::getProperty(const QString &property, X11InputDevice::Atom expectedType, long int nelements, QList<T> &values) const
{
    // get property data & values
    uint32_t *data = nullptr;
    unsigned long nitems = 0;
    int expectedFormat = 32;

    xcb_input_get_device_property_reply_t *reply = getPropertyData(property, expectedType, expectedFormat, nelements);
    if (!reply) {
        return false;
    }
    data = static_cast<uint32_t *>(xcb_input_get_device_property_items(reply));
    nitems = reply->num_items;

    for (unsigned long i = 0; i < nitems; ++i) {
        T replyData = 0;
        memcpy(&replyData, data + i, sizeof(uint32_t));

        values.append(replyData);
    }

    free(reply);

    return true;
}

xcb_input_get_device_property_reply_t *
X11InputDevice::getPropertyData(const QString &property, X11InputDevice::Atom expectedType, int expectedFormat, long int nelements) const
{
    Q_D(const X11InputDevice);

    // check parameters
    if (!isOpen()) {
        qCWarning(COMMON) << QString::fromLatin1("Can not get XInput property '%1' as no device was opened!").arg(property);
        return nullptr;
    }

    if (nelements < 1) {
        qCWarning(COMMON) << QString::fromLatin1("Can not get XInput property '%1' as less than one element was requested!").arg(property);
        return nullptr;
    }

    // lookup property atom
    Atom propertyAtom = XCB_ATOM_NONE;

    if (!lookupProperty(property, propertyAtom)) {
        qCWarning(COMMON) << QString::fromLatin1("Can not get unsupported XInput property '%1'!").arg(property);
        return nullptr;
    }

    // get device property and validate it
    Atom actualType = XCB_ATOM_NONE;
    int actualFormat = 0;

    xcb_input_get_device_property_cookie_t cookie =
        xcb_input_get_device_property(QX11Info::connection(), propertyAtom, XCB_ATOM_ANY, 0, nelements, d->deviceid, false);
    xcb_input_get_device_property_reply_t *reply = xcb_input_get_device_property_reply(QX11Info::connection(), cookie, nullptr);

    if (reply) {
        actualType = reply->type;
        actualFormat = reply->format;
    } else {
        qCWarning(COMMON) << QString::fromLatin1("Could not get XInput property '%1'!").arg(property);
        return nullptr;
    }

    if (actualFormat != expectedFormat || actualType != expectedType) {
        qCWarning(COMMON) << QString::fromLatin1(
                                 "Can not process incompatible Xinput property '%1': Format is '%2', expected was '%3'. Type is '%4', expected was '%5'.")
                                 .arg(property)
                                 .arg(actualFormat)
                                 .arg(expectedFormat)
                                 .arg(actualType)
                                 .arg(expectedType);
        free(reply);
        return nullptr;
    }

    return reply;
}

bool X11InputDevice::lookupProperty(const QString &property, X11InputDevice::Atom &atom) const
{
    if (!isOpen() || property.isEmpty()) {
        return false;
    }

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), false, property.toLatin1().length(), property.toLatin1().constData());
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(QX11Info::connection(), cookie, nullptr);

    if (reply) {
        atom = reply->atom;
        free(reply);
    } else {
        atom = XCB_ATOM_NONE;
    }

    if (atom == XCB_ATOM_NONE) {
        qCWarning(COMMON) << QString::fromLatin1("The X server does not support XInput property '%1'!").arg(property);
        return false;
    }

    return true;
}

template<typename T>
bool X11InputDevice::setProperty(const QString &property, X11InputDevice::Atom expectedType, const QList<T> &values) const
{
    Q_D(const X11InputDevice);

    int expectedFormat = 32; // XInput1 uses 32 bit for each property

    // check parameters
    if (!isOpen()) {
        qCWarning(COMMON) << QString::fromLatin1("Can not set XInput property '%1' as no device was opened!").arg(property);
        return false;
    }

    if (values.size() == 0) {
        qCWarning(COMMON) << QString::fromLatin1("Can not set XInput property '%1' as no values were provided!").arg(property);
        return false;
    }

    // lookup property atom
    Atom propertyAtom = XCB_ATOM_NONE;

    if (!lookupProperty(property, propertyAtom)) {
        qCWarning(COMMON) << QString::fromLatin1("Can not set unsupported XInput property '%1'!").arg(property);
        return false;
    }

    // get property and validate format and type
    Atom actualType;
    int actualFormat;

    xcb_input_get_device_property_cookie_t cookie =
        xcb_input_get_device_property(QX11Info::connection(), propertyAtom, XCB_ATOM_ANY, 0, values.size(), d->deviceid, false);
    xcb_input_get_device_property_reply_t *reply = xcb_input_get_device_property_reply(QX11Info::connection(), cookie, nullptr);

    if (reply) {
        actualType = reply->type;
        actualFormat = reply->format;
        free(reply);
    } else {
        qCWarning(COMMON) << QString::fromLatin1("Could not get XInput property '%1' for type and format validation!").arg(property);
        return false;
    }

    if (actualFormat != expectedFormat || actualType != expectedType) {
        qCWarning(COMMON) << QString::fromLatin1(
                                 "Can not process incompatible Xinput property '%1': Format is '%2', expected was '%3'. Type is '%4', expected was '%5'.")
                                 .arg(property)
                                 .arg(actualFormat)
                                 .arg(expectedFormat)
                                 .arg(actualType)
                                 .arg(expectedType);
        return false;
    }

    // create new data array - for XInput1 the data always to be of type long
    uint32_t *data = new uint32_t[values.size()];

    for (int i = 0; i < values.size(); ++i) {
        const T &value = values.at(i);
        memcpy(data + i, &value, sizeof(uint32_t));
    }

    xcb_input_change_device_property(QX11Info::connection(), propertyAtom, expectedType, d->deviceid, 32, XCB_PROP_MODE_REPLACE, values.size(), data);

    // cleanup
    delete[] data;

    // flush the output buffer to make sure all properties are updated
    xcb_flush(QX11Info::connection());

    return true;
}
