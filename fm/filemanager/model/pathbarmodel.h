/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PATHBARMODEL_H
#define PATHBARMODEL_H

#include <QAbstractItemModel>
#include <QUrl>

struct PathBarItem {
    QString name;
    QUrl url;
};

class PathBarModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

public:
    enum DataRole {
        NameRole = Qt::UserRole + 1,
        UrlRole,
        PathRole
    };
    Q_ENUMS(DataRole);

    explicit PathBarModel(QObject *parent = nullptr);
    ~PathBarModel();

    QString url() const;
    void setUrl(const QString &url);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

signals:
    void urlChanged();

private:
    QString m_url;
    QList<PathBarItem *> m_pathList;
};

#endif // PATHBARMODEL_H
