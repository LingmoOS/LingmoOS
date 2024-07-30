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

#ifndef TABLETPAGEWIDGET_H
#define TABLETPAGEWIDGET_H

#include "screenmap.h"
#include "screenrotation.h"
#include "screenspace.h"

#include <QString>
#include <QWidget>

namespace Ui
{
class TabletPageWidget;
}

namespace Wacom
{

class ProfileManagementInterface;

/**
 * The "Tablet" tab of the main KCM widget.
 */
class TabletPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabletPageWidget(QWidget *parent = nullptr);

    ~TabletPageWidget() override;

    void setTabletId(const QString &tabletId);

    /**
     * Loads settings from the current profile and updates the widget accordingly.
     */
    void loadFromProfile(ProfileManagementInterface &profileManagement);

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
     * Called when the user enables/disables the auto-rotation checkbox.
     */
    void onAutoRotateChanged(int state);

    /**
     * Called when any profile property value changes.
     */
    void onProfileChanged();

    /**
     * Called when the user changes the rotation settings.
     */
    void onRotationChanged();

    /**
     * Called when the user presses the pad tablet mapping button.
     */
    void onTabletMappingClicked();

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

    /**
     * Emitted when the rotation changes to inform our other widgets about it.
     */
    void rotationChanged(const ScreenRotation &rotation);

protected:
    /**
     * Gets the current rotation in profile format.
     * These are the values returned by ScreenRotation.key().
     *
     * @return The current rotation in profile format.
     */
    const QString getRotation() const;

    /**
     * @return The current tablet to screen mapping.
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
     * @return The current screen space selection.
     */
    const ScreenSpace &getScreenSpace() const;

    /**
     * Gets the current screen area mapping in profile format.
     *
     * @return A screen mapping as returned by ScreenSpace::toString().
     */
    const QString getScreenSpaceAsString() const;

    /**
     * Gets the current tracking mode selection.
     *
     * @return Either "absolute" or "relative".
     */
    const QString getTrackingMode() const;

    /**
     * Checks if auto-rotation or inverted auto-rotation is enabled.
     *
     * @return True if auto-rotation is enabled, else false.
     */
    bool isAutoRotationEnabled() const;

    /**
     * Check if inverted auto-rotation is enabled.
     *
     * @return True if inverted auto-rotation is active, else false.
     */
    bool isAutoRotateInversionEnabled() const;

    /**
     * Changes the auto-rotation configuration and updates the widgets.
     *
     * @param value True to enable auto-rotation, false to disable it.
     */
    void setAutoRotationEnabled(bool value);

    /**
     * Changes the inverted-auto rotation configuration. This will not
     * update the auto-rotation flag.
     *
     * @param value True to enable inverted auto-rotation, false to disable it.
     */
    void setAutoRotateInversionEnabled(bool value);

    /**
     * Sets the auto-rotation settings based on a string in profile format
     * and updates all widgets accordingly. Valid values for the parameter
     * are all values returned by ScreenRotation::key().
     *
     * @param value The new rotation in profile format.
     */
    void setRotation(const QString &value);

    /**
     * Sets a new tablet area mapping and updates all widgets accordingly.
     *
     * @param screenMap The new tablet to screen mapping.
     */
    void setScreenMap(const ScreenMap &screenMap);

    /**
     * Sets a new tablet area mapping and updates all widgets accordingly.
     * The given value has to be in profile format as returned by ScreenMap::toString().
     *
     * @param value The new tablet area mapping in profile format.
     */
    void setScreenMap(const QString &value);

    /**
     * Sets a new screen area mapping and updates all widgets accordingly.
     *
     * @param screenSpace The new screen space mapping selection.
     */
    void setScreenSpace(const ScreenSpace &screenSpace);

    /**
     * Sets a new screen area mapping and updates all widgets accordingly.
     * The given value has to be in profile format as returned by ScreenSpace::toString()
     *
     * @param value The new screen area mapping selection.
     */
    void setScreenSpace(const QString &value);

    /**
     * Sets the tracking mode and updates all widgets accordingly.
     *
     * @param value Either "absolute" or "relative"
     */
    void setTrackingMode(const QString &value);

private:
    /**
     * Checks if the current tablet mapping is available for the currently selected
     * tracking mode. If it is not available, a warning is displayed to the user.
     */
    void assertValidTabletMapping();

    /**
     * Initializes this widget. Must only be called once by the constructor.
     */
    void setupUi();

private:
    Ui::TabletPageWidget *ui;
    QString _tabletId;
    TabletArea _tabletGeometry; // The full tablet area as rectangle.
    ScreenMap _screenMap; // The current tablet to screen mapping of the pad.
    ScreenSpace _screenSpace; // The current screen mapping of the pad.

    QString _deviceNameStylus; // The Xinput name of the stylus device of the current tablet.
    QString _deviceNameTouch; // The Xinput name of the touch device of the current tablet.

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
