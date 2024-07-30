/*
 *  SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "PowerButtonActionModel.h"

#include <powerdevilenums.h>
#include <powerdevilpowermanagement.h>

#include <KLocalizedString>

#include <type_traits> // std::is_same_v, std::underlying_type_t

using namespace Qt::StringLiterals;

PowerButtonActionModel::PowerButtonActionModel(QObject *parent, PowerDevil::PowerManagement *pm, std::initializer_list<PowerDevil::PowerButtonAction> actions)
    : QAbstractListModel(parent)
{
    for (PowerDevil::PowerButtonAction action : actions) {
        appendAction(action, pm);
    }
}

void PowerButtonActionModel::appendAction(PowerDevil::PowerButtonAction action, PowerDevil::PowerManagement *pm)
{
    switch (action) {
    case PowerDevil::PowerButtonAction::NoAction:
        m_data.append(Data{
            .name = i18n("Do nothing"),
            .iconName = u"dialog-cancel-symbolic"_s,
            .value = qToUnderlying(PowerDevil::PowerButtonAction::NoAction),
        });
        break;

    case PowerDevil::PowerButtonAction::Sleep:
        if (pm->canSuspend()) {
            m_data.append(Data{
                .name = i18nc("Suspend to RAM", "Sleep"),
                .iconName = u"system-suspend-symbolic"_s,
                .value = qToUnderlying(PowerDevil::PowerButtonAction::Sleep),
            });
        }
        break;

    case PowerDevil::PowerButtonAction::Hibernate:
        if (pm->canHibernate()) {
            m_data.append(Data{
                .name = i18n("Hibernate"),
                .iconName = u"system-suspend-hibernate-symbolic"_s,
                .value = qToUnderlying(PowerDevil::PowerButtonAction::Hibernate),
            });
        }
        break;

    case PowerDevil::PowerButtonAction::Shutdown:
        m_data.append(Data{
            .name = i18nc("Power down the computer", "Shut down"),
            .iconName = u"system-shutdown-symbolic"_s,
            .value = qToUnderlying(PowerDevil::PowerButtonAction::Shutdown),
        });
        break;

    case PowerDevil::PowerButtonAction::LockScreen:
        m_data.append(Data{
            .name = i18n("Lock screen"),
            .iconName = u"system-lock-screen-symbolic"_s,
            .value = qToUnderlying(PowerDevil::PowerButtonAction::LockScreen),
        });
        break;

    case PowerDevil::PowerButtonAction::PromptLogoutDialog:
        m_data.append(Data{
            .name = i18n("Show logout screen"),
            .iconName = u"system-log-out-symbolic"_s,
            .value = qToUnderlying(PowerDevil::PowerButtonAction::PromptLogoutDialog),
        });
        break;

    case PowerDevil::PowerButtonAction::TurnOffScreen:
        m_data.append(Data{
            .name = i18n("Turn off screen"),
            .iconName = u"preferences-desktop-screensaver-symbolic"_s,
            .value = qToUnderlying(PowerDevil::PowerButtonAction::TurnOffScreen),
        });
        break;

    case PowerDevil::PowerButtonAction::ToggleScreenOnOff:
        m_data.append(Data{
            .name = i18n("Toggle screen on/off"),
            .iconName = u"osd-shutd-screen-symbolic"_s,
            .value = qToUnderlying(PowerDevil::PowerButtonAction::TurnOffScreen),
        });
        break;
    }
}

QVariant PowerButtonActionModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_data.size()) {
        return {};
    }

    switch (role) {
    case Icon:
        return QIcon::fromTheme(m_data[index.row()].iconName);
    case Name:
        return m_data[index.row()].name;
    case Value:
        static_assert(std::is_same_v<decltype(Data::value), std::underlying_type_t<PowerDevil::PowerButtonAction>>);
        return m_data[index.row()].value;
    case IconName:
        return m_data[index.row()].iconName;
    default:
        return {};
    }
}

int PowerButtonActionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_data.size());
}

QHash<int, QByteArray> PowerButtonActionModel::roleNames() const
{
    return QHash<int, QByteArray>{{Icon, "icon"_ba}, {Name, "name"_ba}, {Value, "value"_ba}, {IconName, "iconName"_ba}};
}
