/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "searchpresetmodel.h"

#include "knewstuffquick_debug.h"

#include <KLocalizedString>

SearchPresetModel::SearchPresetModel(KNSCore::EngineBase *engine)
    : QAbstractListModel(engine)
    , m_engine(engine)
{
    connect(m_engine, &KNSCore::EngineBase::signalSearchPresetsLoaded, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

SearchPresetModel::~SearchPresetModel() = default;

QHash<int, QByteArray> SearchPresetModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{DisplayNameRole, "displayName"}, {IconRole, "iconName"}};
    return roles;
}

QVariant SearchPresetModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && checkIndex(index)) {
        const QList<KNSCore::Provider::SearchPreset> presets = m_engine->searchPresets();
        const KNSCore::Provider::SearchPreset preset = presets[index.row()];

        if (role == DisplayNameRole) {
            if (QString name = preset.displayName; !name.isEmpty()) {
                return name;
            }

            switch (preset.type) {
            case KNSCore::Provider::SearchPresetTypes::GoBack:
                return i18nc("Knewstuff5", "Back");
            case KNSCore::Provider::SearchPresetTypes::Popular:
                return i18nc("Knewstuff5", "Popular");
            case KNSCore::Provider::SearchPresetTypes::Featured:
                return i18nc("Knewstuff5", "Featured");
            case KNSCore::Provider::SearchPresetTypes::Start:
                return i18nc("Knewstuff5", "Restart");
            case KNSCore::Provider::SearchPresetTypes::New:
                return i18nc("Knewstuff5", "New");
            case KNSCore::Provider::SearchPresetTypes::Root:
                return i18nc("Knewstuff5", "Home");
            case KNSCore::Provider::SearchPresetTypes::Shelf:
                return i18nc("Knewstuff5", "Shelf");
            case KNSCore::Provider::SearchPresetTypes::FolderUp:
                return i18nc("Knewstuff5", "Up");
            case KNSCore::Provider::SearchPresetTypes::Recommended:
                return i18nc("Knewstuff5", "Recommended");
            case KNSCore::Provider::SearchPresetTypes::Subscription:
                return i18nc("Knewstuff5", "Subscriptions");
            case KNSCore::Provider::SearchPresetTypes::AllEntries:
                return i18nc("Knewstuff5", "All Entries");
            default:
                return i18nc("Knewstuff5", "Search Preset: %1", preset.request.searchTerm);
            }
        } else if (role == IconRole) {
            if (QString name = preset.iconName; !name.isEmpty()) {
                return name;
            }

            switch (preset.type) {
            case KNSCore::Provider::SearchPresetTypes::GoBack:
                return QStringLiteral("arrow-left");
            case KNSCore::Provider::SearchPresetTypes::Popular:
            case KNSCore::Provider::SearchPresetTypes::Featured:
            case KNSCore::Provider::SearchPresetTypes::Recommended:
                return QStringLiteral("rating");
            case KNSCore::Provider::SearchPresetTypes::New:
                return QStringLiteral("change-date-symbolic");
            case KNSCore::Provider::SearchPresetTypes::Start:
                return QStringLiteral("start-over");
            case KNSCore::Provider::SearchPresetTypes::Root:
                return QStringLiteral("go-home");
            case KNSCore::Provider::SearchPresetTypes::Shelf:
            case KNSCore::Provider::SearchPresetTypes::Subscription:
                return QStringLiteral("bookmark");
            case KNSCore::Provider::SearchPresetTypes::FolderUp:
                return QStringLiteral("arrow-up");
            default:
                return QStringLiteral("search");
            }
        }
    }
    return QVariant();
}

int SearchPresetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_engine->searchPresets().count();
}

void SearchPresetModel::loadSearch(const QModelIndex &index)
{
    if (index.row() >= rowCount() || !index.isValid()) {
        qCWarning(KNEWSTUFFQUICK) << "index SearchPresetModel::loadSearch invalid" << index;
        return;
    }
    const KNSCore::Provider::SearchPreset preset = m_engine->searchPresets().at(index.row());
    m_engine->search(preset.request);
}

#include "moc_searchpresetmodel.cpp"
