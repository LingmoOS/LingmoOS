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

#ifndef TOUCHPAGEWIDGET_H
#define TOUCHPAGEWIDGET_H

#include "screenmap.h"
#include "screenrotation.h"
#include "screenspace.h"

#include <QString>
#include <QWidget>

namespace Ui
{
class TouchPageWidget;
}

namespace Wacom
{

class ProfileManagementInterface;

/**
 * The "Touch" tab of the main KCM widget.
 */
class TouchPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchPageWidget(QWidget *parent = nullptr);
    ~TouchPageWidget() override;

    void setTabletId(const QString &tabletId);

    /**
     * Loads settings from the current profile and updates the widget accordingly.
     */
    void loadFromProfile(Wacom::ProfileManagementInterface &profileManagement);

    /**
     * Reinitializes the widget when a new tablet gets connected.
     */
    void reloadWidget();

    /**
     * Saves the current settings to the current profile.
     */
    void saveToProfile(ProfileManagementInterface &profileManagement);

public slots:

    /**
     * Called when the user enables/disables gesture support.
     */
    void onGesturesModeChanged(int state);

    /**
     * Called when any profile property value changes.
     */
    void onProfileChanged();

    /**
     * Called by our TabletPageWidget to inform us that the user changed the rotation settings.
     */
    void onRotationChanged(const ScreenRotation &rotation);

    /**
     * Called when the user presses the touch tablet mapping button.
     */
    void onTabletMappingClicked();

    /**
     * Called when the user enables/disables touch support.
     */
    void onTouchModeChanged(int state);

    /**
     * Called when the state of the absolute tracking mode changes.
     */
    void onTrackingModeAbsolute(bool activated);

    /**
     * Called when the state of the relative tracking mode changes.
     */
    void onTrackingModeRelative(bool activated);

signals:

    /**
     * Emitted when the user changes configuration settings.
     */
    void changed();

protected:
    /**
     * Gets the current value for the gesture support configuration.
     *
     * @return Either "on" or "off".
     */
    const QString getGestureSupportEnabled() const;

    /**
     * @return The current screen mapping of the touch device.
     */
    const ScreenMap &getScreenMap() const;

    /**
     * Gets the current tablet area mapping in profile format as
     * returned by ScreenMap::toString().
     *
     * @return The current tablet mapping in profile format.
     */
    const QString getScreenMapAsString() const;

    /**
     * @return The current screen space mapping.
     */
    const ScreenSpace &getScreenSpace() const;

    /**
     * Gets the current screen space mapping in profile format.
     *
     * @return The current screen mapping as returned by ScreenSpace::toString().
     */
    const QString getScreenSpaceAsString() const;

    /**
     * Gets the minimum motion before sending a scroll gesture.
     *
     * @return A value >= 0 as string.
     */
    const QString getScrollDistance() const;

    /**
     * Gets the current state of the scroll inversion checkbox as string.
     *
     * @return Either "on" or "off".
     */
    const QString getScrollInversion() const;

    /**
     * Gets the minimum time between taps for a right click.
     *
     * @return A value >= 0 as string.
     */
    const QString getTapTime() const;

    /**
     * Gets the current value of the touch support configuration.
     *
     * @return Either "on" or "off".
     */
    const QString getTouchSupportEnabled() const;

    /**
     * Returns the current tracking mode configuration.
     *
     * @return Either "absolute" or "relative".
     */
    const QString getTrackingMode() const;

    /**
     * Gets the minimum distance for a zoom gesture.
     *
     * @return A value >= 0 as string.
     */
    const QString getZoomDistance() const;

    /**
     * Checks if gesture support is selected.
     *
     * @return True if gesture support is enabled, else false.
     */
    bool isGesturesSupportEnabled() const;

    /**
     * Checks if touch support is selected.
     *
     * @return True if touch support is enabled, else false.
     */
    bool isTouchSupportEnabled() const;

    /**
     * Sets a new value for gesture support and updates all widgets accordingly.
     *
     * @param value Either true or false.
     */
    void setGesturesSupportEnabled(bool value);

    void setScreenMap(const ScreenMap &screenMap);

    /**
     * Sets a new tablet area mapping and updates all widgets accordingly.
     * The given value has to be in profile format as returned by ScreenMap::toString().
     *
     * @param value The new tablet area mapping in profile format.
     */
    void setScreenMap(const QString &value);

    /**
     * Sets a new screen space mapping and updates all widgets accordingly.
     *
     * @param screenSpace The new screen space mapping selection.
     */
    void setScreenSpace(const ScreenSpace &screenSpace);

    /**
     * Sets a new screen space mapping and updates all widgets accordingly.
     * The given value has to be in profile format as returned by ScreenSpace::toString()
     *
     * @param value The new screen area mapping selection.
     */
    void setScreenSpace(const QString &value);

    /**
     * Sets the minimum motion before sending a scroll gesture and updates
     * all widgets accordingly.
     *
     * @param value A value >= 0 as string.
     */
    void setScrollDistance(const QString &value);

    /**
     * Sets the value of the scroll inversion checkbox.
     *
     * @param value Either "on" or "off" or empty string.
     */
    void setScrollInversion(const QString &value);

    /**
     * Sets the minimum time between taps for a right click and updates
     * all widgets accordingly.
     *
     * @param value A value >= 0 as string.
     */
    void setTapTime(const QString &value);

    /**
     * Sets the tracking mode and updates all widgets accordingly.
     *
     * @param value Either "absolute" or "relative"
     */
    void setTrackingMode(const QString &value);

    /**
     * Sets a new value for touch support and updates all widgets accordingly.
     *
     * @param value Either true or false.
     */
    void setTouchSupportEnabled(bool value);

    /**
     * Sets the minimum distance for a zoom gesture and updates all widgets accordingly.
     *
     * @param value A value >= 0 as string.
     */
    void setZoomDistance(const QString &value);

private:
    /**
     * Checks if the current tablet mapping is available for the currently selected
     * tracking mode. If it is not available, a warning is displayed to the user.
     */
    void assertValidTabletMapping();

    /**
     * Initializes this widget. Must only be called once by a constructor.
     */
    void setupUi();

private:
    Ui::TouchPageWidget *ui = nullptr;

    ScreenRotation _tabletRotation = ScreenRotation::NONE; // The currently selected tablet rotation.
    TabletArea _tabletGeometry; // The full touch area as rectangle.
    ScreenMap _screenMap; // The current tablet to screen mapping of the touch device.
    ScreenSpace _screenSpace; // The current screen mapping of the touch device.
    QString _touchDeviceName; // The Xinput name of the touch device of the current tablet.
    QString _tabletId;
}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
