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

#include "touchpagewidget.h"
#include "ui_touchpagewidget.h"

#include "dbustabletinterface.h"
#include "deviceprofile.h"
#include "profilemanagement.h"
#include "property.h"
#include "stringutils.h"
#include "tabletareaselectiondialog.h"
#include "x11wacom.h"

#include <QStringList>

using namespace Wacom;

TouchPageWidget::TouchPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TouchPageWidget)
{
    setupUi();
}

TouchPageWidget::~TouchPageWidget()
{
    delete ui;
}

void TouchPageWidget::setTabletId(const QString &tabletId)
{
    _tabletId = tabletId;
}

void TouchPageWidget::loadFromProfile(ProfileManagementInterface &profileManagement)
{
    DeviceProfile touchProfile = profileManagement.loadDeviceProfile(DeviceType::Touch);

    // set all properties no matter if the tablet supports that device
    // to get all widgets properly initialized.

    setTouchSupportEnabled(touchProfile.getPropertyAsBool(Property::Touch));
    setTrackingMode(touchProfile.getProperty(Property::Mode));
    setScreenSpace(touchProfile.getProperty(Property::ScreenSpace));
    setScreenMap(touchProfile.getProperty(Property::ScreenMap));
    setGesturesSupportEnabled(touchProfile.getPropertyAsBool(Property::Gesture));
    setScrollDistance(touchProfile.getProperty(Property::ScrollDistance));
    setScrollInversion(touchProfile.getProperty(Property::InvertScroll));
    setZoomDistance(touchProfile.getProperty(Property::ZoomDistance));
    setTapTime(touchProfile.getProperty(Property::TapTime));
}

void TouchPageWidget::reloadWidget()
{
    // get all tablet device names we need
    QDBusReply<QString> touchDeviceNameReply = DBusTabletInterface::instance().getDeviceName(_tabletId, DeviceType::Touch.key());

    // update name and maximum tablet area for all devices
    _touchDeviceName.clear();
    _tabletGeometry = TabletArea();
    _screenMap = ScreenMap();

    if (touchDeviceNameReply.isValid()) {
        _touchDeviceName = touchDeviceNameReply.value();
        if (!_touchDeviceName.isEmpty()) { // touch device available
            _tabletGeometry = X11Wacom::getMaximumTabletArea(touchDeviceNameReply.value());
            _screenMap = ScreenMap(_tabletGeometry);
        }
    }
}

void TouchPageWidget::saveToProfile(ProfileManagementInterface &profileManagement)
{
    if (_touchDeviceName.isEmpty()) {
        return; // no touch device available
    }

    DeviceProfile touchProfile = profileManagement.loadDeviceProfile(DeviceType::Touch);

    touchProfile.setProperty(Property::Touch, getTouchSupportEnabled());
    touchProfile.setProperty(Property::Mode, getTrackingMode());
    touchProfile.setProperty(Property::ScreenSpace, getScreenSpaceAsString());
    touchProfile.setProperty(Property::ScreenMap, getScreenMapAsString());
    touchProfile.setProperty(Property::Gesture, getGestureSupportEnabled());
    touchProfile.setProperty(Property::ScrollDistance, getScrollDistance());
    touchProfile.setProperty(Property::InvertScroll, getScrollInversion());
    touchProfile.setProperty(Property::ZoomDistance, getZoomDistance());
    touchProfile.setProperty(Property::TapTime, getTapTime());
    touchProfile.setProperty(Property::Rotate, _tabletRotation.key());

    profileManagement.saveDeviceProfile(touchProfile);
}

void TouchPageWidget::onGesturesModeChanged(int state)
{
    if (state == Qt::Unchecked) {
        ui->gesturesWarning->animatedHide();
    } else {
        ui->gesturesWarning->animatedShow();
    }

    setGesturesSupportEnabled(state == Qt::Checked);
    onProfileChanged();
}

void TouchPageWidget::onProfileChanged()
{
    emit changed();
}

void TouchPageWidget::onRotationChanged(const ScreenRotation &rotation)
{
    _tabletRotation = rotation;
}

void TouchPageWidget::onTabletMappingClicked()
{
    TabletAreaSelectionDialog selectionDialog;
    selectionDialog.setupWidget(getScreenMap(), _touchDeviceName, _tabletRotation);
    selectionDialog.select(getScreenSpace());

    if (selectionDialog.exec() == QDialog::Accepted) {
        setScreenMap(selectionDialog.getScreenMap());
        setScreenSpace(selectionDialog.getScreenSpace());
        onProfileChanged();
    }
}

void TouchPageWidget::onTouchModeChanged(int state)
{
    // Show/hide the gestures warning as needed, since its UI gets enabled and
    // disabled dynamically when touch is turned on or off
    if (state == Qt::Unchecked) {
        ui->gesturesWarning->animatedHide();
    } else if (ui->gesturesCheckBox->isChecked()) {
        ui->gesturesWarning->animatedShow();
    }

    setTouchSupportEnabled(state == Qt::Checked);
    onProfileChanged();
}

void TouchPageWidget::onTrackingModeAbsolute(bool activated)
{
    if (!activated) {
        return;
    }

    setTrackingMode(QLatin1String("absolute"));
    onProfileChanged();
}

void TouchPageWidget::onTrackingModeRelative(bool activated)
{
    if (!activated) {
        return;
    }

    setTrackingMode(QLatin1String("relative"));
    onProfileChanged();
}

const QString TouchPageWidget::getGestureSupportEnabled() const
{
    return (isGesturesSupportEnabled() ? QLatin1String("on") : QLatin1String("off"));
}

const ScreenMap &TouchPageWidget::getScreenMap() const
{
    return _screenMap;
}

const QString TouchPageWidget::getScreenMapAsString() const
{
    return getScreenMap().toString();
}

const ScreenSpace &TouchPageWidget::getScreenSpace() const
{
    return _screenSpace;
}

const QString TouchPageWidget::getScreenSpaceAsString() const
{
    return getScreenSpace().toString();
}

const QString TouchPageWidget::getScrollDistance() const
{
    return QString::number(ui->scrollDistanceSpinBox->value());
}

const QString TouchPageWidget::getScrollInversion() const
{
    return (ui->scrollInversionCheckBox->isChecked() ? QLatin1String("on") : QLatin1String("off"));
}

const QString TouchPageWidget::getTapTime() const
{
    return QString::number(ui->tapTimeSpinBox->value());
}

const QString TouchPageWidget::getTouchSupportEnabled() const
{
    return (isTouchSupportEnabled() ? QLatin1String("on") : QLatin1String("off"));
}

const QString TouchPageWidget::getTrackingMode() const
{
    if (ui->trackAbsoluteRadioButton->isChecked()) {
        return QLatin1String("absolute");
    }

    return QLatin1String("relative");
}

const QString TouchPageWidget::getZoomDistance() const
{
    return QString::number(ui->zoomDistanceSpinBox->value());
}

bool TouchPageWidget::isGesturesSupportEnabled() const
{
    return ui->gesturesCheckBox->isChecked();
}

bool TouchPageWidget::isTouchSupportEnabled() const
{
    return ui->touchCheckBox->isChecked();
}

void TouchPageWidget::setGesturesSupportEnabled(bool value)
{
    ui->gesturesGroupBox->setEnabled(value);

    ui->gesturesCheckBox->blockSignals(true);
    ui->gesturesCheckBox->setChecked(value);
    ui->gesturesCheckBox->blockSignals(false);
}

void TouchPageWidget::setScreenMap(const ScreenMap &screenMap)
{
    _screenMap = screenMap;

    assertValidTabletMapping();
}

void TouchPageWidget::setScreenMap(const QString &value)
{
    setScreenMap(ScreenMap(value));
}

void TouchPageWidget::setScreenSpace(const ScreenSpace &screenSpace)
{
    _screenSpace = screenSpace;

    assertValidTabletMapping();
}

void TouchPageWidget::setScreenSpace(const QString &value)
{
    setScreenSpace(ScreenSpace(value));
}

void TouchPageWidget::setScrollDistance(const QString &value)
{
    ui->scrollDistanceSpinBox->blockSignals(true);
    ui->scrollDistanceSpinBox->setValue(value.toInt());
    ui->scrollDistanceSpinBox->blockSignals(false);
}

void TouchPageWidget::setScrollInversion(const QString &value)
{
    ui->scrollInversionCheckBox->blockSignals(true);
    ui->scrollInversionCheckBox->setChecked(StringUtils::asBool(value));
    ui->scrollInversionCheckBox->blockSignals(false);
}

void TouchPageWidget::setTouchSupportEnabled(bool value)
{
    ui->trackingModeGroupBox->setEnabled(value);
    ui->touchMappingGroupBox->setEnabled(value);
    ui->gesturesCheckBox->setEnabled(value);

    if (isGesturesSupportEnabled()) {
        ui->gesturesGroupBox->setEnabled(value);
    }

    ui->touchCheckBox->blockSignals(true);
    ui->touchCheckBox->setChecked(value);
    ui->touchCheckBox->blockSignals(false);
}

void TouchPageWidget::setTapTime(const QString &value)
{
    ui->tapTimeSpinBox->blockSignals(true);
    ui->tapTimeSpinBox->setValue(value.toInt());
    ui->tapTimeSpinBox->blockSignals(false);
}

void TouchPageWidget::setTrackingMode(const QString &value)
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

void TouchPageWidget::setZoomDistance(const QString &value)
{
    ui->zoomDistanceSpinBox->blockSignals(true);
    ui->zoomDistanceSpinBox->setValue(value.toInt());
    ui->zoomDistanceSpinBox->blockSignals(false);
}

void TouchPageWidget::assertValidTabletMapping()
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

void TouchPageWidget::setupUi()
{
    ui->setupUi(this);

    // init screen mapping warning
    ui->trackingWarningIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(QSize(16, 16)));
    ui->trackingWarningIcon->setVisible(false);
    ui->trackingWarningLabel->setVisible(false);
}

#include "moc_touchpagewidget.cpp"
