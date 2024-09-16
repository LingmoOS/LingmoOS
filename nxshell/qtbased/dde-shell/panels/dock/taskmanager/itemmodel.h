// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QAbstractListModel>

namespace dock {
class AbstractItem;

class ItemModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        ItemIdRole = Qt::UserRole + 1,
        NameRole,
        IconNameRole,
        ActiveRole,
        AttentionRole,
        MenusRole,
        DockedRole,
        // data type
        WindowsRole,
        DesktopFilesIconsRole,
        DockedDirRole,
    };
    Q_ENUM(Roles)

    static ItemModel* instance();
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = ItemIdRole) const Q_DECL_OVERRIDE;
    Q_INVOKABLE void moveTo(const QString &id, int index);

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    QPointer<AbstractItem> getItemById(const QString& id) const;
    void addItem(QPointer<AbstractItem> item);
    QJsonArray dumpDockedItems() const;

private Q_SLOTS:
    void onItemDestroyed();
    void onItemChanged();

private:
    explicit ItemModel(QObject* parent = nullptr);

    int m_recentSize;
    QList<QPointer<AbstractItem>> m_items;
};
}
