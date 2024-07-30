/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configstorage.h"

#include <QMetaEnum>

#include <KSharedConfig>

CalendarSystemModel::CalendarSystemModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QMetaEnum e = QMetaEnum::fromType<CalendarSystem::System>();

    beginInsertRows(QModelIndex(), 0, e.keyCount() - 2 /* Gregorian */);

    m_items.reserve(e.keyCount() - 1);
    for (int k = 0; k < e.keyCount(); k++) {
        const auto system = static_cast<CalendarSystem::System>(e.value(k));

        auto it = std::find_if(s_calendarMap.cbegin(), s_calendarMap.cend(), [system](const std::pair<QString, CalendarSystemItem> &pr) {
            return pr.second.system == system;
        });

        if (it != s_calendarMap.cend()) {
            m_items.emplace_back(it->second);
        }
    }

    endInsertRows();
}

QVariant CalendarSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const auto &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return item.text;
    case IdRole:
        return item.id;
    default:
        return QVariant();
    }
}

int CalendarSystemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QHash<int, QByteArray> CalendarSystemModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {IdRole, QByteArrayLiteral("id")},
    };
}

int CalendarSystemModel::indexOf(const QString &id) const
{
    const auto it = std::find_if(m_items.cbegin(), m_items.cend(), [&id](const CalendarSystemItem &item) {
        return item.id == id;
    });

    if (it != m_items.cend()) {
        return std::distance(m_items.cbegin(), it);
    }

    return 0;
}

ConfigStorage::ConfigStorage(QObject *parent)
    : QObject(parent)
    , m_calendarSystemModel(new CalendarSystemModel(this))
{
    auto config = KSharedConfig::openConfig(QStringLiteral("plasma_calendar_alternatecalendar"));
    m_generalConfigGroup = config->group("General");

    m_calendarSystem = m_generalConfigGroup.readEntry("calendarSystem", "Gregorian");
    m_dateOffset = m_generalConfigGroup.readEntry("dateOffset", 0);
}

CalendarSystemModel *ConfigStorage::calendarSystemModel() const
{
    return m_calendarSystemModel;
}

int ConfigStorage::currentIndex() const
{
    return m_calendarSystemModel->indexOf(m_calendarSystem);
}

void ConfigStorage::save()
{
    m_generalConfigGroup.writeEntry("calendarSystem", m_calendarSystem, KConfigBase::Notify);
    m_generalConfigGroup.writeEntry("dateOffset", m_dateOffset, KConfigBase::Notify);

    m_generalConfigGroup.sync();
}
