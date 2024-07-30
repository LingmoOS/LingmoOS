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

#ifndef TABLETHANDLER_H
#define TABLETHANDLER_H

#include "property.h"
#include "screenrotation.h"
#include "tablethandlerinterface.h"
#include "tabletinformation.h"

#include <QObject>
#include <QString>
#include <QStringList>

class QScreen;

namespace Wacom
{
class ScreenSpace;
class TabletProfile;
class TabletHandlerPrivate;

class TabletHandler : public TabletHandlerInterface
{
    Q_OBJECT

public:
    TabletHandler();

    /**
     * A constructor used for unit testing.
     *
     * @param profileFile The full path to the profiles configuration file.
     * @param configFile  The full path to the main configuration file.
     */
    TabletHandler(const QString &profileFile, const QString configFile);

    ~TabletHandler() override;

    /**
     * returns the current value for a specific tablet setting
     * This is forwarded to the right backend specified by m_curDevice
     *
     * @param tabletId The id of the tablet where the property should be fetched from
     * @param deviceType name of the tablet device we set. Internal name of the pad/stylus/eraser/cursor
     * @param property the property we are looking for
     *
     * @return the value as string
     */
    QString getProperty(const QString &tabletId, const DeviceType &deviceType, const Property &property) const override;

    /**
     * @brief Reads a list of all available profiles from the profile manager.
     *
     * @param tabletId The id of the tablet which shall be queried
     * @return the list of all available profiles
     */
    QStringList listProfiles(const QString &tabletId) override;

    /**
     * @brief Applies a profile to the tablet device
     *
     * The profile must be known to the profile manager, otherwise a
     * notification error is displayed.
     *
     * @param tabletId The id of the tablet for which the profile shall be set
     * @param profile The name of the profile to apply.
     */
    void setProfile(const QString &tabletId, const QString &profile) override;

    /**
     * Sets the configuration of @p param from @p device with @p value
     * This is forwarded to the right backend specified by m_curDevice
     *
     * @param tabletId The identifier of the device to set the property on.
     * @param deviceType The type of the device to set the property on.
     * @param property The property to set.
     * @param value    New value of the parameter
     */
    void setProperty(const QString &tabletId, const DeviceType &deviceType, const Property &property, const QString &value) override;

    QStringList getProfileRotationList(const QString &tabletId) override;

    void setProfileRotationList(const QString &tabletId, const QStringList &rotationList) override;

public Q_SLOTS:
    /**
     * @brief Handles the connection of a new tablet device.
     *
     * This slot has to be connected to the X device event notifier and
     * executed when a new tablet device is plugged in.
     *
     * @param info The device info as reported by X11.
     */
    void onTabletAdded(const TabletInformation &info);

    /**
     * @brief Handles the removal of a tablet device.
     *
     * This slot has to be connected to the X device event notifier and
     * executed when a tablet is disconnected from the system.
     *
     * @param info The device info as reported by X11.
     */
    void onTabletRemoved(const TabletInformation &info);

    /**
     * @brief Handles rotating the tablet.
     *
     * This slot has to be connected to the X event notifier and executed
     * when the screen is rotated.
     *
     * @param output Name of the screen that has been rotated.
     * @param newScreenRotation The screen rotation.
     */
    void onScreenRotated(QString output, const Qt::ScreenOrientation &newScreenRotation);

    /**
     * @brief Handles screens being connected and disconnected
     * @param screen Screen being connected or disconnected
     */
    void onScreenAddedRemoved(QScreen *screen);

    /**
     * @brief Handles changes in existing screen's geometry
     */
    void onScreenGeometryChanged();

    /**
     * Toggles the stylus/eraser to absolute/relative mode
     */
    void onTogglePenMode();

    /**
     * Toggles the touch tool on/off
     */
    void onToggleTouch();

    /**
     * @brief Toggles to which screen the stylus/eraser/touch will be mapped
     *
     * the order is
     * @li full
     * @li screen1
     * @li screen2
     */
    void onToggleScreenMapping();

    /**
     * @brief Maps stylus/eraser/touch to the full available screen space
     *
     * in xsetwacom file
     *  @li copies AreaMapFull to Area
     *  @li sets ScreenSpace=full
     */
    void onMapToFullScreen();

    /**
     * @brief Maps stylus/eraser/touch to the second screen
     *
     * in xsetwacom file
     *  @li copies AreaMap0 to Area in
     *  @li sets MapToOutput=0
     *  @li sets ScreenSpace=map0
     */
    void onMapToScreen1();

    /**
     * @brief Maps stylus/eraser/touch to the second screen
     *
     * in xsetwacom file
     *  @li copies AreaMap1 to Area in
     *  @li sets MapToOutput=1
     *  @li sets ScreenSpace=map1
     */
    void onMapToScreen2();

    /**
     * @brief Switch to the next profile in the rotation list
     */
    void onNextProfile();

    /**
     * @brief Switch to the previous profile in the rotation list
     */
    void onPreviousProfile();

Q_SIGNALS:
    /**
     * Emitted if the user should be notified.
     */
    void notify(const QString &eventId, const QString &title, const QString &message, bool suggestConfigure);

    /**
     * Emitted when the profile of the current tablet is changed.
     *
     * @param tabletId The identifier of the tablet.
     * @param profile The name of the new active profile.
     */
    void profileChanged(const QString &tabletId, const QString &profile);

    /**
     * Emitted when a new tablet is connected or if the currently active tablet changes.
     */
    void tabletAdded(const TabletInformation &info);

    /**
     * Emitted when the currently active tablet is removed.
     */
    void tabletRemoved(const QString &tabletId);

private:
    /**
     * Auto rotates the tablet if auto-rotation is enabled. If auto-rotation
     * is disabled, the tablet's rotation settings will be left untouched.
     *
     * @param tabletId The id of the Tablet that will be used
     * @param tabletProfile  The tablet profile to read the rotation settings from.
     * @param output Name of the screen that has been rotated. Empty string means scan all devices.
     * @param screenRotation Rotation of the screen specified by screenIndex.
     */
    void autoRotateTablet(const QString &tabletId,
                          const TabletProfile &tabletProfile,
                          QString output = QString(),
                          ScreenRotation screenRotations = ScreenRotation::NONE);

    /**
     * Checks if the current tablet supports the given device type.
     *
     * @param tabletId The id of the Tablet that will be checked
     * @param type The device type to check for.
     *
     * @return True if the tablet supports the given device, else false.
     */
    bool hasDevice(const QString &tabletId, const DeviceType &type) const;

    /**
     * Checks if there currently is a tablet available.
     *
     * @param tabletId The id of the Tablet that will be checked
     * @return True if a tablet is available, else false.
     */
    bool hasTablet(const QString &tabletId) const;

    /**
     * Maps a tablet device to a screen and updated its profile.
     * However the profile will not be saved, only updated with
     * the new settings.
     *
     * @param tabletId The id of the Tablet that will be altered
     * @param device The device to map.
     * @param screepSpace The screen space to map the device to.
     * @param trackingMode The tracking mode to apply.
     * @param tabletProfile The profile to update.
     */
    void mapDeviceToOutput(const QString &tabletId,
                           const Wacom::DeviceType &device,
                           const ScreenSpace &screenSpace,
                           const QString &trackingMode,
                           TabletProfile &tabletProfile);

    /**
     * Maps the stylus, eraser and touch device to the given output and
     * updates the profiles accordingly. If no mapping is configured the
     * device is mapped to the full desktop.
     *
     * @param tabletId The id of the Tablet that will be altered
     * @param output A string returned by ScreenSpace::toString()
     */
    void mapPenToScreenSpace(const QString &tabletId, const ScreenSpace &screenSpace, const QString &trackingMode = QLatin1String("absolute"));

    /**
     * Maps stylus/eraser/touch to their current screen space.
     * The tablet profile will be saved after the operation is complete.
     *
     * @param tabletId The id of the Tablet that will be altered
     * @param tabletProfile The tablet profile to use.
     */
    void mapTabletToCurrentScreenSpace(const QString &tabletId, TabletProfile &tabletProfile);

    Q_DECLARE_PRIVATE(TabletHandler)
    TabletHandlerPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
