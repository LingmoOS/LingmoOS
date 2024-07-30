/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "selectdeviceandfilespage.h"
#include "../discoverwidget.h"
#include "../sendfilewizard.h"

#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <BluezQt/Device>

SelectDeviceAndFilesPage::SelectDeviceAndFilesPage(SendFileWizard *wizard)
    : QWizardPage(wizard)
    , m_wizard(wizard)
{
    setupUi(this);

    DiscoverWidget *widget = new DiscoverWidget(m_wizard->manager(), this);
    widget->setContentsMargins(0, 0, 0, 0);
    discoverLayout->addWidget(widget);

    int buttonSize = selectBtn->sizeHint().height();
    selectBtn->setFixedSize(buttonSize, buttonSize);
    selectBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));

    connect(widget, &DiscoverWidget::deviceSelected, this, &SelectDeviceAndFilesPage::deviceSelected);
    connect(selectBtn, &QPushButton::clicked, this, &SelectDeviceAndFilesPage::openFileDialog);
}

void SelectDeviceAndFilesPage::deviceSelected(BluezQt::DevicePtr device)
{
    static_cast<SendFileWizard *>(wizard())->setDevice(device);

    Q_EMIT completeChanged();
}

void SelectDeviceAndFilesPage::openFileDialog()
{
    const QStringList &files = QFileDialog::getOpenFileNames(this, //
                                                             i18n("Open file…"),
                                                             QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                             QStringLiteral("*"));

    if (files.isEmpty()) {
        selectLbl->setText(i18n("Select one or more files:"));
    } else {
        QStringList fileNames;
        for (const QString &file : files) {
            QFileInfo info(file);
            fileNames.append(info.fileName());
        }
        selectLbl->setText(i18n("Selected files: <b>%1</b>", fileNames.join(QStringLiteral(", "))));
    }

    m_wizard->setFiles(files);

    Q_EMIT completeChanged();
}

bool SelectDeviceAndFilesPage::isComplete() const
{
    if (!m_wizard->device()) {
        return false;
    }

    if (m_wizard->files().isEmpty()) {
        return false;
    }

    return true;
}

#include "moc_selectdeviceandfilespage.cpp"
