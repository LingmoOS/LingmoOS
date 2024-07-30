/*
    SPDX-FileCopyrightText: 2010-2012 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mobileconnectionwizard.h"
#include "uiutils.h"

#include <QVBoxLayout>

#include <KLocalizedString>
#include <Solid/Device>

#include <NetworkManagerQt/ModemDevice>

#include <ModemManagerQt/Manager>
#include <ModemManagerQt/Modem>

#define NUMBER_OF_STATIC_ENTRIES 3

MobileConnectionWizard::MobileConnectionWizard(NetworkManager::ConnectionSettings::ConnectionType connectionType, QWidget *parent)
    : QWizard(parent)
{
    if (connectionType == NetworkManager::ConnectionSettings::Unknown) {
        mInitialMethodType = false;
    } else {
        mInitialMethodType = true;

        if (connectionType == NetworkManager::ConnectionSettings::Bluetooth) {
            mType = NetworkManager::ConnectionSettings::Gsm;
        } else {
            mType = connectionType;
        }
    }

    mProviders = new MobileProviders();
    setWindowTitle(i18nc("Mobile Connection Wizard", "New Mobile Broadband Connection"));
    addPage(createIntroPage());
    addPage(createCountryPage());
    addPage(createProvidersPage());
    addPage(createPlansPage());
    addPage(createConfirmPage());
    setOptions(QWizard::NoBackButtonOnStartPage);
}

MobileConnectionWizard::~MobileConnectionWizard()
{
    delete mProviders;
}

MobileProviders::ErrorCodes MobileConnectionWizard::getError()
{
    if (mProviders) {
        return mProviders->getError();
    }
    return MobileProviders::Success;
}

void MobileConnectionWizard::initializePage(int id)
{
    switch (id) {
    case 1: { // Country Page
        if (country.isEmpty()) {
            country = mProviders->countryFromLocale();
        }

        if (country.isEmpty())
            mCountryList->setCurrentRow(0);
        else {
            const QList<QListWidgetItem *> items = mCountryList->findItems(mProviders->getCountryName(country), Qt::MatchExactly);
            if (!items.empty()) {
                mCountryList->setCurrentItem(items.first());
            }
        }

        if (!mInitialMethodType) {
            NetworkManager::Device::Ptr iface = NetworkManager::findNetworkInterface(mDeviceComboBox->itemData(mDeviceComboBox->currentIndex()).toString());
            if (iface) {
                NetworkManager::ModemDevice::Ptr nmModemIface = iface.objectCast<NetworkManager::ModemDevice>();
                if (nmModemIface && UiUtils::modemSubType(nmModemIface->currentCapabilities()) == NetworkManager::ModemDevice::CdmaEvdo) {
                    mType = NetworkManager::ConnectionSettings::Cdma;
                } else {
                    mType = NetworkManager::ConnectionSettings::Gsm;
                }
            } else {
                mType = static_cast<NetworkManager::ConnectionSettings::ConnectionType>(mDeviceComboBox->itemData(mDeviceComboBox->currentIndex()).toUInt());
            }
        }

        if (mProviders->getError() != MobileProviders::Success) {
            accept();
        }
        break;
    }

    case 2: // Providers Page
        country = mCountryList->currentItem()->text();
        mProvidersList->clear();
        lineEditProvider->clear();
        radioAutoProvider->setChecked(true);

        switch (type()) {
        case NetworkManager::ConnectionSettings::Gsm:
            mProvidersList->insertItems(0, mProviders->getProvidersList(country, NetworkManager::ConnectionSettings::Gsm));
            break;
        case NetworkManager::ConnectionSettings::Cdma:
            mProvidersList->insertItems(0, mProviders->getProvidersList(country, NetworkManager::ConnectionSettings::Cdma));
            break;
        default:
            break;
        }
        mProvidersList->setCurrentRow(0);
        if (mProvidersList->count() > 0) {
            mProvidersList->setEnabled(true);
            radioAutoProvider->setEnabled(true);
            mProvidersList->setFocus();
        } else {
            mProvidersList->setEnabled(false);
            radioAutoProvider->setEnabled(false);
            radioManualProvider->setChecked(true);
            // TODO: this does not work, try reimplementing QWizardPage::isComplete()
            // button(QWizard::NextButton)->setEnabled(false);
        }
        break;

    case 3: // Plans Page
        disconnect(mPlanComboBox, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &MobileConnectionWizard::slotEnablePlanEditBox);
        mPlanComboBox->clear();
        if (type() != NetworkManager::ConnectionSettings::Gsm) {
            goto OUT_3;
        }
        if (radioManualProvider->isChecked()) {
            mPlanComboBox->addItem(i18nc("Mobile Connection Wizard", "My plan is not listed…"));

            if (lineEditProvider->text().isEmpty()) {
                lineEditProvider->setText(i18nc("Mobile Connection Wizard", "Unknown Provider"));
            }
        } else {
            if (mProvidersList->currentItem() != nullptr) {
                const QStringList mApns = mProviders->getApns(mProvidersList->currentItem()->text());

                if (!mApns.isEmpty()) {
                    mPlanComboBox->insertItems(0, mApns);
                    mPlanComboBox->setItemText(0, i18nc("Mobile Connection Wizard", "Default"));
                }
            }

            if (mPlanComboBox->count()) {
                mPlanComboBox->insertSeparator(1);
            }
            mPlanComboBox->addItem(i18nc("Mobile Connection Wizard", "My plan is not listed…"));
        }
        mPlanComboBox->setCurrentIndex(0);
        slotEnablePlanEditBox(mPlanComboBox->currentIndex());
    OUT_3:
        connect(mPlanComboBox, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &MobileConnectionWizard::slotEnablePlanEditBox);
        break;

    case 4: // Confirm Page
        if (radioManualProvider->isChecked()) {
            labelProvider->setText("    " + lineEditProvider->text() + ", " + country);
            provider = lineEditProvider->text();
        } else {
            labelProvider->setText("    " + mProvidersList->currentItem()->text() + ", " + country);
            provider = mProvidersList->currentItem()->text();
        }

        if (type() == NetworkManager::ConnectionSettings::Cdma) {
            labelPlanLabel->hide();
            labelPlan->hide();
            labelApn->hide();
            userApn->clear();
            apn.clear();
        } else {
            labelPlanLabel->show();
            labelPlan->show();
            labelApn->show();

            if (mPlanComboBox->currentText() == i18nc("Mobile Connection Wizard", "My plan is not listed…")) {
                labelPlan->setText("    " + userApn->text());
                labelApn->setText("    " + i18nc("Mobile Connection Wizard", "APN: %1", userApn->text()));
                apn = userApn->text();
            } else {
                int i = mPlanComboBox->currentIndex();
                i = i > 1 ? (i - 1) : 0; // ignores separator's index (i == 1).

                QStringList mApns = mProviders->getApns(mProvidersList->currentItem()->text());
                labelPlan->setText("    " + mPlanComboBox->currentText());
                labelApn->setText("    " + i18nc("Mobile Connection Wizard", "APN: %1", mApns.at(i)));
                apn = mApns.at(i);
            }
        }
        break;
    }
}

int MobileConnectionWizard::nextId() const
{
    // Providers page
    if (currentId() == 2 && type() != NetworkManager::ConnectionSettings::Gsm) {
        // Jumps to Confirm page instead of Plans page if type != Gsm.
        return 4;
    } else {
        return QWizard::nextId();
    }
}

QVariantList MobileConnectionWizard::args()
{
    QVariantList temp;

    switch (type()) {
    case NetworkManager::ConnectionSettings::Cdma:
        temp << provider << mProviders->getCdmaInfo(provider);
        break;

    case NetworkManager::ConnectionSettings::Gsm:
        temp << provider /*<< mProviders->getNetworkIds(provider)*/ << mProviders->getApnInfo(apn);
        break;

    default:
        break;
    }
    return temp;
}

/**********************************************************/
/* Intro page */
/**********************************************************/

QWizardPage *MobileConnectionWizard::createIntroPage()
{
    auto page = new QWizardPage();
    page->setTitle(i18nc("Mobile Connection Wizard", "Set up a Mobile Broadband Connection"));
    auto layout = new QVBoxLayout;

    auto label =
        new QLabel(i18nc("Mobile Connection Wizard", "This assistant helps you easily set up a mobile broadband connection to a cellular (3G) network."));
    label->setWordWrap(true);
    layout->addWidget(label);

    label = new QLabel(QLatin1Char('\n') + i18nc("Mobile Connection Wizard", "You will need the following information:"));
    layout->addWidget(label);

    label = new QLabel(QStringLiteral("  . %1\n  . %2\n  . %3")
                           .arg(i18nc("Mobile Connection Wizard", "Your broadband provider's name"),
                                i18nc("Mobile Connection Wizard", "Your broadband billing plan name"),
                                i18nc("Mobile Connection Wizard", "(in some cases) Your broadband billing plan APN (Access Point Name)")));
    layout->addWidget(label);

    if (!mInitialMethodType) {
        label = new QLabel('\n' + i18nc("Mobile Connection Wizard", "Create a connection for &this mobile broadband device:"));
        layout->addWidget(label);

        mDeviceComboBox = new KComboBox();
        mDeviceComboBox->addItem(i18nc("Mobile Connection Wizard", "Any GSM device"));
        mDeviceComboBox->setItemData(0, NetworkManager::ConnectionSettings::Gsm);
        mDeviceComboBox->addItem(i18nc("Mobile Connection Wizard", "Any CDMA device"));
        mDeviceComboBox->setItemData(1, NetworkManager::ConnectionSettings::Cdma);
        mDeviceComboBox->insertSeparator(NUMBER_OF_STATIC_ENTRIES - 1);
        label->setBuddy(mDeviceComboBox);
        layout->addWidget(mDeviceComboBox);

        connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &MobileConnectionWizard::introDeviceAdded);
        connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &MobileConnectionWizard::introDeviceRemoved);
        connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &MobileConnectionWizard::introStatusChanged);

        introAddInitialDevices();
    }

    page->setLayout(layout);

    return page;
}

void MobileConnectionWizard::introAddDevice(const NetworkManager::Device::Ptr &device)
{
    QString desc;

    ModemManager::ModemDevice::Ptr modem = ModemManager::findModemDevice(device->udi());
    if (modem) {
        ModemManager::Modem::Ptr modemInterface = modem->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
        if (modemInterface->powerState() == MM_MODEM_POWER_STATE_ON) {
            desc.append(modemInterface->manufacturer());
            desc.append(QLatin1Char(' '));
            desc.append(modemInterface->model());
        } else {
            QString deviceName = modemInterface->device();
            for (const Solid::Device &d : Solid::Device::allDevices()) {
                if (d.udi().contains(deviceName, Qt::CaseInsensitive)) {
                    deviceName = d.product();
                    if (!deviceName.startsWith(d.vendor())) {
                        deviceName = d.vendor() + QLatin1Char(' ') + deviceName;
                    }
                    desc.append(deviceName);
                    break;
                }
            }
        }
    }

    NetworkManager::ModemDevice::Ptr nmModemIface = device.objectCast<NetworkManager::ModemDevice>();
    if (!nmModemIface) {
        return;
    }

    if (UiUtils::modemSubType(nmModemIface->currentCapabilities()) == NetworkManager::ModemDevice::GsmUmts) {
        if (desc.isEmpty()) {
            desc.append(i18nc("Mobile Connection Wizard", "Installed GSM device"));
        }
    } else if (UiUtils::modemSubType(nmModemIface->currentCapabilities()) == NetworkManager::ModemDevice::CdmaEvdo) {
        if (desc.isEmpty()) {
            desc.append(i18nc("Mobile Connection Wizard", "Installed CDMA device"));
        }
    } else {
        return;
    }

    mDeviceComboBox->addItem(desc, device->uni());

    if (mDeviceComboBox->count() == NUMBER_OF_STATIC_ENTRIES) {
        mDeviceComboBox->setCurrentIndex(0);
    } else {
        mDeviceComboBox->setCurrentIndex(NUMBER_OF_STATIC_ENTRIES);
    }
}

void MobileConnectionWizard::introDeviceAdded(const QString &uni)
{
    introAddDevice(NetworkManager::findNetworkInterface(uni));
}

void MobileConnectionWizard::introDeviceRemoved(const QString &uni)
{
    int index = mDeviceComboBox->findData(uni);

    mDeviceComboBox->removeItem(index);

    if (mDeviceComboBox->count() == NUMBER_OF_STATIC_ENTRIES) {
        mDeviceComboBox->setCurrentIndex(0);
        if (currentId() > 0) {
            close();
        }
    } else {
        mDeviceComboBox->setCurrentIndex(NUMBER_OF_STATIC_ENTRIES);
    }
}

void MobileConnectionWizard::introStatusChanged(NetworkManager::Status status)
{
    switch (status) {
    case NetworkManager::Unknown:
    case NetworkManager::Asleep:
    case NetworkManager::Disconnected:
    case NetworkManager::Disconnecting:
        introRemoveAllDevices();
        break;
    case NetworkManager::Connecting:
    case NetworkManager::ConnectedLinkLocal:
    case NetworkManager::ConnectedSiteOnly:
    case NetworkManager::Connected:
        introAddInitialDevices();
        break;
    }
}

void MobileConnectionWizard::introAddInitialDevices()
{
    for (const NetworkManager::Device::Ptr &n : NetworkManager::networkInterfaces()) {
        introAddDevice(n);
    }

    if (mDeviceComboBox->count() == NUMBER_OF_STATIC_ENTRIES) {
        mDeviceComboBox->setCurrentIndex(0);
    } else {
        mDeviceComboBox->setCurrentIndex(NUMBER_OF_STATIC_ENTRIES);
    }
}

void MobileConnectionWizard::introRemoveAllDevices()
{
    mDeviceComboBox->clear();
    mDeviceComboBox->addItem(i18nc("Mobile Connection Wizard", "Any GSM device"));
    mDeviceComboBox->setItemData(0, NetworkManager::ConnectionSettings::Gsm);
    mDeviceComboBox->addItem(i18nc("Mobile Connection Wizard", "Any CDMA device"));
    mDeviceComboBox->setItemData(1, NetworkManager::ConnectionSettings::Cdma);
    mDeviceComboBox->insertSeparator(NUMBER_OF_STATIC_ENTRIES - 1);
    mDeviceComboBox->setCurrentIndex(0);
}

/**********************************************************/
/* Country page */
/**********************************************************/

QWizardPage *MobileConnectionWizard::createCountryPage()
{
    auto page = new QWizardPage();
    page->setTitle(i18nc("Mobile Connection Wizard", "Choose your Provider's Country"));
    auto layout = new QVBoxLayout;

    auto label = new QLabel(i18nc("Mobile Connection Wizard", "Country List:"));
    layout->addWidget(label);

    mCountryList = new QListWidget();
    mCountryList->addItem(i18nc("Mobile Connection Wizard", "My country is not listed"));
    mCountryList->insertItems(1, mProviders->getCountryList());
    layout->addWidget(mCountryList);

    page->setLayout(layout);

    return page;
}

/**********************************************************/
/* Providers page */
/**********************************************************/

QWizardPage *MobileConnectionWizard::createProvidersPage()
{
    auto page = new QWizardPage();
    page->setTitle(i18nc("Mobile Connection Wizard", "Choose your Provider"));
    auto layout = new QVBoxLayout;

    radioAutoProvider = new QRadioButton(i18nc("Mobile Connection Wizard", "Select your provider from a &list:"));
    radioAutoProvider->setChecked(true);
    layout->addWidget(radioAutoProvider);

    mProvidersList = new QListWidget();
    connect(mProvidersList, &QListWidget::itemSelectionChanged, this, &MobileConnectionWizard::slotCheckProviderList);
    connect(mProvidersList, &QListWidget::itemClicked, this, &MobileConnectionWizard::slotCheckProviderList);
    layout->addWidget(mProvidersList);

    radioManualProvider = new QRadioButton(i18nc("Mobile Connection Wizard", "I cannot find my provider and I wish to enter it &manually:"));
    layout->addWidget(radioManualProvider);
    connect(radioManualProvider, &QRadioButton::toggled, this, &MobileConnectionWizard::slotEnableProviderEdit);

    lineEditProvider = new KLineEdit();
    layout->addWidget(lineEditProvider);
    connect(lineEditProvider, &KLineEdit::textEdited, this, &MobileConnectionWizard::slotCheckProviderEdit);

    page->setLayout(layout);

    return page;
}

void MobileConnectionWizard::slotEnableProviderEdit(bool checked)
{
    if (checked) {
        lineEditProvider->setFocus();
    } else {
        mProvidersList->setFocus();
    }
}

void MobileConnectionWizard::slotCheckProviderEdit()
{
    radioManualProvider->setChecked(true);
    // TODO: this does not work, try reimplementing QWizardPage::isComplete()
    // button(QWizard::NextButton)->setEnabled(true);
}

void MobileConnectionWizard::slotCheckProviderList()
{
    radioAutoProvider->setChecked(true);
    lineEditProvider->clear();
}

/**********************************************************/
/* Plan page */
/**********************************************************/

QWizardPage *MobileConnectionWizard::createPlansPage()
{
    auto page = new QWizardPage();
    page->setTitle(i18nc("Mobile Connection Wizard", "Choose your Billing Plan"));
    auto layout = new QBoxLayout(QBoxLayout::TopToBottom);

    auto label = new QLabel(i18nc("Mobile Connection Wizard", "&Select your plan:"));
    layout->addWidget(label);

    mPlanComboBox = new KComboBox();
    label->setBuddy(mPlanComboBox);
    layout->addWidget(mPlanComboBox);

    label = new QLabel('\n' + i18nc("Mobile Connection Wizard", "Selected plan &APN (Access Point Name):"));
    layout->addWidget(label);

    userApn = new KLineEdit();
    userApn->setEnabled(false);
    label->setBuddy(userApn);
    layout->addWidget(userApn);

    auto layout2 = new QHBoxLayout();
    label = new QLabel();
    label->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(32));
    layout2->addWidget(label, 0, Qt::AlignTop);
    label = new QLabel(i18nc("Mobile Connection Wizard",
                             "Warning: Selecting an incorrect plan may result in billing issues for your broadband account or may prevent connectivity.\n\nIf "
                             "you are unsure of your plan please ask your provider for your plan's APN."));
    label->setWordWrap(true);
    layout2->addWidget(label);
    layout->addWidget(new QLabel());
    layout->addLayout(layout2);

    page->setLayout(layout);

    return page;
}

void MobileConnectionWizard::slotEnablePlanEditBox(int index)
{
    const QString text = mPlanComboBox->itemText(index);
    if (type() != NetworkManager::ConnectionSettings::Gsm) {
        return;
    }
    if (text == i18nc("Mobile Connection Wizard", "My plan is not listed…")) {
        userApn->clear();
        userApn->setEnabled(true);
    } else {
        if (mProvidersList->currentItem() != nullptr) {
            int i = mPlanComboBox->currentIndex();
            if (i > 0)
                i = i - 1; // Skipping the separator (i==1)
            QStringList mApns = mProviders->getApns(mProvidersList->currentItem()->text());
            userApn->setText(mApns.at(i));
        }
        userApn->setEnabled(false);
    }
}

/**********************************************************/
/* Confirm page */
/**********************************************************/

QWizardPage *MobileConnectionWizard::createConfirmPage()
{
    auto page = new QWizardPage();
    page->setTitle(i18nc("Mobile Connection Wizard", "Confirm Mobile Broadband Settings"));
    auto layout = new QVBoxLayout;

    auto label = new QLabel(i18nc("Mobile Connection Wizard", "Your mobile broadband connection is configured with the following settings:"));
    label->setWordWrap(true);
    layout->addWidget(label);

    label = new QLabel('\n' + i18nc("Mobile Connection Wizard", "Your Provider:"));
    layout->addWidget(label);
    labelProvider = new QLabel();
    layout->addWidget(labelProvider);

    labelPlanLabel = new QLabel(QLatin1Char('\n') + i18nc("Mobile Connection Wizard", "Your Plan:"));
    layout->addWidget(labelPlanLabel);
    labelPlan = new QLabel();
    layout->addWidget(labelPlan);
    labelApn = new QLabel();
    labelApn->setEnabled(false);
    layout->addWidget(labelApn);

    page->setLayout(layout);

    return page;
}

#include "moc_mobileconnectionwizard.cpp"
