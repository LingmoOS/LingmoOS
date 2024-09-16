// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLUGINLISTMODEL_H_
#define PLUGINLISTMODEL_H_

#include <qvariant.h>

#include <QAbstractListModel>
#include <QList>
#include <QMutex>

class PluginListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PluginListModel(QObject *parent = nullptr);
    ~PluginListModel() override;

    enum PluginRoles { PathRole = Qt::UserRole + 1 };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QStringList items;
};

#endif // PLUGINLISTMODEL_H_
