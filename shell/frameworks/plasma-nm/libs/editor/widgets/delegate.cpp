/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "delegate.h"

#include <QLineEdit>
#include <QStandardItem>
#include <QStandardItemModel>

Delegate::Delegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
Delegate::~Delegate() = default;

QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    auto editor = new QLineEdit(parent);

    return editor;
}

void Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    auto le = static_cast<QLineEdit *>(editor);
    le->setText(value);
}

void Delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto le = static_cast<QLineEdit *>(editor);

    model->setData(index, le->text(), Qt::EditRole);
}

void Delegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

#include "moc_delegate.cpp"
