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

#include "xsetwacomadaptor.h"

#include "buttonshortcut.h"
#include "logging.h"
#include "screenrotation.h"
#include "stringutils.h"
#include "tabletarea.h"
#include "xsetwacomproperty.h"

#include <QProcess>
#include <QRegularExpression>

using namespace Wacom;

namespace Wacom
{
class XsetwacomAdaptorPrivate
{
public:
    QMap<QString, QString> buttonMap;
    QString device;
}; // CLASS
} // NAMESPACE

XsetwacomAdaptor::XsetwacomAdaptor(const QString &deviceName)
    : PropertyAdaptor(nullptr)
    , d_ptr(new XsetwacomAdaptorPrivate)
{
    Q_D(XsetwacomAdaptor);
    d->device = deviceName;
}

XsetwacomAdaptor::XsetwacomAdaptor(const QString &deviceName, const QMap<QString, QString> &buttonMap)
    : PropertyAdaptor(nullptr)
    , d_ptr(new XsetwacomAdaptorPrivate)
{
    Q_D(XsetwacomAdaptor);

    d->buttonMap = buttonMap;
    d->device = deviceName;
}

XsetwacomAdaptor::~XsetwacomAdaptor()
{
    delete this->d_ptr;
}

const QList<Property> XsetwacomAdaptor::getProperties() const
{
    return XsetwacomProperty::ids();
}

const QString XsetwacomAdaptor::getProperty(const Property &property) const
{
    Q_D(const XsetwacomAdaptor);

    const XsetwacomProperty *xsetproperty = XsetwacomProperty::map(property);

    if (!xsetproperty) {
        qCWarning(KDED) << QString::fromLatin1("Can not get unsupported property '%1' using xsetwacom!").arg(property.key());
        return QString();
    }

    // TODO: get invert scroll parameter

    QString convertedParam = convertParameter(*xsetproperty);
    QString xsetwacomValue = getParameter(d->device, convertedParam);

    // convert value to a unified format
    convertFromXsetwacomValue(*xsetproperty, xsetwacomValue);

    qCDebug(KDED) << QString::fromLatin1("Reading property '%1' from device '%2' -> '%3'.").arg(property.key()).arg(d->device).arg(xsetwacomValue);

    return xsetwacomValue;
}

bool XsetwacomAdaptor::setProperty(const Property &property, const QString &value)
{
    Q_D(const XsetwacomAdaptor);

    qCDebug(KDED) << QString::fromLatin1("Setting property '%1' to '%2' on device '%3'.").arg(property.key()).arg(value).arg(d->device);

    const XsetwacomProperty *xsetproperty = XsetwacomProperty::map(property);

    if (!xsetproperty) {
        qCWarning(KDED) << QString::fromLatin1("Can not set unsupported property '%1' to '%2' on device '%3' using xsetwacom!")
                               .arg(property.key())
                               .arg(value)
                               .arg(d->device);
        return false;
    }

    // check for properties which need special handling
    if (property == Property::Area) {
        return setArea(value);

    } else if (property == Property::Rotate) {
        return setRotation(value);

    } else {
        // normal property
        QString convertedParam = convertParameter(*xsetproperty);
        QString convertedValue = value;
        convertToXsetwacomValue(*xsetproperty, convertedValue);

        return setParameter(d->device, convertedParam, convertedValue);
    }

    return false;
}

bool XsetwacomAdaptor::supportsProperty(const Property &property) const
{
    return (XsetwacomProperty::map(property) != nullptr);
}

const QString XsetwacomAdaptor::convertParameter(const XsetwacomProperty &param) const
{
    Q_D(const XsetwacomAdaptor);

    QString modifiedParam = param.key();

    // convert tablet button number to hardware button number
    static const QRegularExpression rx(QLatin1String("^Button\\s*([0-9]+)$"), QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = rx.match(modifiedParam);

    if (match.hasMatch()) {
        QString hwButtonNumber = match.captured(1);

        QString kernelButtonNumber;

        if (!d->buttonMap.isEmpty()) {
            kernelButtonNumber = d->buttonMap.value(hwButtonNumber);
        }

        if (kernelButtonNumber.isEmpty()) {
            kernelButtonNumber = hwButtonNumber;
        }

        qCDebug(KDED) << QString::fromLatin1("Mapping tablet button %1 to X11 button %2.").arg(hwButtonNumber).arg(kernelButtonNumber);

        modifiedParam = QString(QLatin1String("Button %1")).arg(kernelButtonNumber);
    }

    return modifiedParam;
}

void XsetwacomAdaptor::convertButtonShortcut(const XsetwacomProperty &property, QString &value) const
{
    static const QRegularExpression rx(QLatin1String("^Button\\s*[0-9]+$"), QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = rx.match(property.key());

    if (match.hasMatch()) {
        ButtonShortcut buttonshortcut(value);
        value = buttonshortcut.toString();
    }
}

void XsetwacomAdaptor::convertFromXsetwacomValue(const XsetwacomProperty &property, QString &value) const
{
    // convert button shortcuts to a unified format
    convertButtonShortcut(property, value);
}

void XsetwacomAdaptor::convertToXsetwacomValue(const XsetwacomProperty &property, QString &value) const
{
    // convert button shortcuts to a unified format
    convertButtonShortcut(property, value);
}

const QString XsetwacomAdaptor::getParameter(const QString &device, const QString &param) const
{
    QProcess getConf;
    getConf.start(QString::fromLatin1("xsetwacom"), QStringList() << QString::fromLatin1("get") << device << param);
    if (!getConf.waitForStarted() || !getConf.waitForFinished()) {
        return QString();
    }

    QString result = QLatin1String(getConf.readAll());
    return result.remove(QLatin1Char('\n'));
}

bool XsetwacomAdaptor::setArea(const QString &value)
{
    Q_D(const XsetwacomAdaptor);

    TabletArea area(value);

    if (area.isEmpty()) {
        return setParameter(d->device, XsetwacomProperty::ResetArea.key(), QString());
    }

    return setParameter(d->device, XsetwacomProperty::Area.key(), area.toString());
}

bool XsetwacomAdaptor::setRotation(const QString &value)
{
    Q_D(const XsetwacomAdaptor);

    const ScreenRotation *lookup = ScreenRotation::find(value);
    ScreenRotation rotation = lookup ? *lookup : ScreenRotation::NONE;

    // only accept real rotations
    if (rotation == ScreenRotation::NONE || rotation == ScreenRotation::CW || rotation == ScreenRotation::CCW || rotation == ScreenRotation::HALF) {
        setParameter(d->device, XsetwacomProperty::Rotate.key(), rotation.key());
        return true;
    }

    // do not set this value as it is not a real screen rotation
    // probably some auto-mode.
    return false;
}

bool XsetwacomAdaptor::setParameter(const QString &device, const QString &param, const QString &value) const
{
    QProcess setConf;

    // https://bugs.kde.org/show_bug.cgi?id=454947
    static const QRegularExpression buttonWithNumber(QStringLiteral("^Button \\d+$"));
    if (param.contains(buttonWithNumber)) {
        const QStringList splitted = param.split(QLatin1Char(' '));
        setConf.start(QString::fromLatin1("xsetwacom"), QStringList() << QString::fromLatin1("set") << device << splitted[0] << splitted[1] << value);
    } else {
        if (!value.isEmpty()) {
            setConf.start(QString::fromLatin1("xsetwacom"), QStringList() << QString::fromLatin1("set") << device << param << value);
        } else {
            setConf.start(QString::fromLatin1("xsetwacom"), QStringList() << QString::fromLatin1("set") << device << param);
        }
    }

    if (!setConf.waitForStarted() || !setConf.waitForFinished()) {
        return false;
    }

    QByteArray errorOutput = setConf.readAll();

    if (!errorOutput.isEmpty()) {
        qCDebug(KDED) << errorOutput;
        return false;
    }

    return true;
}
