/*
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "SleepModeModel.h"

#include <powerdevilenums.h>
#include <powerdevilpowermanagement.h>

#include <KLocalizedString>

#include <type_traits> // std::is_same_v, std::underlying_type_t

SleepModeModel::SleepModeModel(QObject *parent, PowerDevil::PowerManagement *pm)
    : QAbstractListModel(parent)
{
    if (pm->canSuspend()) {
        m_data.append(Data{
            .name = i18nc("Suspend to RAM", "Standby"),
            .subtitle = i18nc("Subtitle description for 'Standby' sleep option", "Save session to memory"),
            .value = qToUnderlying(PowerDevil::SleepMode::SuspendToRam),
        });
    }

    if (pm->canHybridSuspend()) {
        m_data.append(Data{
            .name = i18n("Hybrid sleep"),
            .subtitle = i18nc("Subtitle description for 'Hybrid sleep' sleep option", "Save session to both memory and disk"),
            .value = qToUnderlying(PowerDevil::SleepMode::HybridSuspend),
        });
    }

    if (pm->canSuspendThenHibernate()) {
        m_data.append(Data{
            .name = i18n("Standby, then hibernate"),
            .subtitle = i18nc("Subtitle description for 'Standby, then hibernate' sleep option", "Switch to hibernation after a period of inactivity"),
            .value = qToUnderlying(PowerDevil::SleepMode::SuspendThenHibernate),
        });
    }
}

QVariant SleepModeModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_data.size()) {
        return {};
    }

    switch (role) {
    case Name:
        return m_data[index.row()].name;
    case Subtitle:
        return m_data[index.row()].subtitle;
    case Value:
        static_assert(std::is_same_v<decltype(Data::value), std::underlying_type_t<PowerDevil::SleepMode>>);
        return m_data[index.row()].value;
    default:
        return {};
    }
}

int SleepModeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_data.size());
}

QHash<int, QByteArray> SleepModeModel::roleNames() const
{
    return QHash<int, QByteArray>{{Name, "name"}, {Subtitle, "subtext"}, {Value, "value"}};
}
