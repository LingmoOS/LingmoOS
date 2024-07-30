/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kserviceselectdlg.h"
#include "kservicelistwidget.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

#include <KLocalizedString>

QList<KPluginMetaData> allParts()
{
    return KPluginMetaData::findPlugins(QStringLiteral("kf6/parts"));
}

KPartSelectDlg::KPartSelectDlg(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QLatin1String("serviceSelectDlg"));
    setModal(true);
    setWindowTitle(i18n("Add Service"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(i18n("Select service:")));
    m_listbox = new QListWidget();
    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    const auto parts = allParts();
    for (const KPluginMetaData &part : parts) {
        m_listbox->addItem(new PluginListItem(part));
    }

    m_listbox->model()->sort(0);
    m_listbox->setMinimumHeight(350);
    m_listbox->setMinimumWidth(400);
    layout->addWidget(m_listbox);
    layout->addWidget(m_buttonBox);
    connect(m_listbox, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

KPartSelectDlg::~KPartSelectDlg()
{
}

KPluginMetaData KPartSelectDlg::chosenPart()
{
    int selIndex = m_listbox->currentRow();
    auto selItem = static_cast<PluginListItem *>(m_listbox->item(selIndex));
    return selItem->metaData;
}

#include "moc_kserviceselectdlg.cpp"
