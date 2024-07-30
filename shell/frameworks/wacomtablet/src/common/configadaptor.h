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

#ifndef CONFIGADAPTOR_H
#define CONFIGADAPTOR_H

#include "propertyadaptor.h"

#include <KConfigGroup>

namespace Wacom
{

/**
 * The interface for configuration adapters.
 *
 * These adapters support direct reading and writing of KDE config groups as
 * well as property mapping between configuration file keys and config or
 * system properties. In case of a general adapter, an adaptee can be provided
 * where all configuration properties will be read from.
 */
class ConfigAdaptor : public PropertyAdaptor
{
public:
    /**
     * Default constructor.
     *
     * @param adaptee The property adapter where all properties are read from (possibly NULL).
     */
    explicit ConfigAdaptor(PropertyAdaptor *adaptee)
        : PropertyAdaptor(adaptee)
    {
    }

    /**
     * Default destructor
     */
    ~ConfigAdaptor() override
    {
    }

    /**
     * Loads a config from the given config group. The default implementation
     * does nothing and always returns false.
     *
     * @param config The configuration group to load the configuration from.
     *
     * @return True on success, false on error.
     */
    virtual bool loadConfig(const KConfigGroup &config) = 0;

    /**
     * Save the configuration to a config group. The default implementation
     * does nothing and always returns false.
     *
     * @param config The configuration group to save to configuration in.
     *
     * @return True on success, false on error.
     */
    virtual bool saveConfig(KConfigGroup &config) const = 0;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
