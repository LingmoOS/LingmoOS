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

#include "x11wacom.h"

#include "logging.h"
#include "x11input.h"
#include "x11inputdevice.h"

using namespace Wacom;

const TabletArea X11Wacom::getMaximumTabletArea(const QString &deviceName)
{
    TabletArea maximumAreaRect;

    if (deviceName.isEmpty()) {
        qCWarning(COMMON) << QString::fromLatin1("Internal Error: Missing device name parameter!");
        return maximumAreaRect;
    }

    // find the xinput device
    X11InputDevice x11Device;

    if (!X11Input::findDevice(deviceName, x11Device)) {
        qCWarning(COMMON) << QString::fromLatin1("Failed to lookup X11 input device '%1'!").arg(deviceName);
        return maximumAreaRect;
    }

    // get the current property value as backup
    static const QString areaProperty = X11Input::PROPERTY_WACOM_TABLET_AREA;
    QList<long> previousArea;

    if (!x11Device.getLongProperty(areaProperty, previousArea, 4)) {
        qCWarning(COMMON) << QString::fromLatin1("Failed to get tablet area property from X11 input device '%1'!").arg(deviceName);
        return maximumAreaRect;
    }

    // reset the area so it turns back to the maximum.. does not seem to be working
    QList<long> resetArea;
    resetArea.append(-1);
    resetArea.append(-1);
    resetArea.append(-1);
    resetArea.append(-1);

    if (!x11Device.setLongProperty(areaProperty, resetArea)) {
        qCWarning(COMMON) << QString::fromLatin1("Failed to reset tablet area property on X11 input device '%1'!").arg(deviceName);
        return maximumAreaRect;
    }

    // get the maximum area
    QList<long> maximumArea;

    if (x11Device.getLongProperty(areaProperty, maximumArea, 4) && maximumArea.size() == 4) {
        maximumAreaRect.setX(maximumArea.at(0));
        maximumAreaRect.setY(maximumArea.at(1));
        maximumAreaRect.setWidth(maximumArea.at(2) - maximumArea.at(0));
        maximumAreaRect.setHeight(maximumArea.at(3) - maximumArea.at(1));
    }

    // reset the area back to the previous value
    if (!x11Device.setLongProperty(areaProperty, previousArea)) {
        qCWarning(COMMON) << QString::fromLatin1("Failed to set tablet area property on X11 input device '%1'!").arg(deviceName);
    }

    qCDebug(COMMON) << "getMaximumTabletArea result" << maximumAreaRect.toString();

    return maximumAreaRect;
}

bool X11Wacom::isScrollDirectionInverted(const QString &deviceName)
{
    X11InputDevice device;

    if (!X11Input::findDevice(deviceName, device)) {
        return false;
    }

    const auto buttonMap = device.getDeviceButtonMapping();

    if (buttonMap.count() == 0 || buttonMap.count() < 5) {
        return false;
    }

    return (buttonMap.at(3) == 5 && buttonMap.at(4) == 4);
}

bool X11Wacom::setCoordinateTransformationMatrix(const QString &deviceName, qreal offsetX, qreal offsetY, qreal width, qreal height)
{
    X11InputDevice device;

    if (!X11Input::findDevice(deviceName, device)) {
        return false;
    }

    /*
     *  | width  0       offsetX |
     *  | 0      height  offsetY |
     *  | 0      0          1    |
     */
    QList<float> matrix;
    matrix.append(width);
    matrix.append(0);
    matrix.append(offsetX);
    matrix.append(0);
    matrix.append(height);
    matrix.append(offsetY);
    matrix.append(0);
    matrix.append(0);
    matrix.append(1);

    return device.setFloatProperty(X11Input::PROPERTY_TRANSFORM_MATRIX, matrix);
}

bool X11Wacom::setScrollDirection(const QString &deviceName, bool inverted)
{
    X11InputDevice device;

    if (!X11Input::findDevice(deviceName, device)) {
        return false;
    }

    auto buttonMap = device.getDeviceButtonMapping();

    if (buttonMap.count() == 0 || buttonMap.count() < 5) {
        return false;
    }

    if (inverted) {
        buttonMap[3] = 5;
        buttonMap[4] = 4;
    } else {
        buttonMap[3] = 4;
        buttonMap[4] = 5;
    }

    return device.setDeviceButtonMapping(buttonMap);
}
