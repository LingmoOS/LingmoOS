/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <qnamespace.h>

class KDirWatch;

struct ProfileData {
    QString name;
    QString profileIdentifier;
    QString iconName;
};

class ProfilesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName WRITE setAppName NOTIFY appNameChanged)

public:
    explicit ProfilesModel(QObject *parent = nullptr);

    enum Roles {
        NameRole = Qt::DisplayRole,
        ProfileIdentifierRole = Qt::UserRole,
        IconNameRole = Qt::DecorationRole,
    };

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Q_INVOKABLE void openProfile(const QString profileIdentifier);

    int rowCount(const QModelIndex & /*parent*/ = QModelIndex()) const override
    {
        return m_data.size();
    }

    QString appName() const
    {
        return m_appName;
    }

    void setAppName(const QString &name)
    {
        if (m_appName != name) {
            m_appName = name;
            init();
            Q_EMIT appNameChanged();
        }
    }

Q_SIGNALS:
    void appNameChanged();

private:
    void init();
    void loadProfiles();

    QString m_appName;
    KDirWatch *m_dirWatch = nullptr;
    QList<ProfileData> m_data;
};
