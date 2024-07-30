// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "reproducibilitymodel.h"

#include <QMetaEnum>

#include <KLocalizedString>

QList<ReportInterface::Reproducible> ReproducibilityModel::reproducibilities()
{
    QList<ReportInterface::Reproducible> list;
    auto roleEnum = QMetaEnum::fromType<ReportInterface::Reproducible>();
    for (int i = 0; i < roleEnum.keyCount(); ++i) {
        const int value = roleEnum.value(i);
        Q_ASSERT(value != -1);
        list.append(static_cast<ReportInterface::Reproducible>(value));
    }
    return list;
}

int ReproducibilityModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.size();
}

QVariant ReproducibilityModel::data(const QModelIndex &index, int intRole) const
{
    if (!index.isValid()) {
        return {};
    }

    switch (static_cast<Role>(intRole)) {
    case Role::String:
        switch (m_list.at(index.row())) {
        case ReportInterface::ReproducibleUnsure:
            return i18nc("@item:inlistbox  user didn't tried to repeat the crash situation", "I did not try again");
        case ReportInterface::ReproducibleNever:
            return i18nc("@item:inlistbox the crash cannot be reproduce. reproduciblity->never", "Never");
        case ReportInterface::ReproducibleSometimes:
            return i18nc("@item:inlistbox the bug can be reproduced sometimes", "Sometimes");
        case ReportInterface::ReproducibleEverytime:
            return i18nc("@item:inlistbox the bug can be reproduced every time", "Every time");
        }
        return {};
    case Role::Integer:
        return m_list.at(index.row());
    }

    return {};
}

QHash<int, QByteArray> ReproducibilityModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (!roles.isEmpty()) {
        return roles;
    }

    const QMetaEnum roleEnum = QMetaEnum::fromType<Role>();
    for (int i = 0; i < roleEnum.keyCount(); ++i) {
        const int value = roleEnum.value(i);
        Q_ASSERT(value != -1);
        roles[static_cast<int>(value)] = QByteArray("ROLE_") + roleEnum.valueToKey(value);
    }
    return roles;
}

#include "moc_reproducibilitymodel.cpp"
