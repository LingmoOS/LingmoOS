// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "passwordsortproxymodel.h"
#include "passwordsmodel.h"

#include <QDebug>

using namespace PlasmaPass;

PasswordSortProxyModel::PasswordSortProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    sort(0); // enable sorting
}

bool PasswordSortProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto typeLeft = static_cast<PasswordsModel::EntryType>(source_left.data(PasswordsModel::EntryTypeRole).toInt());
    const auto typeRight = static_cast<PasswordsModel::EntryType>(source_right.data(PasswordsModel::EntryTypeRole).toInt());

    // Folders first
    if (typeLeft != typeRight) {
        return typeLeft == PasswordsModel::FolderEntry;
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

#include "moc_passwordsortproxymodel.cpp"
