/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ipv6delegate.h"

#include <QLineEdit>

#include "simpleipv6addressvalidator.h"

IpV6Delegate::IpV6Delegate(QObject *parent)
    : Delegate(parent)
{
}
IpV6Delegate::~IpV6Delegate() = default;

QWidget *IpV6Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    auto editor = new QLineEdit(parent);
    editor->setValidator(new SimpleIpV6AddressValidator(SimpleIpV6AddressValidator::Base, editor));

    return editor;
}

#include "moc_ipv6delegate.cpp"
