/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QAbstractListModel>
#include <QQmlParserStatus>

#include <qqmlregistration.h>

#include <KService>

class QDBusInterface;

/**
 * @todo write docs
 */
class ToolsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        IdRole = Qt::UserRole,
        NameRole,
        IconRole,
        ShortcutRole,
    };

    explicit ToolsModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void trigger(const QString &id);

private:
    void addFromService(const QString &serviceName);

    struct Entry {
        QString id;
        QString name;
        QString icon;
        QString shortcut;
        KService::Ptr service;
    };
    QList<Entry> m_entries;
};
