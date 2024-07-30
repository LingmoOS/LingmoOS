/*
 *  SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <powerdevilenums.h>

#include <QAbstractListModel>
#include <QIcon>
#include <QList>
#include <QString>

#include "powerdevilconfigcommonprivate_export.h"

namespace PowerDevil
{
class PowerManagement;
}

class POWERDEVILCONFIGCOMMONPRIVATE_EXPORT PowerButtonActionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum { Icon = Qt::DecorationRole, Name = Qt::DisplayRole, Value = Qt::UserRole, IconName = Qt::UserRole + 1 };

    PowerButtonActionModel(QObject *parent,
                           PowerDevil::PowerManagement *pm,
                           std::initializer_list<PowerDevil::PowerButtonAction> actions = {
                               PowerDevil::PowerButtonAction::NoAction,
                               PowerDevil::PowerButtonAction::Sleep,
                               PowerDevil::PowerButtonAction::Hibernate,
                               PowerDevil::PowerButtonAction::Shutdown,
                               PowerDevil::PowerButtonAction::PromptLogoutDialog,
                               PowerDevil::PowerButtonAction::LockScreen,
                               PowerDevil::PowerButtonAction::TurnOffScreen,
                               PowerDevil::PowerButtonAction::ToggleScreenOnOff,
                           });

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void appendAction(PowerDevil::PowerButtonAction, PowerDevil::PowerManagement *pm);

private:
    struct Data {
        QString name;
        QString iconName;
        uint value;
    };

    QList<Data> m_data;
};
