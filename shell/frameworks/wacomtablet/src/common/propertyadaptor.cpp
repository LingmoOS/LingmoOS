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

#include "propertyadaptor.h"

#include "logging.h"
#include "stringutils.h"

using namespace Wacom;

namespace Wacom
{
/**
 * Private class for the d-pointer
 */
class PropertyAdaptorPrivate
{
public:
    PropertyAdaptor *adaptee = nullptr;
};
}

PropertyAdaptor::PropertyAdaptor()
    : d_ptr(new PropertyAdaptorPrivate)
{
}

PropertyAdaptor::PropertyAdaptor(PropertyAdaptor *adaptee)
    : d_ptr(new PropertyAdaptorPrivate)
{
    Q_D(PropertyAdaptor);
    d->adaptee = adaptee;
}

PropertyAdaptor::~PropertyAdaptor()
{
    delete this->d_ptr;
}

const QList<Property> PropertyAdaptor::getProperties() const
{
    Q_D(const PropertyAdaptor);

    if (d->adaptee) {
        return d->adaptee->getProperties();
    }

    qCWarning(COMMON) << QString::fromLatin1("Someone is trying to get a list of properties, but no one implemented PropertyAdaptor::getProperties()!");
    return QList<Property>();
}

const QString PropertyAdaptor::getProperty(const Property &property) const
{
    Q_D(const PropertyAdaptor);

    if (d->adaptee) {
        return d->adaptee->getProperty(property);
    }

    qCWarning(COMMON)
        << QString::fromLatin1("Someone is trying to get property '%1', but no one implemented PropertyAdaptor::getProperty()!").arg(property.key());
    return QString();
}

bool PropertyAdaptor::getPropertyAsBool(const Property &property) const
{
    return StringUtils::asBool(getProperty(property));
}

bool PropertyAdaptor::setProperty(const Property &property, const QString &value)
{
    Q_D(PropertyAdaptor);

    if (d->adaptee) {
        return d->adaptee->setProperty(property, value);
    }

    qCWarning(COMMON) << QString::fromLatin1("Someone is trying to set property '%1' to '%2', but no one implemented PropertyAdaptor::setProperty()!")
                             .arg(property.key())
                             .arg(value);
    return false;
}

bool PropertyAdaptor::supportsProperty(const Property &property) const
{
    Q_D(const PropertyAdaptor);

    if (d->adaptee) {
        return d->adaptee->supportsProperty(property);
    }

    QList<Property> props = getProperties();

    foreach (const Property &prop, props) {
        if (property == prop) {
            return true;
        }
    }

    return false;
}

PropertyAdaptor *PropertyAdaptor::getAdaptee()
{
    Q_D(PropertyAdaptor);
    return d->adaptee;
}

const PropertyAdaptor *PropertyAdaptor::getAdaptee() const
{
    Q_D(const PropertyAdaptor);
    return d->adaptee;
}
