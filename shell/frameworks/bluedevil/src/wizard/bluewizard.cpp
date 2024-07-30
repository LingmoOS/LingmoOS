/*
    SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "bluewizard.h"
#include "bluedevil_wizard.h"
#include "pages/connect.h"
#include "pages/discover.h"
#include "pages/fail.h"
#include "pages/pairing.h"
#include "pages/success.h"
#include "wizardagent.h"

#include <QApplication>
#include <QDBusConnection>
#include <QProcess>
#include <QPushButton>
#include <QString>

#include <KLocalizedString>
#include <KStandardGuiItem>

#include <BluezQt/InitManagerJob>

BlueWizard::BlueWizard()
    : QWizard()
    , m_agent(new WizardAgent(this))
{
    setOption(QWizard::IndependentPages, true);

    setPage(Discover, new DiscoverPage(this));
    setPage(Connect, new ConnectPage(this));
    setPage(Pairing, new PairingPage(this));
    setPage(Success, new SuccessPage(this));
    setPage(Fail, new FailPage(this));

    QPushButton *backButton = new QPushButton(this);
    KGuiItem::assign(backButton, KStandardGuiItem::back(KStandardGuiItem::UseRTL));

    QPushButton *forwardButton = new QPushButton(this);
    KGuiItem::assign(forwardButton, KStandardGuiItem::forward(KStandardGuiItem::UseRTL));

    QPushButton *applyButton = new QPushButton(this);
    KGuiItem::assign(applyButton, KStandardGuiItem::apply());

    QPushButton *cancelButton = new QPushButton(this);
    KGuiItem::assign(cancelButton, KStandardGuiItem::cancel());

    setButton(QWizard::BackButton, backButton);
    setButton(QWizard::NextButton, forwardButton);
    setButton(QWizard::FinishButton, applyButton);
    setButton(QWizard::CancelButton, cancelButton);

    // We do not want "Forward" as text
    setButtonText(QWizard::NextButton, i18nc("Action to go to the next page on the wizard", "Next"));
    setButtonText(QWizard::FinishButton, i18nc("Action to finish the wizard", "Finish"));

    // Initialize BluezQt
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &BluezQt::InitManagerJob::result, this, &BlueWizard::initJobResult);

    // When Finished page is opened, close wizard automatically
    const auto onCurrentIdChanged = [this](int id) {
        if (id == Success) {
            done(QDialog::Accepted);
        }
        // Sending notification in SuccessPage is asynchronous, so this needs to be queued.
    };
    connect(this, &QWizard::currentIdChanged, this, onCurrentIdChanged, Qt::QueuedConnection);
}

BluezQt::DevicePtr BlueWizard::device() const
{
    return m_device;
}

void BlueWizard::setDevice(BluezQt::DevicePtr device)
{
    m_device = device;
}

WizardAgent *BlueWizard::agent() const
{
    return m_agent;
}

BluezQt::Manager *BlueWizard::manager() const
{
    return m_manager;
}

void BlueWizard::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(BLUEDEVIL_WIZARD_LOG) << "Error initializing manager:" << job->errorText();
        qApp->exit(1);
        return;
    }

    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Manager initialized";

    // Make sure to register agent when bluetoothd starts
    operationalChanged(m_manager->isOperational());
    connect(m_manager, &BluezQt::Manager::operationalChanged, this, &BlueWizard::operationalChanged);

    // Only show wizard after init is completed
    show();
}

void BlueWizard::operationalChanged(bool operational)
{
    if (operational) {
        m_manager->registerAgent(m_agent);
    } else {
        // Attempt to start bluetoothd
        BluezQt::Manager::startService();
    }
}

void BlueWizard::done(int result)
{
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Wizard done:" << result;

    QWizard::done(result);
    qApp->exit(result);
}

#include "moc_bluewizard.cpp"
