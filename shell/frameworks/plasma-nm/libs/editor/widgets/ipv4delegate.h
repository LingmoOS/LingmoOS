/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IPV4DELEGATE_H
#define IPV4DELEGATE_H

#include "delegate.h"
#include <QWidget>

class IpV4Delegate : public Delegate
{
    Q_OBJECT
public:
    explicit IpV4Delegate(QObject *parent = nullptr);
    ~IpV4Delegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
