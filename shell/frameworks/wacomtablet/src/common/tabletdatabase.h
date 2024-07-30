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

#ifndef TABLETDATABASE_H
#define TABLETDATABASE_H

#include "tabletinformation.h"

#include <KSharedConfig>

#include <QMap>
#include <QString>

// forward declarations
class KConfigGroup;

namespace Wacom
{

// Wacom forward declarations
class TabletDatabasePrivate;

class TabletDatabase
{
public:
    virtual ~TabletDatabase();

    /**
     * Returns the only instance of this class.
     *
     * @return Instance of this singleton.
     */
    static TabletDatabase &instance();

    /**
     * Looks up the backend based on the given company id.
     *
     * @param companyId The company ID of the company to lookup the backend for.
     */
    QString lookupBackend(const QString &companyId) const;

    /**
     * Looks up tablet information from database and sets all available information
     * on the given tablet information object.
     *
     * @param tabletId   The tablet identifier to lookup.
     * @param companyId  The company ID of the tablet to lookup.
     * @param tabletInfo The tablet information object to set the data on.
     *
     * @return True if the tablet was found, else false.
     */
    bool lookupTablet(const QString &tabletId, const QString &companyId, Wacom::TabletInformation &tabletInfo) const;

    /**
     * Looks up tablet information from database and sets all available information
     * on the given tablet information object.
     *
     * @param tabletId   The tablet identifier to lookup.
     * @param tabletInfo The tablet information object to set the data on.
     *
     * @return True if the tablet was found, else false.
     */
    bool lookupTablet(const QString &tabletId, Wacom::TabletInformation &tabletInfo) const;

    /**
     * This method is only required for unit testing. Do not use it for normal operations!
     *
     * @param dataDirectory   The full path to the directory containing all data files.
     * @param companyFileName The file name (without path) of the company configuration file.
     */
    void setDatabase(const QString &dataDirectory, const QString &companyFileName);

private:
    /**
     * Private constructor as this class is a singleton.
     */
    TabletDatabase();

    /**
     * Copy constructor which does nothing as this class is a singleton.
     */
    explicit TabletDatabase(const TabletDatabase &that) = delete;

    /**
     * Copy operator which does nothing as this class is a singleton.
     */
    TabletDatabase &operator=(const TabletDatabase &that) = delete;

    /**
     * Reads the button map from the given device group and sets it on the tablet information object.
     *
     * @param deviceGroup The device group to read the data from.
     * @param tabletInfo  The tablet information object to set the button map on.
     *
     * @return True if a button map was found, else false.
     */
    bool getButtonMap(const KConfigGroup &deviceGroup, TabletInformation &tabletInfo) const;

    /**
     * Gets basic information about the given tablet and sets it on the given
     * tablet information object. The parameters tablet id, company id and company
     * name will also be set on the tablet information object.
     *
     * @param deviceGroup The device group to read the data from.
     * @param tabletId    The tablet's identifier.
     * @param companyId   The tablet vendor's id.
     * @param companyName The tablet vendor's name.
     *
     * @return True on success, false on error.
     */
    bool getInformation(const KConfigGroup &deviceGroup,
                        const QString &tabletId,
                        const QString &companyId,
                        const QString &companyName,
                        TabletInformation &tabletInfo) const;

    /**
     * Looks up a tablet configuration group in the given file. If the group exists
     * it will be set on the given tablet group parameter.
     *
     * @param tabletConfigFile The configuration file name (without path) to search in.
     * @param tabletId         The tablet identifier to search.
     * @param tabletGroup      The resulting tablet group if the tablet is found.
     *
     * @return True if the tablet was found, else false.
     */
    bool lookupTabletGroup(const QString &tabletsConfigFile, const QString &tabletId, KConfigGroup &tabletGroup) const;

    /**
     * Opens a database configuration file.
     *
     * @param configFileName The file name (without path) to open.
     * @param configFile     The resulting config file object if the file is found.
     *
     * @return True if the file could be opened, else false.
     */
    bool openConfig(const QString &configFileName, KSharedConfig::Ptr &configFile) const;

    /**
     * Opens the company configuration file.
     *
     * @param configFile The resulting shared config object if the configuration file is found.
     *
     * @return True if the file is found and could be opened, else false.
     */
    bool openCompanyConfig(KSharedConfig::Ptr &configFile) const;

    Q_DECLARE_PRIVATE(TabletDatabase)
    TabletDatabasePrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
