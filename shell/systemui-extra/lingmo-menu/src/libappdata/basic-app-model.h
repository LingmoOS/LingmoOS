/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_MENU_BASIC_APP_MODEL_H
#define LINGMO_MENU_BASIC_APP_MODEL_H

#include <QAbstractListModel>
#include <QVector>

#include "data-entity.h"
#include "app-database-interface.h"

namespace LingmoUIMenu {

class BasicAppModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static BasicAppModel *instance();

    /**
     * 数据库访问接口
     * @return @AppDatabaseInterface *
     */
    const AppDatabaseInterface *databaseInterface() const;
    DataEntity appOfIndex(int row) const;
    int indexOfApp(const QString &appid) const;
    bool getAppById(const QString &appid, DataEntity &app) const;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
    void onAppAdded(const LingmoUIMenu::DataEntityVector &apps);
    void onAppUpdated(const QVector<QPair<LingmoUIMenu::DataEntity, QVector<int> > > &updates);
    void onAppDeleted(const QStringList &apps);

private:
    explicit BasicAppModel(QObject *parent = nullptr);
    AppDatabaseInterface *m_databaseInterface {nullptr};

    QVector<DataEntity> m_apps;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_BASIC_APP_MODEL_H
