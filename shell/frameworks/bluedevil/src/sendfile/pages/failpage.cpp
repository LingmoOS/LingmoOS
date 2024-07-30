/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "failpage.h"
#include "../sendfilewizard.h"
#include "bluedevil_sendfile.h"

#include <QPushButton>

#include <KLocalizedString>
#include <KPixmapSequenceOverlayPainter>
#include <KStandardGuiItem>

#include <BluezQt/Device>

FailPage::FailPage(SendFileWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
{
    setupUi(this);

    failIcon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-error")).pixmap(48));
}

void FailPage::initializePage()
{
    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "Initialize Fail Page";

    const QList<QWizard::WizardButton> list{QWizard::Stretch, QWizard::CancelButton};

    m_wizard->setButtonLayout(list);

    BluezQt::DevicePtr device = m_wizard->device();

    if (m_errorMessage.has_value()) {
        failLbl->setText(m_errorMessage.value());
    } else if (device->name().isEmpty()) {
        failLbl->setText(i18nc("This string is shown when the wizard fail", "The connection to the device has failed: %1", m_wizard->errorMessage()));
    } else {
        failLbl->setText(i18n("The connection to %1 has failed: %2", device->name(), m_wizard->errorMessage()));
    }
}

void FailPage::setErrorMessage(const QString &errorMessage)
{
    m_errorMessage = errorMessage;
}

#include "moc_failpage.cpp"
