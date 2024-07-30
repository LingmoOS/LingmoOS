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

#include "deviceprofileconfigadaptor.h"

#include "logging.h"

#include "deviceproperty.h"

#include <QString>
#include <QStringList>
#include <QtGlobal>

using namespace Wacom;

DeviceProfileConfigAdaptor::DeviceProfileConfigAdaptor(DeviceProfile &profile)
    : ConfigAdaptor(&profile)
{
}

DeviceProfileConfigAdaptor::~DeviceProfileConfigAdaptor()
{
}

bool DeviceProfileConfigAdaptor::loadConfig(const KConfigGroup &config)
{
    const DeviceProperty *deviceProperty = nullptr;

    QStringList keys = config.keyList();

    foreach (const QString &key, keys) {
        QString lookupKey = key.toLower();

        // handle old configuration format - this should be removed in the future as
        // it prevents us from using configuration keys which start with 'x' or '0'.
        // "1" is removed for "1TVResolution[01]" parameter
        if (lookupKey.startsWith(QLatin1String("x")) || lookupKey.startsWith(QLatin1String("0")) || lookupKey.startsWith(QLatin1String("1"))) {
            lookupKey.remove(0, 1);
        }

        // underscore is replaced for now to keep compatibility with older configuration files ("Screen_No" parameter)
        lookupKey = lookupKey.replace(QLatin1String("_"), QLatin1String(""));

        deviceProperty = DeviceProperty::find(lookupKey);

        if (deviceProperty == nullptr) {
            qCWarning(COMMON) << QString::fromLatin1("Unable to read unsupported configuration property '%1' from config file!").arg(key);
            continue;
        }

        setProperty(deviceProperty->id(), config.readEntry(key));
    }

    return true;
}

bool DeviceProfileConfigAdaptor::saveConfig(KConfigGroup &config) const
{
    const DeviceProperty *deviceProperty = nullptr;
    QList<Property> properties = getProperties();
    QString value;

    foreach (const Property &property, properties) {
        deviceProperty = DeviceProperty::map(property);

        if (deviceProperty == nullptr) {
            qCWarning(COMMON) << QString::fromLatin1("Unable to save unsupported system property '%1' to config file!").arg(property.key());
            continue;
        }

        value = getProperty(deviceProperty->id());

        if (value.isEmpty()) {
            config.deleteEntry(deviceProperty->key());
        } else {
            config.writeEntry(deviceProperty->key(), value);
        }
    }

    return !properties.isEmpty();
}
