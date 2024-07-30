/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connectioneditordialog.h"

#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>

ConnectionEditorDialog::ConnectionEditorDialog(const NetworkManager::ConnectionSettings::Ptr &connection, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , m_buttonBox(new QDialogButtonBox(this))
    , m_connectionEditorTabWidget(new ConnectionEditorTabWidget(connection, parent, f))
{
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_connectionEditorTabWidget);

    m_buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    m_buttonBox->button(QDialogButtonBox::Save)->setEnabled(m_connectionEditorTabWidget->isValid());
    layout->addWidget(m_buttonBox);

    setLayout(layout);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &ConnectionEditorDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ConnectionEditorDialog::reject);
    connect(m_connectionEditorTabWidget, &ConnectionEditorTabWidget::validityChanged, this, &ConnectionEditorDialog::onValidityChanged);

    if (connection->id().isEmpty()) {
        setWindowTitle(i18n("New Connection (%1)", connection->typeAsString(connection->connectionType())));
    } else {
        setWindowTitle(i18n("Edit Connection '%1'", connection->id()));
    }
}

ConnectionEditorDialog::~ConnectionEditorDialog() = default;

NMVariantMapMap ConnectionEditorDialog::setting() const
{
    return m_connectionEditorTabWidget->setting();
}

void ConnectionEditorDialog::onValidityChanged(bool valid)
{
    m_buttonBox->button(QDialogButtonBox::Save)->setEnabled(valid);
}

#include "moc_connectioneditordialog.cpp"
