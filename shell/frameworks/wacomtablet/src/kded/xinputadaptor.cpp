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

#include "xinputadaptor.h"

#include "logging.h"
#include "screensinfo.h"
#include "screenspace.h"
#include "stringutils.h"
#include "x11input.h"
#include "x11inputdevice.h"
#include "x11wacom.h"
#include "xinputproperty.h"

#include <QApplication>

using namespace Wacom;

namespace Wacom
{
class XinputAdaptorPrivate
{
public:
    QString deviceName;
    X11InputDevice device;
}; // CLASS
} // NAMESPACE

XinputAdaptor::XinputAdaptor(const QString &deviceName)
    : PropertyAdaptor(nullptr)
    , d_ptr(new XinputAdaptorPrivate)
{
    Q_D(XinputAdaptor);
    d->deviceName = deviceName;
    X11Input::findDevice(deviceName, d->device);
}

XinputAdaptor::~XinputAdaptor()
{
    delete this->d_ptr;
}

const QList<Property> XinputAdaptor::getProperties() const
{
    return XinputProperty::ids();
}

const QString XinputAdaptor::getProperty(const Property &property) const
{
    Q_D(const XinputAdaptor);

    const XinputProperty *xinputproperty = XinputProperty::map(property);

    if (!xinputproperty) {
        qCWarning(KDED) << QString::fromLatin1("Can not get unsupported property '%1' from device '%2' using xinput!").arg(property.key()).arg(d->deviceName);
        return QString();
    }

    if (!d->device.isOpen()) {
        qCWarning(KDED)
            << QString::fromLatin1("Can not get property '%1' from device '%2' because the device is not available!").arg(property.key()).arg(d->deviceName);
        return QString();
    }

    return getProperty(*xinputproperty);
}

bool XinputAdaptor::setProperty(const Property &property, const QString &value)
{
    Q_D(const XinputAdaptor);

    qCDebug(KDED) << QString::fromLatin1("Setting property '%1' to '%2'.").arg(property.key()).arg(value);

    const XinputProperty *xinputproperty = XinputProperty::map(property);

    if (!xinputproperty) {
        qCWarning(KDED) << QString::fromLatin1("Can not set unsupported property '%1' to '%2' on device '%3' using xinput!")
                               .arg(property.key())
                               .arg(value)
                               .arg(d->deviceName);
        return false;
    }

    if (!d->device.isOpen()) {
        qCWarning(KDED) << QString::fromLatin1("Can not set property '%1' to '%2' on device '%3' because the device is not available!")
                               .arg(property.key())
                               .arg(value)
                               .arg(d->deviceName);
        return false;
    }

    return setProperty(*xinputproperty, value);
}

bool XinputAdaptor::supportsProperty(const Property &property) const
{
    return (XinputProperty::map(property) != nullptr);
}

const QString XinputAdaptor::getProperty(const XinputProperty &property) const
{
    Q_D(const XinputAdaptor);

    if (property == XinputProperty::CursorAccelProfile) {
        return getLongProperty(property);

    } else if (property == XinputProperty::CursorAccelAdaptiveDeceleration) {
        return getFloatProperty(property);

    } else if (property == XinputProperty::CursorAccelConstantDeceleration) {
        return getFloatProperty(property);

    } else if (property == XinputProperty::CursorAccelVelocityScaling) {
        return getFloatProperty(property);

    } else if (property == XinputProperty::InvertScroll) {
        return (X11Wacom::isScrollDirectionInverted(d->deviceName) ? QLatin1String("on") : QLatin1String("off"));

    } else {
        qCWarning(KDED) << QString::fromLatin1("Getting Xinput property '%1' is not yet implemented!").arg(property.key());
    }

    return QString();
}

const QString XinputAdaptor::getFloatProperty(const XinputProperty &property, long nelements) const
{
    Q_D(const XinputAdaptor);

    QList<float> values;

    if (!d->device.getFloatProperty(property.key(), values, nelements)) {
        qCWarning(KDED) << QString::fromLatin1("Failed to get Xinput property '%1' from device '%2'!").arg(property.key()).arg(d->deviceName);
        return QString();
    }

    return numbersToString<float>(values);
}

const QString XinputAdaptor::getLongProperty(const XinputProperty &property, long nelements) const
{
    Q_D(const XinputAdaptor);

    QList<long> values;

    if (!d->device.getLongProperty(property.key(), values, nelements)) {
        qCWarning(KDED) << QString::fromLatin1("Failed to get Xinput property '%1' from device '%2'!").arg(property.key()).arg(d->deviceName);
        return QString();
    }

    return numbersToString<long>(values);
}

bool XinputAdaptor::mapTabletToScreen(const QString &screenArea) const
{
    Q_D(const XinputAdaptor);

    // what we need is the Coordinate Transformation Matrix
    // in the normal case where the whole screen is used we end up with a 3x3 identity matrix
    // in our case we want to change that
    // | w  0  offsetX |
    // | 0  h  offsetY |
    // | 0  0     1    |

    qCDebug(KDED) << "Mapping to area: " << screenArea;

    if (screenArea.isEmpty()) {
        return false; // nothing to do
    }

    // get the space the user wants to use to map the tablet
    QRect screenAreaGeometry;
    QRect fullScreenGeometry = ScreensInfo::getUnifiedDisplayGeometry();
    ScreenSpace screenSpace(screenArea);

    switch (screenSpace.getType()) {
    case Wacom::ScreenSpace::ScreenSpaceType::Desktop: {
        qCDebug(KDED) << "Full screen area selected: " << fullScreenGeometry;
        screenAreaGeometry = fullScreenGeometry;

        break;
    }
    case Wacom::ScreenSpace::ScreenSpaceType::Output: {
        auto output = screenSpace.toString();
        QMap<QString, QRect> screenList = ScreensInfo::getScreenGeometries();

        if (!screenList.contains(output)) {
            qCDebug(KDED) << "Selected monitor no longer connected - using full screen: " << fullScreenGeometry;
            screenAreaGeometry = fullScreenGeometry;
        } else {
            qCDebug(KDED) << "Use monitor geometry for screen " << output << ": " << screenList.value(output);
            screenAreaGeometry = screenList.value(output);
        }

        break;
    }
    case Wacom::ScreenSpace::ScreenSpaceType::Area: {
        screenAreaGeometry = screenSpace.getArea();
        qCDebug(KDED) << "Geometry selected: " << screenAreaGeometry;
        break;
    }
    case Wacom::ScreenSpace::ScreenSpaceType::ArbitraryTranslationMatrix: {
        qCDebug(KDED) << "Arbitrary transformation matrix is selected" << screenSpace.getSpeed();

        return X11Wacom::setCoordinateTransformationMatrix(d->deviceName, 0, 0, screenSpace.getSpeed().x(), screenSpace.getSpeed().y());
    }
    }

    // calculate the new transformation matrix
    int screenX = screenAreaGeometry.x();
    int screenY = screenAreaGeometry.y();
    int screenW = screenAreaGeometry.width();
    int screenH = screenAreaGeometry.height();

    qreal w = static_cast<qreal>(screenW) / fullScreenGeometry.width();
    qreal h = static_cast<qreal>(screenH) / fullScreenGeometry.height();

    qreal offsetX = static_cast<qreal>(screenX) / fullScreenGeometry.width();
    qreal offsetY = static_cast<qreal>(screenY) / fullScreenGeometry.height();

    qCDebug(KDED) << "Apply Coordinate Transformation Matrix";
    qCDebug(KDED) << w << "0" << offsetX;
    qCDebug(KDED) << "0" << h << offsetY;
    qCDebug(KDED) << "0"
                  << "0"
                  << "1";

    return X11Wacom::setCoordinateTransformationMatrix(d->deviceName, offsetX, offsetY, w, h);
}

template<typename T>
const QString XinputAdaptor::numbersToString(const QList<T> &values) const
{
    QString result;

    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) {
            result += QLatin1String(" ");
        }

        result += QString::number(values.at(i));
    }

    return result;
}

bool XinputAdaptor::setProperty(const XinputProperty &property, const QString &value) const
{
    Q_D(const XinputAdaptor);

    if (property == XinputProperty::CursorAccelProfile) {
        return d->device.setLongProperty(property.key(), value);

    } else if (property == XinputProperty::CursorAccelAdaptiveDeceleration) {
        return d->device.setFloatProperty(property.key(), value);

    } else if (property == XinputProperty::CursorAccelConstantDeceleration) {
        return d->device.setFloatProperty(property.key(), value);

    } else if (property == XinputProperty::CursorAccelVelocityScaling) {
        return d->device.setFloatProperty(property.key(), value);

    } else if (property == XinputProperty::InvertScroll) {
        return X11Wacom::setScrollDirection(d->deviceName, StringUtils::asBool(value));

    } else if (property == XinputProperty::ScreenSpace) {
        return mapTabletToScreen(value);

    } else {
        qCWarning(KDED) << QString::fromLatin1("Setting Xinput property '%1' is not yet implemented!").arg(property.key());
    }

    return false;
}
