/*
    SPDX-FileCopyrightText: 2009 Andrey Batyiev <batyiev@gmail.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "editlistdialog.h"

#include <QDialogButtonBox>
#include <QLayout>

#include <KLineEdit>

EditListDialog::EditListDialog(QWidget *parent)
    : QDialog(parent)
    , editListWidget(new KEditListWidget(this))
{
    editListWidget->setCheckAtEntering(true);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, this, &EditListDialog::dialogAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(new QVBoxLayout);
    layout()->addWidget(editListWidget);
    layout()->addWidget(buttons);
}

EditListDialog::~EditListDialog() = default;

void EditListDialog::setItems(const QStringList &items)
{
    editListWidget->setItems(items);
}

QStringList EditListDialog::items() const
{
    return editListWidget->items();
}

void EditListDialog::removeEmptyItems(QStringList &list)
{
    QStringList::iterator it = list.begin();
    const QStringList::iterator end = list.end();
    while (it != end) {
        if ((*it).trimmed().isEmpty()) {
            it = list.erase(it);
        } else {
            ++it;
        }
    }
}

void EditListDialog::dialogAccepted()
{
    QStringList list = items();
    removeEmptyItems(list);
    Q_EMIT itemsEdited(list);
}

void EditListDialog::setValidator(const QValidator *validator)
{
    editListWidget->lineEdit()->setValidator(validator);
}

const QValidator *EditListDialog::validator() const
{
    return editListWidget->lineEdit()->validator();
}

void EditListDialog::setToolTip(const QString &toolTip)
{
    editListWidget->lineEdit()->setToolTip(toolTip);
}

#include "moc_editlistdialog.cpp"
