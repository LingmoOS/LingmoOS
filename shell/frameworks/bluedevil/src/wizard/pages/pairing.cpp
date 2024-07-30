/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "pairing.h"
#include "../bluewizard.h"
#include "../wizardagent.h"
#include "bluedevil_wizard.h"

#include <QPushButton>

#include <KLocalizedString>
#include <KStandardGuiItem>

#include <BluezQt/Adapter>
#include <BluezQt/PendingCall>

PairingPage::PairingPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
    , m_success(false)
{
    setupUi(this);

    QFont font(pinNumber->font());
    font.setPointSize(42);
    font.setBold(true);
    pinNumber->setFont(font);
}

int PairingPage::nextId() const
{
    if (m_success) {
        return BlueWizard::Connect;
    }
    return BlueWizard::Fail;
}

void PairingPage::initializePage()
{
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Initialize Pairing Page";

    m_device = m_wizard->device();
    m_wizard->setButtonLayout(wizardButtonsLayout());

    QPushButton *cancel = new QPushButton(this);
    KGuiItem::assign(cancel, KStandardGuiItem::cancel());
    connect(cancel, &QPushButton::clicked, this, &PairingPage::cancelClicked);
    wizard()->setButton(QWizard::CustomButton1, cancel);

    frame->hide();
    progressBar->show();
    connectingLbl->show();
    confirmLbl->clear();

    connectingLbl->setText(i18n("Connecting to %1…", m_device->name()));

    connect(m_wizard->agent(), &WizardAgent::pinRequested, this, &PairingPage::pinRequested);
    connect(m_wizard->agent(), &WizardAgent::confirmationRequested, this, &PairingPage::confirmationRequested);

    BluezQt::PendingCall *pairCall = m_device->pair();
    connect(pairCall, &BluezQt::PendingCall::finished, this, &PairingPage::pairingFinished);
}

void PairingPage::pairingFinished(BluezQt::PendingCall *call)
{
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Pairing finished:";
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "\t error     : " << (bool)call->error();
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "\t errorText : " << call->errorText();

    m_success = !call->error();
    if (m_device->isConnected()) {
        BluezQt::PendingCall *disconnectCall = m_device->disconnectFromDevice();
        connect(disconnectCall, &BluezQt::PendingCall::finished, m_wizard, &QWizard::next);
    } else {
        m_wizard->next();
    }
}

void PairingPage::pinRequested(const QString &pin)
{
    // Don't ask user to introduce the PIN if it was used from database
    if (m_wizard->agent()->isFromDatabase()) {
        return;
    }

    frame->show();
    connectingLbl->hide();
    progressBar->hide();

    if (m_device->type() == BluezQt::Device::Keyboard) {
        confirmLbl->setText(i18n("Please introduce the PIN in your keyboard when it appears and press Enter"));
    } else {
        confirmLbl->setText(i18n("Please introduce the PIN in your device when it appears"));
    }

    pinNumber->setText(pin);
}

void PairingPage::confirmationRequested(const QString &passkey, const BluezQt::Request<> &req)
{
    m_req = req;

    frame->show();
    connectingLbl->hide();
    progressBar->hide();

    QPushButton *matches = new QPushButton(this);
    KGuiItem::assign(matches, KStandardGuiItem::apply());
    matches->setText(i18n("Matches"));

    QPushButton *notMatch = new QPushButton(this);
    KGuiItem::assign(notMatch, KStandardGuiItem::cancel());
    notMatch->setText(i18n("Does not match"));

    connect(matches, &QPushButton::clicked, this, &PairingPage::matchesClicked);
    connect(notMatch, &QPushButton::clicked, this, &PairingPage::notMatchClicked);

    m_wizard->setButton(QWizard::CustomButton1, matches);
    m_wizard->setButton(QWizard::CustomButton2, notMatch);

    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton1;
    list << QWizard::CustomButton2;
    m_wizard->setButtonLayout(list);

    pinNumber->setText(passkey);
    confirmLbl->setText(i18n("Please, confirm that the PIN displayed on %1 matches the wizard one.", m_wizard->device()->name()));
}

void PairingPage::matchesClicked()
{
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);
    wizard()->button(QWizard::CustomButton2)->setEnabled(false);

    m_req.accept();
}

void PairingPage::notMatchClicked()
{
    m_req.reject();
}

void PairingPage::cancelClicked()
{
    m_device->cancelPairing();
}

QList<QWizard::WizardButton> PairingPage::wizardButtonsLayout() const
{
    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton1;
    return list;
}

#include "moc_pairing.cpp"
