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

#include "buttonpagewidget.h"
#include "ui_buttonpagewidget.h"

#include "buttonactionselectiondialog.h"
#include "buttonactionselectorwidget.h"
#include "profilemanagement.h"

// common includes
#include "buttonshortcut.h"
#include "dbustabletinterface.h"
#include "deviceprofile.h"
#include "property.h"
#include "stringutils.h"
#include "tabletinfo.h"

// stdlib
#include <memory>

// KDE includes

// Qt includes
#include <QDBusInterface>
#include <QDBusReply>
#include <QDialog>
#include <QFile>
#include <QKeySequence>
#include <QLabel>
#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QStandardPaths>

using namespace Wacom;

QString findLayoutFile(const QString &filename)
{
    if (filename.isEmpty())
        return QString();

    const QString builtinLayout = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("wacomtablet/images/%1.png").arg(filename));
    if (QFile::exists(builtinLayout)) {
        return builtinLayout;
    } else if (QFile::exists(filename)) {
        return filename;
    }

    return QString();
}

ButtonPageWidget::ButtonPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ButtonPageWidget)
{
    setupUi();
    reloadWidget();
}

ButtonPageWidget::~ButtonPageWidget()
{
    delete ui;
}

void ButtonPageWidget::setTabletId(const QString &tabletId)
{
    _tabletId = tabletId;
}

void ButtonPageWidget::saveToProfile(ProfileManagementInterface &profileManagement)
{
    DeviceProfile padProfile = profileManagement.loadDeviceProfile(DeviceType::Pad);

    // save button shortcuts
    ButtonActionSelectorWidget *buttonSelector;

    for (int i = 1; i < 19; ++i) {
        buttonSelector = this->findChild<ButtonActionSelectorWidget *>(QString::fromLatin1("button%1ActionSelector").arg(i));

        if (buttonSelector && buttonSelector->isEnabled()) {
            padProfile.setButton(i, buttonSelector->getShortcut().toString());
        } else {
            // Make sure only valid buttons are set.
            // If invalid button numbers are set they might overwrite mapped buttons.
            // For instance, if button 4 gets mapped to 9, then setting button 9 will
            // overwrite the value from button 4 unless the device actually has a
            // button 9 which would then in turn have another mapping to another X11
            // button number, e.g. 13.
            padProfile.setButton(i, QString());
        }
    }

    // save strip shortcuts - reset invalid ones - same reasons as above
    QString stripLUp, stripRUp, stripLDown, stripRDown;

    if (ui->touchStripGroupBox->isEnabled()) {
        if (ui->leftStripWidget->isEnabled()) {
            stripLUp = ui->leftStripUpSelector->getShortcut().toString();
            stripLDown = ui->leftStripDownSelector->getShortcut().toString();
        }

        if (ui->rightStripWidget->isEnabled()) {
            stripRUp = ui->rightStripUpSelector->getShortcut().toString();
            stripRDown = ui->rightStripDownSelector->getShortcut().toString();
        }
    }

    padProfile.setProperty(Property::StripLeftUp, stripLUp);
    padProfile.setProperty(Property::StripLeftDown, stripLDown);
    padProfile.setProperty(Property::StripRightUp, stripRUp);
    padProfile.setProperty(Property::StripRightDown, stripRDown);

    // save wheel and ring shortcuts - reset invalid values - same reasons as above
    QString absWUp, absWDown;

    if (ui->touchRingGroupBox->isEnabled() || ui->wheelGroupBox->isEnabled()) {
        // ring and wheel shortcuts are treated the same but only one value may be written,
        // as the other one could be empty. Use whichever value we can get our hands on first.
        if (ui->ringUpSelector->getShortcut().isSet()) {
            absWUp = ui->ringUpSelector->getShortcut().toString();
        } else {
            absWUp = ui->wheelUpSelector->getShortcut().toString();
        }

        if (ui->ringDownSelector->getShortcut().isSet()) {
            absWDown = ui->ringDownSelector->getShortcut().toString();
        } else {
            absWDown = ui->wheelDownSelector->getShortcut().toString();
        }
    }

    padProfile.setProperty(Property::AbsWheelUp, absWUp);
    padProfile.setProperty(Property::AbsWheel2Up, absWUp);
    padProfile.setProperty(Property::AbsWheelDown, absWDown);
    padProfile.setProperty(Property::AbsWheel2Down, absWDown);

    // save device profile
    profileManagement.saveDeviceProfile(padProfile);
}

void ButtonPageWidget::loadFromProfile(ProfileManagementInterface &profileManagement)
{
    DeviceProfile padProfile = profileManagement.loadDeviceProfile(DeviceType::Pad);
    QString propertyValue;

    for (int i = 1; i < 19; i++) {
        auto buttonSelector = this->findChild<ButtonActionSelectorWidget *>(QString::fromLatin1("button%1ActionSelector").arg(i));
        propertyValue = padProfile.getButton(i);

        if (buttonSelector) {
            buttonSelector->setShortcut(ButtonShortcut(propertyValue));
        }
    }

    // set wheel and ring shortcuts
    propertyValue = padProfile.getProperty(Property::AbsWheelUp);
    ui->wheelUpSelector->setShortcut(ButtonShortcut(propertyValue));
    ui->ringUpSelector->setShortcut(ButtonShortcut(propertyValue));

    propertyValue = padProfile.getProperty(Property::AbsWheelDown);
    ui->wheelDownSelector->setShortcut(ButtonShortcut(propertyValue));
    ui->ringDownSelector->setShortcut(ButtonShortcut(propertyValue));

    // set strip shortcuts
    propertyValue = padProfile.getProperty(Property::StripLeftUp);
    ui->leftStripUpSelector->setShortcut(ButtonShortcut(propertyValue));

    propertyValue = padProfile.getProperty(Property::StripLeftDown);
    ui->leftStripDownSelector->setShortcut(ButtonShortcut(propertyValue));

    propertyValue = padProfile.getProperty(Property::StripRightUp);
    ui->rightStripUpSelector->setShortcut(ButtonShortcut(propertyValue));

    propertyValue = padProfile.getProperty(Property::StripRightDown);
    ui->rightStripDownSelector->setShortcut(ButtonShortcut(propertyValue));
}

void ButtonPageWidget::reloadWidget()
{
    const int padButtons = DBusTabletInterface::instance().getInformation(_tabletId, TabletInfo::NumPadButtons.key()).value().toInt();

    QLabel *buttonLabel = nullptr;
    ButtonActionSelectorWidget *buttonSelector = nullptr;
    for (int i = 1; i < 19; i++) {
        buttonSelector = this->findChild<ButtonActionSelectorWidget *>(QString::fromLatin1("button%1ActionSelector").arg(i));
        buttonLabel = this->findChild<QLabel *>(QString::fromLatin1("button%1Label").arg(i));

        if (!buttonSelector || !buttonLabel) {
            continue;
        }

        // we have to disable the widgets as well because when writing
        // the configuration file we can not reliably determine their state
        // based on visibility.
        if (i <= padButtons) {
            buttonLabel->setVisible(true);
            buttonSelector->setEnabled(true);
            buttonSelector->setVisible(true);
        } else {
            buttonLabel->setVisible(false);
            buttonSelector->setEnabled(false);
            buttonSelector->setVisible(false);
        }
    }

    const QString padLayoutProperty = DBusTabletInterface::instance().getInformation(_tabletId, TabletInfo::ButtonLayout.key());
    const QString layoutFile = findLayoutFile(padLayoutProperty);
    if (!layoutFile.isEmpty()) {
        // FIXME: libwacom svg's are large in size and have small labels
        // This looks ugly for some devices/with some themes, think of a better layout
        ui->padImage->setPixmap(QPixmap(layoutFile));
    }

    const bool anyButtonsOrLayout = padButtons > 0 || (!layoutFile.isEmpty());
    ui->buttonGroupBox->setVisible(anyButtonsOrLayout);

    bool hasLeftTouchStrip = StringUtils::asBool(DBusTabletInterface::instance().getInformation(_tabletId, TabletInfo::HasLeftTouchStrip.key()));
    bool hasRightTouchStrip = StringUtils::asBool(DBusTabletInterface::instance().getInformation(_tabletId, TabletInfo::HasRightTouchStrip.key()));

    if (!hasLeftTouchStrip && !hasRightTouchStrip) {
        ui->touchStripGroupBox->setEnabled(false);
        ui->touchStripGroupBox->setVisible(false);

    } else {
        ui->touchStripGroupBox->setEnabled(true);
        ui->touchStripGroupBox->setVisible(true);

        // Hide the strip input widgets directly instead of
        // their parent widget, to keep the layout stable.
        // Hiding the parent widget will mess up the layout!
        // Also disable the widgets so we can reliable determine
        // which settings to save.
        if (!hasLeftTouchStrip) {
            ui->leftStripWidget->setEnabled(false);
            ui->leftStripUpLabel->setVisible(false);
            ui->leftStripUpSelector->setVisible(false);
            ui->leftStripDownLabel->setVisible(false);
            ui->leftStripDownSelector->setVisible(false);
        } else {
            ui->leftStripWidget->setEnabled(true);
            ui->leftStripUpLabel->setVisible(true);
            ui->leftStripUpSelector->setVisible(true);
            ui->leftStripDownLabel->setVisible(true);
            ui->leftStripDownSelector->setVisible(true);
        }

        if (!hasRightTouchStrip) {
            ui->rightStripWidget->setEnabled(false);
            ui->rightStripUpLabel->setVisible(false);
            ui->rightStripUpSelector->setVisible(false);
            ui->rightStripDownLabel->setVisible(false);
            ui->rightStripDownSelector->setVisible(false);
        } else {
            ui->rightStripWidget->setEnabled(true);
            ui->rightStripUpLabel->setVisible(true);
            ui->rightStripUpSelector->setVisible(true);
            ui->rightStripDownLabel->setVisible(true);
            ui->rightStripDownSelector->setVisible(true);
        }
    }

    if (!StringUtils::asBool(DBusTabletInterface::instance().getInformation(_tabletId, TabletInfo::HasTouchRing.key()))) {
        ui->touchRingGroupBox->setEnabled(false);
        ui->touchRingGroupBox->setVisible(false);
    } else {
        ui->touchRingGroupBox->setEnabled(true);
        ui->touchRingGroupBox->setVisible(true);
    }

    if (!StringUtils::asBool(DBusTabletInterface::instance().getInformation(_tabletId, TabletInfo::HasWheel.key()))) {
        ui->wheelGroupBox->setEnabled(false);
        ui->wheelGroupBox->setVisible(false);
    } else {
        ui->wheelGroupBox->setEnabled(true);
        ui->wheelGroupBox->setVisible(true);
    }
}

void ButtonPageWidget::onButtonActionChanged()
{
    emit changed();
}

void ButtonPageWidget::setupUi()
{
    ui->setupUi(this);

    connect(ui->button1ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button2ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button3ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button4ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button5ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button6ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button7ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button8ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button9ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button10ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button11ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button12ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button13ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button14ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button15ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button16ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button17ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->button18ActionSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));

    connect(ui->leftStripUpSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->leftStripDownSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->rightStripUpSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->rightStripDownSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));

    connect(ui->ringUpSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->ringDownSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));

    connect(ui->wheelUpSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
    connect(ui->wheelDownSelector, SIGNAL(buttonActionChanged(ButtonShortcut)), this, SLOT(onButtonActionChanged()));
}

#include "moc_buttonpagewidget.cpp"
