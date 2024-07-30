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

#ifndef DBUSTABLETSERVICE_H
#define DBUSTABLETSERVICE_H

#include "tablethandlerinterface.h"
#include "tabletinformation.h"

#include <QObject>
#include <QString>
#include <QStringList>

namespace Wacom
{
class DBusTabletServicePrivate;

/**
 * @brief The D-Bus tablet service.
 *
 * Handles all D-Bus requests to the tablet daemon and passes them to the
 * tablet handler. All work is done by other modules, this class only
 * exists to separate the d-bus interface from the business logic and to
 * facilitate unit testing.
 */
class DBusTabletService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Wacom")

public:
    explicit DBusTabletService(TabletHandlerInterface &tabletHandler);
    ~DBusTabletService() override;

    // d-bus slots
public Q_SLOTS:
    /**
     * List all connected tablets (by tabletid)
     *
     * @return StringList of the connected tablets with their id
     */
    Q_SCRIPTABLE const QStringList getTabletList() const;

    /**
     * List of the internal device names (pad/stylus/eraser) as used by xsetwacom command
     *
     * @param tabletId the ID of the Tablet to check
     * @return StringList of the connected input devices
     */
    Q_SCRIPTABLE const QStringList getDeviceList(const QString &tabletId) const;

    /**
     * Gets the name of a device (pad/stylus/...).
     *
     * @param tabletId the ID of the Tablet to check
     * @param device A device as returned by DeviceType::key()
     *
     * @return The name of the device.
     */
    Q_SCRIPTABLE QString getDeviceName(const QString &tabletId, const QString &device) const;

    /**
     * Gets information from the tablet.
     *
     * @param tabletId the ID of the Tablet to check
     * @param info The type of information.
     *
     * @return The information value.
     */
    Q_SCRIPTABLE QString getInformation(const QString &tabletId, const QString &info) const;

    /**
     * Returns the current active profile for this tablet.
     *
     * This is not necessary the real configuration in case some other program changed the tablet
     * behaviour. But this is the name of the profile that was used last.
     * Can be used to show in the applet as information or as beginning selection in the kcmodule.
     *
     * @param tabletId the ID of the Tablet to check
     * @return name of the last used profile
     */
    Q_SCRIPTABLE QString getProfile(const QString &tabletId) const;

    /**
     * Returns the current value for a specific tablet device (stylus/eraser/pad/...).
     *
     * @param tabletId the ID of the Tablet to check
     * @param deviceType Type of device (stylus/eraser/...) to get the value from.
     * @param property   The property we are looking for.
     *
     * @return the value as string
     */
    Q_SCRIPTABLE QString getProperty(const QString &tabletId, const QString &deviceType, const QString &property) const;

    /**
     * Tells you if the detected tablet has configurable pushbuttons or not
     *
     * @param tabletId the ID of the Tablet to check
     *
     * @return @c true if pushbuttons are available and thus the conf dialogue can be shown
     *         @c false if nothing is available
     */
    Q_SCRIPTABLE bool hasPadButtons(const QString &tabletId) const;

    /**
     * Checks if a tablet is detected and available for further usage
     *
     * @param tabletId the ID of the Tablet to check
     *
     * @return @c true if tablet is available, @c false otherwise
     */
    Q_SCRIPTABLE bool isAvailable(const QString &tabletId) const;

    /**
     * Returns a list of all available profiles
     *
     * This way around the plasma applet does not check the local KConfig file itself
     * and can be used as a remote applet.
     *
     * @param tabletId the ID of the Tablet to check
     *
     * @return the list of all available profiles
     */
    Q_SCRIPTABLE QStringList listProfiles(const QString &tabletId);

    /**
     * Applies a profile to the tablet device
     *
     * The profile must exist in the tabletprofilerc file and thus created by the kcmodule.
     * Otherwise a notification error is send and shown.
     *
     * @param tabletId the ID of the Tablet to check
     *
     * @param profile name of the profile as specified in the tabletprofilesrc file.
     */
    Q_SCRIPTABLE void setProfile(const QString &tabletId, const QString &profile);

    /**
     * Sets the configuration of @p property from @p deviceType to @p value.
     *
     * @param tabletId the ID of the Tablet to check
     * @param deviceType The device type to set the value on.
     * @param property   The property to set.
     * @param value      The new value of the property.
     */
    Q_SCRIPTABLE void setProperty(const QString &tabletId, const QString &deviceType, const QString &property, const QString &value);

    /**
     * Returns the list for the device profile rotation
     *
     * This list will define which profile will be loaded
     * via the global next/prev profile shortcut
     *
     * @param tabletId the ID of the Tablet to check
     *
     * @return stringlist with profile names in the rotation
     */
    Q_SCRIPTABLE QStringList getProfileRotationList(const QString &tabletId);

    /**
     * Set the Profile rotation list for this Device
     *
     * @param tabletId the ID of the Tablet to check
     * @param rotationList stringlist with profilenames in the right order
     */
    Q_SCRIPTABLE void setProfileRotationList(const QString &tabletId, const QStringList &rotationList);

    /**
     * @brief Get touch sensor device USB ID
     *
     * Some tablets report devices with multiple USB IDs (e.g. Cintiq Companion Hybrid)
     * Primary device (usually the one containing stylus sensor) should have this set
     * to the USB ID of the corresponding secondary device (touch sensor)
     *
     * @param tabletId of the device to check
     * @return tabletId of the touch device
     */
    Q_SCRIPTABLE QString getTouchSensorId(const QString &tabletId);

    /**
     * @brief Reports if this device is a secondary touch sensor
     *
     * Touch sensor devices shouldn't be visible in the KCM
     *
     * @param tabletId of the device to check
     * @return true if it's a touch sensor
     */
    Q_SCRIPTABLE bool isTouchSensor(const QString &tabletId);

    // d-bus signals
Q_SIGNALS:

    /**
     * Emitted if a new tablet is connected and detected
     *
     * This signal is send via DBus to inform other about the recently added device
     */
    Q_SCRIPTABLE void tabletAdded(const QString &tabletId);

    /**
     * Emitted if a known tablet is removed
     *
     * This signal is send via DBus to inform other about the recently removed device
     */
    Q_SCRIPTABLE void tabletRemoved(const QString &tabletId);

    /**
     * Emitted when the profile of the device is changed
     *
     * This signal is send via DBus to inform other about the change
     */
    Q_SCRIPTABLE void profileChanged(const QString &tabletId, const QString &profile);

    // normal Qt slots
public slots:

    //! Has to be called when the current profile was changed.
    void onProfileChanged(const QString &tabletId, const QString &profile);

    //! Has to be called when a new tablet is added.
    void onTabletAdded(const TabletInformation &info);

    //! Has to be called when the current tablet is removed.
    void onTabletRemoved(const QString &tabletId);

private:
    Q_DECLARE_PRIVATE(DBusTabletService)
    DBusTabletServicePrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
