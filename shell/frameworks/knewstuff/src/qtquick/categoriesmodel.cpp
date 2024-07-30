/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "categoriesmodel.h"

#include "provider.h"

#include <KLocalizedString>

CategoriesModel::CategoriesModel(KNSCore::EngineBase *parent)
    : QAbstractListModel(parent)
    , m_engine(parent)
{
    connect(m_engine, &KNSCore::EngineBase::signalCategoriesMetadataLoded, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

CategoriesModel::~CategoriesModel() = default;

QHash<int, QByteArray> CategoriesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{NameRole, "name"}, {IdRole, "id"}, {DisplayNameRole, "displayName"}};
    return roles;
}

int CategoriesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_engine->categoriesMetadata().count() + 1;
}

QVariant CategoriesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    const QList<KNSCore::Provider::CategoryMetadata> categoriesMetadata = m_engine->categoriesMetadata();
    if (index.row() == 0) {
        switch (role) {
        case NameRole:
            return QString();
        case IdRole:
            return 0;
        case DisplayNameRole:
            return i18nc("The first entry in the category selection list (also the default value)", "All Categories");
        default:
            return QStringLiteral("Unknown role");
        }
    } else if (index.row() <= categoriesMetadata.count()) {
        const KNSCore::Provider::CategoryMetadata category = categoriesMetadata[index.row() - 1];
        switch (role) {
        case NameRole:
            return category.name;
        case IdRole:
            return category.id;
        case DisplayNameRole:
            return category.displayName;
        default:
            return QStringLiteral("Unknown role");
        }
    }
    return QVariant();
}

QString CategoriesModel::idToDisplayName(const QString &id) const // TODO KF6 unused?
{
    QString dispName = i18nc("The string passed back in the case the requested category is not known", "Unknown Category");
    const auto metaData = m_engine->categoriesMetadata();
    for (const KNSCore::Provider::CategoryMetadata &cat : metaData) {
        if (cat.id == id) {
            dispName = cat.displayName;
            break;
        }
    }
    return dispName;
}

#include "moc_categoriesmodel.cpp"
