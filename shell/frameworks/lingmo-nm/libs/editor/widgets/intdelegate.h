/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef INTDELEGATE_H
#define INTDELEGATE_H

#include <QLineEdit>
#include <QStandardItem>
#include <QStandardItemModel>

#include "delegate.h"

class IntDelegate : public Delegate
{
    Q_OBJECT
public:
    explicit IntDelegate(QObject *parent = nullptr);
    IntDelegate(int min, int max, QObject *parent = nullptr);
    ~IntDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_min = -1;
    int m_max = -1;
    bool m_boundary;
};

#endif
