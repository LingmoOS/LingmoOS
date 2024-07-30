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

#ifndef TABLETPROFILECONFIGADAPTOR_H
#define TABLETPROFILECONFIGADAPTOR_H

#include "configadaptor.h"
#include "tabletprofile.h"

namespace Wacom
{

class TabletProfileConfigAdaptorPrivate;

/**
 * A configuration adapter which can read and write tablet profiles.
 */
class TabletProfileConfigAdaptor : public ConfigAdaptor
{
public:
    /**
     * Default constructor
     */
    explicit TabletProfileConfigAdaptor(TabletProfile &profile);

    /**
     * Default destructor
     */
    ~TabletProfileConfigAdaptor() override;

    /**
     * Loads a tablet profile from the given config group into the tablet
     * profile managed by this adaptor. This will also update the name of
     * the managed profile and clear all devices before reading the new data.
     *
     * The given config group should directly contain all device config
     * groups from that tablet.
     *
     * @param config The config group to read from.
     *
     * @return True on success, false on error.
     */
    bool loadConfig(const KConfigGroup &config) override;

    /**
     * Saves the configuration of the managed tablet profile to the given
     * config group. This will clear out all subgroups of the config group
     * first. The name of the given config group will not be updated.
     *
     * @param config The config group to write to.
     *
     * @return True on success, false on error.
     */
    bool saveConfig(KConfigGroup &config) const override;

private:
    Q_DECLARE_PRIVATE(TabletProfileConfigAdaptor)
    TabletProfileConfigAdaptorPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
