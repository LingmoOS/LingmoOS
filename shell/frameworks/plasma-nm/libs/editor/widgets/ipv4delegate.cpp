/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ipv4delegate.h"

#include <QLineEdit>

#include "simpleipv4addressvalidator.h"

IpV4Delegate::IpV4Delegate(QObject *parent)
    : Delegate(parent)
{
}
IpV4Delegate::~IpV4Delegate() = default;

QWidget *IpV4Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    auto editor = new QLineEdit(parent);
    editor->setValidator(new SimpleIpV4AddressValidator(SimpleIpV4AddressValidator::Base, editor));

    return editor;
}

#include "moc_ipv4delegate.cpp"
