/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "selectfilespage.h"
#include "../sendfilewizard.h"

#include <KDirOperator>
#include <KFileItem>
#include <KFileWidget>
#include <KLocalizedString>

#include <QStandardPaths>
#include <QVBoxLayout>

#include <BluezQt/Device>

SelectFilesPage::SelectFilesPage(QWidget *parent)
    : QWizardPage(parent)
{
    m_files = new KFileWidget(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)), this);
    m_files->setMode(KFile::Files);
    m_files->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);

    connect(m_files, &KFileWidget::selectionChanged, this, &SelectFilesPage::selectionChanged);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_files);
    layout->setContentsMargins(0, 0, 0, 0);
}

void SelectFilesPage::initializePage()
{
    SendFileWizard *w = static_cast<SendFileWizard *>(wizard());

    const QSize sizeHint = m_files->dialogSizeHint();
    if (sizeHint.isValid())
        w->resize(sizeHint);

    w->setWindowTitle(i18nc("Send files to a Bluetooth device", "Send to %1", w->device()->name()));
}

void SelectFilesPage::selectionChanged()
{
    QStringList fileList;
    const KFileItemList itemList = m_files->dirOperator()->selectedItems();

    for (const KFileItem &file : itemList) {
        fileList << file.localPath();
    }

    static_cast<SendFileWizard *>(wizard())->setFiles(fileList);
    Q_EMIT completeChanged();
}

bool SelectFilesPage::isComplete() const
{
    return !m_files->dirOperator()->selectedItems().isEmpty();
}

#include "moc_selectfilespage.cpp"
