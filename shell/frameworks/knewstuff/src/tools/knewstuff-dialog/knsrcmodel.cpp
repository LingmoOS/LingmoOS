/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knsrcmodel.h"

#include "enginebase.h"

#include <KConfig>
#include <KConfigGroup>
#include <QDir>

KNSRCModel::KNSRCModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QStringList files = KNSCore::EngineBase::availableConfigFiles();
    for (const auto &file : files) {
        KConfig conf(file);
        KConfigGroup group;
        if (conf.hasGroup(QStringLiteral("KNewStuff3"))) {
            group = conf.group(QStringLiteral("KNewStuff3"));
        } else if (conf.hasGroup(QStringLiteral("KNewStuff"))) {
            group = conf.group(QStringLiteral("KNewStuff"));
        } else {
            qWarning() << file << "doesn't contain a KNewStuff (or KNewStuff3) section.";
            continue;
        }

        QString constructedName{QFileInfo(file).fileName()};
        constructedName = constructedName.left(constructedName.length() - 6);
        constructedName.replace(QLatin1Char{'_'}, QLatin1Char{' '});
        constructedName[0] = constructedName[0].toUpper();

        Entry *entry = new Entry;
        entry->name = group.readEntry("Name", constructedName);
        entry->filePath = file;

        m_entries << entry;
    }
    std::sort(m_entries.begin(), m_entries.end(), [](const Entry *a, const Entry *b) -> bool {
        return QString::localeAwareCompare(b->name, a->name) > 0;
    });
}

KNSRCModel::~KNSRCModel() = default;

QHash<int, QByteArray> KNSRCModel::roleNames() const
{
    static const QHash<int, QByteArray> roleNames{{NameRole, "name"}, {FilePathRole, "filePath"}};
    return roleNames;
}

int KNSRCModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.count();
}

QVariant KNSRCModel::data(const QModelIndex &index, int role) const
{
    if (checkIndex(index)) {
        Entry *entry = m_entries[index.row()];
        switch (role) {
        case NameRole:
            return entry->name;
        case FilePathRole:
            return entry->filePath;
        }
    }
    return QVariant();
}

#include "moc_knsrcmodel.cpp"
