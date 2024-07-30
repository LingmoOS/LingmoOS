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

#ifndef STYLUSPAGEWIDGET_H
#define STYLUSPAGEWIDGET_H

#include <QWidget>

namespace Ui
{
class StylusPageWidget;
}

namespace Wacom
{

class StylusPageWidgetPrivate;

class ButtonShortcut;
class DeviceProfile;
class DeviceType;
class Property;
class ProfileManagementInterface;

/**
 * @brief Provides interface to all settings for the stylus/eraser pen.
 */
class StylusPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StylusPageWidget(QWidget *parent = nullptr);

    ~StylusPageWidget() override;

    void setTabletId(const QString &tabletId);

    /**
     * Called whenever the profile is switched or the widget needs to be reinitialized.
     *
     * Updates all values on the widget to the values from the profile.
     */
    void loadFromProfile(ProfileManagementInterface &profileManagement);

    /**
     * Reloads the widget when the status of the tablet device changes (connects/disconnects)
     */
    void reloadWidget();

    /**
     * Saves all values to the current profile
     */
    void saveToProfile(ProfileManagementInterface &profileManagement);

public slots:
    /**
     * Opens a dialogue that allows the visual selection of the presscurve
     * If Qt detects the tablet (through the xorg.conf file) the changes can be tested
     * directly in the dialogue as well
     */
    void onChangeEraserPressureCurve();

    /**
     * Opens a dialogue that allows the visual selection of the presscurve
     * If Qt detects the tablet (through the xorg.conf file) the changes can be tested
     * directly in the dialogue as well
     */
    void onChangeTipPressureCurve();

    /**
     * Called whenever a value other than the pen buttons is changed.
     * Fires the changed() signal afterwards to inform the main widget that unsaved changes are available
     */
    void onProfileChanged();

signals:
    /**
     * Used to inform the main widget that unsaved changes in the current profile are available
     */
    void changed();

protected:
    const QString getButtonShortcut(const Property &button) const;

    const QString getPressureCurve(const DeviceType &device) const;

    const QString getPressureFeel(const DeviceType &device) const;

    const QString getTabletPcButton() const;

    void setButtonShortcut(const Property &button, const QString &shortcut);

    void setPressureCurve(const DeviceType &device, const QString &value);

    void setPressureFeel(const DeviceType &device, const QString &value);

    void setTabletPcButton(const QString &value);

private:
    void savePropertiesToDeviceProfile(DeviceProfile &profile) const;

    void openPressureCurveDialog(const DeviceType &deviceType);

    /**
     * Sets up the user interface widgets. Should only be called once by a constructor.
     */
    void setupUi();

private:
    QString _tabletId;
    Ui::StylusPageWidget *ui = nullptr;
}; // CLASS
} // NAMESPACE
#endif /*STYLUSPAGEWIDGET_H*/
