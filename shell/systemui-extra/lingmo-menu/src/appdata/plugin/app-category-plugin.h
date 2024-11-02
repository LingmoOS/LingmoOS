/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef LINGMO_MENU_APP_CATEGORY_PLUGIN_H
#define LINGMO_MENU_APP_CATEGORY_PLUGIN_H

#include "data-provider-plugin-iface.h"

namespace LingmoUIMenu {

class AppCategoryPluginPrivate;

class AppCategoryPlugin : public DataProviderPluginIFace
{
    Q_OBJECT
public:
    AppCategoryPlugin();
    ~AppCategoryPlugin() override;

    int index() override;
    QString id() override;
    QString name() override;
    QString icon() override;
    QString title() override;
    PluginGroup::Group group() override;
    QVector<DataEntity> data() override;
    void forceUpdate() override;
    QVector<LabelItem> labels() override;

private Q_SLOTS:
    void onAppAdded(const QList<DataEntity>& apps);
    void onAppDeleted(const QStringList& idList);
    void onAppUpdated(const QList<DataEntity>& apps, bool totalUpdate);

private:
    void reLoadApps();
    void initCategories();
    // 解析Category，获取正确的index
    int parseAppCategory(const DataEntity &app);
    void addAppToCategoryList(const DataEntity &app);

    // 从私有数据类型生成数据
    void generateData(QVector<DataEntity> &data);
    inline void updateData();

private:
    QMutex m_mutex;
    QString m_otherLabelId;
    AppCategoryPluginPrivate *d{nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_CATEGORY_PLUGIN_H
