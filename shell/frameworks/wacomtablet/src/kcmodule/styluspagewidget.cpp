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

#include "styluspagewidget.h"
#include "ui_styluspagewidget.h"

#include "buttonactionselectiondialog.h"
#include "pressurecurvedialog.h"
#include "profilemanagement.h"

// common includes
#include "buttonshortcut.h"
#include "dbustabletinterface.h"
#include "deviceprofile.h"
#include "property.h"

#include <QPixmap>
#include <QStandardPaths>

using namespace Wacom;

StylusPageWidget::StylusPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StylusPageWidget)
{
    setupUi();
}

StylusPageWidget::~StylusPageWidget()
{
    delete ui;
}

void StylusPageWidget::setTabletId(const QString &tabletId)
{
    _tabletId = tabletId;
}

void StylusPageWidget::loadFromProfile(ProfileManagementInterface &profileManagement)
{
    DeviceProfile stylusProfile = profileManagement.loadDeviceProfile(DeviceType::Stylus);
    DeviceProfile eraserProfile = profileManagement.loadDeviceProfile(DeviceType::Eraser);

    // eraser feel / tip feel
    setPressureFeel(DeviceType::Eraser, eraserProfile.getProperty(Property::Threshold));
    setPressureCurve(DeviceType::Eraser, eraserProfile.getProperty(Property::PressureCurve));
    setPressureFeel(DeviceType::Stylus, stylusProfile.getProperty(Property::Threshold));
    setPressureCurve(DeviceType::Stylus, stylusProfile.getProperty(Property::PressureCurve));

    // Button Actions
    setButtonShortcut(Property::Button1, stylusProfile.getProperty(Property::Button1));
    setButtonShortcut(Property::Button2, stylusProfile.getProperty(Property::Button2));
    setButtonShortcut(Property::Button3, stylusProfile.getProperty(Property::Button3));

    // Tap to Click
    setTabletPcButton(stylusProfile.getProperty(Property::TabletPcButton));

    // Raw Sample Rate
    ui->horizontalSliderRawSample->setValue(stylusProfile.getProperty(Property::RawSample).toInt());

    // Suppress Rate
    ui->horizontalSliderSuppress->setValue(stylusProfile.getProperty(Property::Suppress).toInt());
}

void StylusPageWidget::reloadWidget()
{
    // nothing to do
}

void StylusPageWidget::saveToProfile(ProfileManagementInterface &profileManagement)
{
    DeviceProfile stylusProfile = profileManagement.loadDeviceProfile(DeviceType::Stylus);
    DeviceProfile eraserProfile = profileManagement.loadDeviceProfile(DeviceType::Eraser);

    savePropertiesToDeviceProfile(stylusProfile);
    savePropertiesToDeviceProfile(eraserProfile);

    // tap to click
    stylusProfile.setProperty(Property::TabletPcButton, getTabletPcButton());

    profileManagement.saveDeviceProfile(stylusProfile);
    profileManagement.saveDeviceProfile(eraserProfile);
}

void StylusPageWidget::savePropertiesToDeviceProfile(DeviceProfile &profile) const
{
    const auto deviceType = profile.getDeviceType();

    // eraser / tip pressure
    profile.setProperty(Property::Threshold, getPressureFeel(deviceType));
    profile.setProperty(Property::PressureCurve, getPressureCurve(deviceType));

    // Buttons 1, 2 and 3 config
    profile.setProperty(Property::Button1, getButtonShortcut(Property::Button1));
    profile.setProperty(Property::Button2, getButtonShortcut(Property::Button2));
    profile.setProperty(Property::Button3, getButtonShortcut(Property::Button3));

    // Raw Sample Rate
    profile.setProperty(Property::RawSample, QString::number(ui->horizontalSliderRawSample->value()));

    // Suppress Rate
    profile.setProperty(Property::Suppress, QString::number(ui->horizontalSliderSuppress->value()));
}

void StylusPageWidget::onChangeEraserPressureCurve()
{
    openPressureCurveDialog(DeviceType::Eraser);
}

void StylusPageWidget::onChangeTipPressureCurve()
{
    openPressureCurveDialog(DeviceType::Stylus);
}

void StylusPageWidget::onProfileChanged()
{
    emit changed();
}

const QString StylusPageWidget::getButtonShortcut(const Property &button) const
{
    ButtonShortcut shortcut;

    if (button == Property::Button1) {
        shortcut = ui->button1ActionSelector->getShortcut();
    } else if (button == Property::Button2) {
        shortcut = ui->button2ActionSelector->getShortcut();
    } else if (button == Property::Button3) {
        shortcut = ui->button3ActionSelector->getShortcut();
    } else {
        qCWarning(KCM) << QString::fromLatin1("Internal Error: Unknown button property '%1' provided!").arg(button.key());
    }

    return shortcut.toString();
}

const QString StylusPageWidget::getPressureCurve(const DeviceType &type) const
{
    if (type == DeviceType::Stylus) {
        return ui->tipPressureButton->property("curve").toString();
    } else if (type == DeviceType::Eraser) {
        return ui->eraserPressureButton->property("curve").toString();
    } else {
        qCWarning(KCM) << QString::fromLatin1("Invalid device type '%1' provided!").arg(type.key());
    }

    return QString();
}

const QString StylusPageWidget::getPressureFeel(const DeviceType &type) const
{
    if (type == DeviceType::Stylus) {
        return QString::number(ui->tipSlider->value());
    } else if (type == DeviceType::Eraser) {
        return QString::number(ui->eraserSlider->value());
    } else {
        qCWarning(KCM) << QString::fromLatin1("Invalid device type '%1' provided!").arg(type.key());
    }

    return QString();
}

const QString StylusPageWidget::getTabletPcButton() const
{
    return (ui->tpcCheckBox->isChecked() ? QLatin1String("on") : QLatin1String("off"));
}

void StylusPageWidget::setButtonShortcut(const Property &button, const QString &shortcut)
{
    if (button == Property::Button1) {
        ui->button1ActionSelector->setShortcut(ButtonShortcut(shortcut));
    } else if (button == Property::Button2) {
        ui->button2ActionSelector->setShortcut(ButtonShortcut(shortcut));
    } else if (button == Property::Button3) {
        ui->button3ActionSelector->setShortcut(ButtonShortcut(shortcut));
    } else {
        qCWarning(KCM) << QString::fromLatin1("Internal Error: Unknown button property '%1' provided!").arg(button.key());
    }
}

void StylusPageWidget::setPressureCurve(const DeviceType &type, const QString &value)
{
    if (type == DeviceType::Stylus) {
        ui->tipPressureButton->setProperty("curve", value);
    } else if (type == DeviceType::Eraser) {
        ui->eraserPressureButton->setProperty("curve", value);
    } else {
        qCWarning(KCM) << QString::fromLatin1("Internal Error: Invalid device type '%1' provided!").arg(type.key());
    }
}

void StylusPageWidget::setPressureFeel(const DeviceType &type, const QString &value)
{
    if (type == DeviceType::Stylus) {
        ui->tipSlider->setValue(value.toInt());
    } else if (type == DeviceType::Eraser) {
        ui->eraserSlider->setValue(value.toInt());
    } else {
        qCWarning(KCM) << QString::fromLatin1("Internal Error: Invalid device type '%1' provided!").arg(type.key());
    }
}

void StylusPageWidget::setTabletPcButton(const QString &value)
{
    if (value.compare(QLatin1String("on"), Qt::CaseInsensitive) == 0) {
        ui->tpcCheckBox->setChecked(true);
    } else {
        ui->tpcCheckBox->setChecked(false);
    }
}

void StylusPageWidget::openPressureCurveDialog(const DeviceType &deviceType)
{
    QString initialPressureCurve = getPressureCurve(deviceType);
    QScopedPointer<PressureCurveDialog> pressureCurveDialog(new PressureCurveDialog(initialPressureCurve, _tabletId, deviceType, this));

    if (pressureCurveDialog->exec() == QDialog::Accepted) {
        QString newPressureCurve = pressureCurveDialog->getControllPoints();

        if (newPressureCurve != initialPressureCurve) {
            setPressureCurve(deviceType, newPressureCurve);
            emit changed();
        }
    }
}

void StylusPageWidget::setupUi()
{
    ui->setupUi(this);

    ui->penLabel->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("wacomtablet/images/pen.png"))));

    connect(ui->button1ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onProfileChanged()));
    connect(ui->button2ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onProfileChanged()));
    connect(ui->button3ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onProfileChanged()));
}

#include "moc_styluspagewidget.cpp"
