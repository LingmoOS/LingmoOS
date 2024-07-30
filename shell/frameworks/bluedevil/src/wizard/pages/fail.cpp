/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "fail.h"
#include "../bluewizard.h"
#include "bluedevil_wizard.h"

#include <QPushButton>

#include <KLocalizedString>
#include <KPixmapSequenceOverlayPainter>
#include <KStandardGuiItem>

#include <BluezQt/Device>

FailPage::FailPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
{
    setupUi(this);

    failIcon->setPixmap(QIcon::fromTheme(QStringLiteral("emblem-error")).pixmap(48));
}

void FailPage::initializePage()
{
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Initialize Fail Page";

    QPushButton *reset = new QPushButton(this);
    KGuiItem::assign(reset, KStandardGuiItem::reset());
    reset->setText(i18nc("Button offered when the wizard fail. This button will restart the wizard", "Restart the wizard"));
    connect(reset, &QPushButton::clicked, m_wizard, &QWizard::restart);

    m_wizard->setButton(QWizard::CustomButton3, reset);
    m_wizard->setButtonText(QWizard::CancelButton, i18nc("Button that closes the wizard", "Close"));

    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CustomButton3;
    list << QWizard::CancelButton;

    m_wizard->setButtonLayout(list);

    BluezQt::DevicePtr device = m_wizard->device();

    if (device->name().isEmpty()) {
        failLbl->setText(i18nc("This string is shown when the wizard fail", "The setup of the device has failed"));
    } else {
        failLbl->setText(i18n("The setup of %1 has failed", device->name()));
    }
}

#include "moc_fail.cpp"
