/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "intdelegate.h"
#include <QIntValidator>

IntDelegate::IntDelegate(QObject *parent)
    : Delegate(parent)
    , m_boundary(false)
{
}
IntDelegate::IntDelegate(int min, int max, QObject *parent)
    : Delegate(parent)
    , m_min(min)
    , m_max(max)
    , m_boundary(true)
{
}
IntDelegate::~IntDelegate() = default;

QWidget *IntDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    auto editor = new QLineEdit(parent);
    if (m_boundary)
        editor->setValidator(new QIntValidator(m_min, m_max, editor));
    else
        editor->setValidator(new QIntValidator(editor));

    return editor;
}

#include "moc_intdelegate.cpp"
