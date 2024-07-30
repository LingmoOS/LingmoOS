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

#include "tabletpagewidget.h"
#include "ui_tabletpagewidget.h"

#include "dbustabletinterface.h"
#include "deviceprofile.h"
#include "profilemanagement.h"
#include "property.h"
#include "screensinfo.h"
#include "stringutils.h"
#include "tabletareaselectiondialog.h"
#include "x11wacom.h"

#include <QStringList>

using namespace Wacom;

TabletPageWidget::TabletPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TabletPageWidget)
{
    setupUi();
}

TabletPageWidget::~TabletPageWidget()
{
    delete ui;
}

void TabletPageWidget::setTabletId(const QString &tabletId)
{
    _tabletId = tabletId;
}

void TabletPageWidget::loadFromProfile(ProfileManagementInterface &profileManagement)
{
    DeviceProfile stylusProfile = profileManagement.loadDeviceProfile(DeviceType::Stylus);

    setRotation(stylusProfile.getProperty(Property::Rotate));
    setScreenSpace(stylusProfile.getProperty(Property::ScreenSpace));
    setScreenMap(stylusProfile.getProperty(Property::ScreenMap));
    setTrackingMode(stylusProfile.getProperty(Property::Mode));
}

void TabletPageWidget::reloadWidget()
{
    // get all tablet device names we need
    QDBusReply<QString> stylusDeviceNameReply = DBusTabletInterface::instance().getDeviceName(_tabletId, DeviceType::Stylus.key());
    QDBusReply<QString> touchDeviceNameReply = DBusTabletInterface::instance().getDeviceName(_tabletId, DeviceType::Touch.key());

    // update name and maximum tablet area for all devices
    _deviceNameStylus.clear();
    _deviceNameTouch.clear();
    _tabletGeometry = TabletArea();
    _screenMap = ScreenMap();

    if (stylusDeviceNameReply.isValid()) {
        _deviceNameStylus = stylusDeviceNameReply.value();
        _tabletGeometry = X11Wacom::getMaximumTabletArea(stylusDeviceNameReply.value());
        _screenMap = ScreenMap(_tabletGeometry);
    }

    if (touchDeviceNameReply.isValid()) {
        _deviceNameTouch = touchDeviceNameReply.value();
    }
}

void TabletPageWidget::saveToProfile(ProfileManagementInterface &profileManagement)
{
    DeviceProfile padProfile = profileManagement.loadDeviceProfile(DeviceType::Pad);
    DeviceProfile stylusProfile = profileManagement.loadDeviceProfile(DeviceType::Stylus);
    DeviceProfile eraserProfile = profileManagement.loadDeviceProfile(DeviceType::Eraser);
    DeviceProfile touchProfile = profileManagement.loadDeviceProfile(DeviceType::Touch);

    stylusProfile.setProperty(Property::Rotate, getRotation());
    eraserProfile.setProperty(Property::Rotate, getRotation());
    touchProfile.setProperty(Property::Rotate, getRotation());
    padProfile.setProperty(Property::Rotate, QString()); // make sure it is not set on the pad - will mess up touch

    stylusProfile.setProperty(Property::ScreenSpace, getScreenSpaceAsString());
    eraserProfile.setProperty(Property::ScreenSpace, getScreenSpaceAsString());
    padProfile.setProperty(Property::ScreenSpace, QString()); // should not be set on the pad, causes trouble
    padProfile.setProperty(Property::Area, QString()); // should not be set on the pad, causes trouble

    stylusProfile.setProperty(Property::ScreenMap, getScreenMapAsString());
    eraserProfile.setProperty(Property::ScreenMap, getScreenMapAsString());
    padProfile.setProperty(Property::ScreenMap, QString()); // make sure it is not set on the pad

    stylusProfile.setProperty(Property::Mode, getTrackingMode());
    eraserProfile.setProperty(Property::Mode, getTrackingMode());

    profileManagement.saveDeviceProfile(padProfile);
    profileManagement.saveDeviceProfile(stylusProfile);
    profileManagement.saveDeviceProfile(eraserProfile);

    if (!_deviceNameTouch.isEmpty()) {
        profileManagement.saveDeviceProfile(touchProfile);
    }
}

void TabletPageWidget::onAutoRotateChanged(int state)
{
    setAutoRotationEnabled(state == Qt::Checked);
    onProfileChanged();
}

void TabletPageWidget::onProfileChanged()
{
    emit changed();
}

void TabletPageWidget::onTabletMappingClicked()
{
    // get current rotation settings
    // we need to invert it as our rotation settings in this widget have a canvas viewpoint
    const ScreenRotation *lookupRotation = ScreenRotation::find(getRotation());
    ScreenRotation rotation = lookupRotation ? lookupRotation->invert() : ScreenRotation::NONE;

    TabletAreaSelectionDialog selectionDialog;
    selectionDialog.setupWidget(getScreenMap(), _deviceNameStylus, rotation);
    selectionDialog.select(getScreenSpace());

    if (selectionDialog.exec() == QDialog::Accepted) {
        setScreenMap(selectionDialog.getScreenMap());
        setScreenSpace(selectionDialog.getScreenSpace());
        onProfileChanged();
    }
}

void TabletPageWidget::onRotationChanged()
{
    // determine rotation
    const ScreenRotation *lookupRotation = ScreenRotation::find(getRotation());

    // we need to invert it as our rotation settings in this widget have a canvas viewpoint
    // and our private member variable has a tablet viewpoint
    emit rotationChanged(lookupRotation ? lookupRotation->invert() : ScreenRotation::NONE);
}

void TabletPageWidget::onTrackingModeAbsolute(bool activated)
{
    if (!activated) {
        return;
    }

    setTrackingMode(QLatin1String("absolute"));
    onProfileChanged();
}

void TabletPageWidget::onTrackingModeRelative(bool activated)
{
    if (!activated) {
        return;
    }

    setTrackingMode(QLatin1String("relative"));
    onProfileChanged();
}

const QString TabletPageWidget::getRotation() const
{
    QString rotation = ScreenRotation::NONE.key();

    if (isAutoRotationEnabled()) {
        if (isAutoRotateInversionEnabled()) {
            rotation = ScreenRotation::AUTO_INVERTED.key();
        } else {
            rotation = ScreenRotation::AUTO.key();
        }
    } else {
        int index = ui->rotatationSelectionComboBox->currentIndex();
        rotation = ui->rotatationSelectionComboBox->itemData(index).toString();
    }

    return rotation;
}

const ScreenMap &TabletPageWidget::getScreenMap() const
{
    return _screenMap;
}

const QString TabletPageWidget::getScreenMapAsString() const
{
    return getScreenMap().toString();
}

const ScreenSpace &TabletPageWidget::getScreenSpace() const
{
    return _screenSpace;
}

const QString TabletPageWidget::getScreenSpaceAsString() const
{
    return getScreenSpace().toString();
}

const QString TabletPageWidget::getTrackingMode() const
{
    if (ui->trackAbsoluteRadioButton->isChecked()) {
        return QLatin1String("absolute");
    }

    return QLatin1String("relative");
}

bool TabletPageWidget::isAutoRotateInversionEnabled() const
{
    return ui->rotateWithScreenInvertCheckBox->isChecked();
}

bool TabletPageWidget::isAutoRotationEnabled() const
{
    return ui->rotateWithScreenCheckBox->isChecked();
}

void TabletPageWidget::setAutoRotateInversionEnabled(bool value)
{
    ui->rotateWithScreenInvertCheckBox->blockSignals(true);
    ui->rotateWithScreenInvertCheckBox->setChecked(value);
    ui->rotateWithScreenInvertCheckBox->blockSignals(false);
}

void TabletPageWidget::setAutoRotationEnabled(bool value)
{
    ui->rotatationSelectionComboBox->setEnabled(!value);
    ui->rotateWithScreenInvertCheckBox->setEnabled(value);

    if (value) {
        setRotation(ScreenRotation::NONE.key());
    } else {
        setAutoRotateInversionEnabled(false);
    }

    ui->rotateWithScreenCheckBox->blockSignals(true);
    ui->rotateWithScreenCheckBox->setChecked(value);
    ui->rotateWithScreenCheckBox->blockSignals(false);
}

void TabletPageWidget::setRotation(const QString &value)
{
    const ScreenRotation *lookup = ScreenRotation::find(value);
    ScreenRotation rotation = lookup ? *lookup : ScreenRotation::NONE;
    QString rotationValue = rotation.key();

    if (rotation == ScreenRotation::AUTO) {
        setAutoRotationEnabled(true);
        rotationValue = ScreenRotation::NONE.key();

    } else if (rotation == ScreenRotation::AUTO_INVERTED) {
        setAutoRotationEnabled(true);
        setAutoRotateInversionEnabled(true);
        rotationValue = ScreenRotation::NONE.key();
    }

    int rotationIndex = ui->rotatationSelectionComboBox->findData(rotationValue);

    ui->rotatationSelectionComboBox->blockSignals(true);
    ui->rotatationSelectionComboBox->setCurrentIndex(rotationIndex >= 0 ? rotationIndex : 0);
    ui->rotatationSelectionComboBox->blockSignals(false);

    onRotationChanged();
}

void TabletPageWidget::setScreenMap(const ScreenMap &screenMap)
{
    _screenMap = screenMap;

    assertValidTabletMapping();
}

void TabletPageWidget::setScreenMap(const QString &value)
{
    setScreenMap(ScreenMap(value));
}

void TabletPageWidget::setScreenSpace(const ScreenSpace &screenSpace)
{
    _screenSpace = screenSpace;

    assertValidTabletMapping();
}

void TabletPageWidget::setScreenSpace(const QString &value)
{
    setScreenSpace(ScreenSpace(value));
}

void TabletPageWidget::setTrackingMode(const QString &value)
{
    ui->trackAbsoluteRadioButton->blockSignals(true);
    ui->trackRelativeRadioButton->blockSignals(true);

    if (value.contains(QLatin1String("absolute"), Qt::CaseInsensitive)) {
        ui->trackAbsoluteRadioButton->setChecked(true);
        ui->trackRelativeRadioButton->setChecked(false);
    } else {
        ui->trackAbsoluteRadioButton->setChecked(false);
        ui->trackRelativeRadioButton->setChecked(true);
    }

    ui->trackAbsoluteRadioButton->blockSignals(false);
    ui->trackRelativeRadioButton->blockSignals(false);

    assertValidTabletMapping();
}

void TabletPageWidget::assertValidTabletMapping()
{
    bool isWarningVisible = false;

    if (ui->trackRelativeRadioButton->isChecked()) {
        // Relative mode is selected. In relative mode a
        // device can not be mapped to a single monitor
        ScreenSpace screenSpace = getScreenSpace();

        if (screenSpace.isMonitor()) {
            isWarningVisible = true;
        }
    }

    ui->trackingWarningIcon->setVisible(isWarningVisible);
    ui->trackingWarningLabel->setVisible(isWarningVisible);
}

void TabletPageWidget::setupUi()
{
    ui->setupUi(this);

    // init screen mapping warning
    ui->trackingWarningIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(QSize(16, 16)));
    ui->trackingWarningIcon->setVisible(false);
    ui->trackingWarningLabel->setVisible(false);

    // fill rotation combo box
    // xsetwacom's rotation is based on coordinate rotation, but we are asking the user for a tablet rotation.
    // Therefore we have to swap the values for clockwise and counterclockwise rotation.
    ui->rotatationSelectionComboBox->addItem(i18nc("Either no orientation or the current screen orientation is applied to the tablet.", "Default Orientation"),
                                             ScreenRotation::NONE.key());
    ui->rotatationSelectionComboBox->addItem(i18nc("The tablet will be rotated clockwise.", "Rotate Tablet Clockwise"), ScreenRotation::CCW.key());
    ui->rotatationSelectionComboBox->addItem(i18nc("The tablet will be rotated counterclockwise.", "Rotate Tablet Counterclockwise"), ScreenRotation::CW.key());
    ui->rotatationSelectionComboBox->addItem(i18nc("The tablet will be rotated up side down.", "Rotate Tablet Upside-Down"), ScreenRotation::HALF.key());
}

#include "moc_tabletpagewidget.cpp"
