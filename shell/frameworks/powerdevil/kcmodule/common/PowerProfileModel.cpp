/*
    SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "PowerProfileModel.h"

#include <powerdevilenums.h>

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

PowerProfileModel::PowerProfileModel(QObject *parent)
    : QAbstractListModel(parent)
{
    /* Loads the values for the Power Profile modes */
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Solid.PowerManagement"),
                                                      QStringLiteral("/org/kde/Solid/PowerManagement/Actions/PowerProfile"),
                                                      QStringLiteral("org.kde.Solid.PowerManagement.Actions.PowerProfile"),
                                                      QStringLiteral("profileChoices"));

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg), this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QStringList> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qWarning() << "Failed to query platform profile choices" << reply.error().message();
            return;
        }

        const QHash<QString, QString> profileNames = {
            {QStringLiteral("power-saver"), i18nc("@option:combobox Power profile", "Power Save")},
            {QStringLiteral("balanced"), i18nc("@option:combobox Power profile", "Balanced")},
            {QStringLiteral("performance"), i18nc("@option:combobox Power profile", "Performance")},
        };

        beginResetModel();
        m_data.clear();
        m_data.append(PowerProfileModel::Data{.name = i18n("Leave unchanged"), .value = QString()});

        for (const QString &choice : reply.value()) {
            m_data.append(PowerProfileModel::Data{
                .name = profileNames.value(choice, choice),
                .value = choice,
            });
        }
        endResetModel();
    });
}

QVariant PowerProfileModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_data.size()) {
        return {};
    }

    switch (role) {
    case Name:
        return m_data[index.row()].name;
    case Value:
        return m_data[index.row()].value;
    default:
        return {};
    }
}

int PowerProfileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_data.size());
}

QHash<int, QByteArray> PowerProfileModel::roleNames() const
{
    return QHash<int, QByteArray>{{Name, "name"}, {Value, "value"}};
}
