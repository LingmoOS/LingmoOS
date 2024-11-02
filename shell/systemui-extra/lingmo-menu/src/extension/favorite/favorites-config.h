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
 */

#ifndef LINGMO_MENU_FAVORITES_CONFIG_H
#define LINGMO_MENU_FAVORITES_CONFIG_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>

static const QString APP_ID_SCHEME = "app://";
static const QString FILE_ID_SCHEME = "file://";
static const QString FOLDER_ID_SCHEME = "folder://";

namespace LingmoUIMenu {

class FavoritesConfig : public QObject
{
    Q_OBJECT
public:
    static FavoritesConfig &instance();
    void changeOrder(const int &indexFrom, const int &indexTo);
    /*
     * 在配置文件中对应位置添加id
     * 对应类型： 应用组（folder://）、应用（app://)、文件（file://)
     */
    void insertValue(const QString &id, const int &index = 0);
    void removeValueById(const QString &id);
    void clear();
    int configSize() const;
    /*
     * 在配置文件中获取对应id的位置
     */
    int getOrderById(const QString &id);
    QString getValue(const int &index) const;

Q_SIGNALS:
    void configChanged();

private:
    explicit FavoritesConfig(QObject *parent = nullptr);
    void initConfig();
    void sync();

private:
    static QString s_favoritesConfigFile;
    QStringList m_favoritesList;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_FAVORITES_CONFIG_H
