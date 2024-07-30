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

#ifndef BUTTONPAGEWIDGET_H
#define BUTTONPAGEWIDGET_H

// Qt includes
#include <QWidget>

namespace Ui
{
class ButtonPageWidget;
}

class KComboBox;
class QLabel;

namespace Wacom
{

class ButtonShortcut;
class ProfileManagementInterface;

/**
 * The PadButton widget contains all settings to assign the buttons on the tablet pad
 *
 * The available number of buttons as well as the availability of vertical/horizontal strip
 * or TouchRing is defined by the wacom_devicelist settings file in the data folder.
 */
class ButtonPageWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * default constructor
     *
     * @param parent parent Widget
     */
    explicit ButtonPageWidget(QWidget *parent = nullptr);

    /**
     * default destructor
     */
    ~ButtonPageWidget() override;

    void setTabletId(const QString &tabletId);

    /**
     * Saves all values to the current profile
     */
    void saveToProfile(ProfileManagementInterface &profileManagement);

    /**
     * Reloads the widget when the status of the tablet device changes (connects/disconnects)
     *
     */
    void reloadWidget();

public slots:
    /**
     * Called whenever the profile is switched or the widget needs to be reinitialized.
     *
     * Updates all values on the widget to the values from the profile.
     */
    void loadFromProfile(ProfileManagementInterface &profileManagement);

signals:
    /**
     * Used to inform the main widget that unsaved changes in the current profile are available
     */
    void changed();

private slots:

    /**
     * Called when a button action changed.
     */
    void onButtonActionChanged();

private:
    /**
     * Sets up the UI. Should only be called once by the constructor.
     */
    void setupUi();

private:
    Ui::ButtonPageWidget *ui = nullptr;
    QString _tabletId;
}; // CLASS
} // NAMESPACE
#endif /*BUTTONPAGEWIDGET_H*/
