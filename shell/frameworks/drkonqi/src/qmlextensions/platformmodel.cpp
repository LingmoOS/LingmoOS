// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2009, 2010, 2011 Dario Andres Rodriguez <andresbajotierra@gmail.com>
// SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>

#include "platformmodel.h"

#include <QMetaEnum>

#include <KLocalizedString>

#include "bugzillaintegration/bugzillalib.h"
#include "bugzillaintegration/libbugzilla/clients/bugfieldclient.h"
#include "drkonqi.h"
#include "drkonqi_debug.h"
#include "systeminformation.h"

void PlatformModel::setManager(BugzillaManager *manager)
{
    if (m_manager) {
        m_manager->disconnect(this);
    }
    m_manager = manager;

    beginResetModel();
    m_list.clear();
    m_list << QStringLiteral("unspecified"); // in case the loading fails, always offer unspecified.
    endResetModel();

    Bugzilla::BugFieldClient client;
    auto job = client.getField(QStringLiteral("rep_platform"));
    connect(job, &KJob::finished, this, [this, client](KJob *job) {
        try {
            Bugzilla::BugField::Ptr field = client.getField(job);
            if (!field) {
                // This is a bit flimsy but only acts as save guard.
                // Ideally this code path is never hit.
                throw Bugzilla::RuntimeException(i18nc("@info/status error", "Failed to get platform list"));
            }

            beginResetModel();
            m_list.clear();
            const QList<Bugzilla::BugFieldValue *> values = field->values();
            for (const Bugzilla::BugFieldValue *value : values) {
                m_list << value->name();
            }
            endResetModel();
            Q_EMIT detectedPlatformRowChanged();
        } catch (Bugzilla::Exception &e) {
            qCWarning(DRKONQI_LOG) << e.whatString();
            m_error = e.whatString();
            Q_EMIT errorChanged();
        }
    });

    Q_EMIT managerChanged();
}

int PlatformModel::detectedPlatformRow()
{
    const QString detectedPlatform = DrKonqi::systemInformation()->bugzillaPlatform();
    int index = m_list.indexOf(detectedPlatform);

    if (index < 0) { // failed to restore value
        index = m_list.indexOf(QStringLiteral("unspecified"));
    }
    if (index < 0) { // also failed to find unspecified... shouldn't happen
        index = 0;
    }

    return index;
}

int PlatformModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_list.size();
}

QVariant PlatformModel::data(const QModelIndex &index, int intRole) const
{
    if (!index.isValid()) {
        return {};
    }

    if (intRole <= Qt::UserRole) {
        switch (static_cast<Qt::ItemDataRole>(intRole)) {
        case Qt::DisplayRole:
            return m_list.at(index.row());
        default:
            return {};
        }
    }

    switch (static_cast<Role>(intRole)) {
    case Role::Name:
        return m_list.at(index.row());
    }

    return {};
}

QHash<int, QByteArray> PlatformModel::roleNames() const
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

#include "moc_platformmodel.cpp"
