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

#include "app-category-plugin.h"
#include "app-data-manager.h"

namespace LingmoUIMenu {

// 临时使用，可能会删除
class CategoryPair
{
public:
    CategoryPair() = default;
    CategoryPair(QString id_t, QString localName_t) : id(std::move(id_t)), localName(std::move(localName_t)) {}
    QString id;
    QString localName;
};

class CategoryItem
{
public:
    LabelItem label;
    QList<DataEntity> apps;
};

class AppCategoryPluginPrivate
{
public:
    QHash<QString, int> categoryIndex;
    QList<CategoryItem> categoryData;
};

AppCategoryPlugin::AppCategoryPlugin() : d(new AppCategoryPluginPrivate)
{
    initCategories();
    reLoadApps();

    connect(AppDataManager::instance(), &AppDataManager::appAdded, this, &AppCategoryPlugin::onAppAdded);
    connect(AppDataManager::instance(), &AppDataManager::appDeleted, this, &AppCategoryPlugin::onAppDeleted);
    connect(AppDataManager::instance(), &AppDataManager::appUpdated, this, &AppCategoryPlugin::onAppUpdated);
}

AppCategoryPlugin::~AppCategoryPlugin()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

int AppCategoryPlugin::index()
{
    return 1;
}

QString AppCategoryPlugin::id()
{
    return "category";
}

QString AppCategoryPlugin::name()
{
    return tr("Category");
}

QString AppCategoryPlugin::icon()
{
    return "image://appicon/applications-utilities-symbolic";
}

QString AppCategoryPlugin::title()
{
    return tr("Category");
}

PluginGroup::Group AppCategoryPlugin::group()
{
    return PluginGroup::SortMenuItem;
}

QVector<DataEntity> AppCategoryPlugin::data()
{
    QVector<DataEntity> data;
    generateData(data);
    return data;
}

void AppCategoryPlugin::forceUpdate()
{

}

QVector<LabelItem> AppCategoryPlugin::labels()
{
    QMutexLocker locker(&m_mutex);
    QVector<LabelItem> labels;
    for (const auto &item : d->categoryData) {
//        qDebug() << "==label==" << item.label.id() << item.label.displayName();
        labels.append(item.label);
    }

    return labels;
}

// see: https://specifications.freedesktop.org/menu-spec/latest/apa.html
// see: https://specifications.freedesktop.org/menu-spec/latest/apas02.html
// TODO: 暂时只列出freedesktop规范中的常用分类，需要继续支持显示应用自定义的分类进行显示
void AppCategoryPlugin::initCategories()
{
    QList<CategoryPair> primaryCategories;
    primaryCategories.append({"Audio", tr("Audio")});
    primaryCategories.append({"AudioVideo", tr("AudioVideo")});
    primaryCategories.append({"Development", tr("Development")});
    primaryCategories.append({"Education", tr("Education")});
    primaryCategories.append({"Game", tr("Game")});
    primaryCategories.append({"Graphics", tr("Graphics")});
    primaryCategories.append({"Network", tr("Network")});
    primaryCategories.append({"Office", tr("Office")});
    primaryCategories.append({"Science", tr("Science")});
    primaryCategories.append({"Settings", tr("Settings")});
    primaryCategories.append({"System", tr("System")});
    primaryCategories.append({"Utility", tr("Utility")});
    primaryCategories.append({"Video", tr("Video")});

    // 未定义Category的应用类别
    m_otherLabelId = "Other";
    primaryCategories.append({m_otherLabelId, tr("Other")});

    for (const auto &category : primaryCategories) {
        // 默认拷贝
        CategoryItem categoryItem;
        categoryItem.label.setId(category.id);
        categoryItem.label.setDisplayName(category.localName);
//        categoryItem.label.setDisable(true);
        categoryItem.label.setIndex(d->categoryData.size());

        d->categoryData.append(categoryItem);
        d->categoryIndex.insert(category.id, categoryItem.label.index());
    }
}

int AppCategoryPlugin::parseAppCategory(const DataEntity &app)
{
//    qDebug() << "=parseAppCategory=" << app.name();
    // TODO: 某些应用的Category字段不是使用;进行分割的，是否需要支持？
    QStringList categories = app.category().split(";");
    for (const auto &category : categories) {
        int index = d->categoryIndex.value(category, -1);
        if (index != -1) {
            return index;
        }
    }

//    qDebug() << "Other index:" << d->categoryIndex.value(m_otherLabelId) << "==" << m_otherLabelId;
    return d->categoryIndex.value(m_otherLabelId);
}

void AppCategoryPlugin::addAppToCategoryList(const DataEntity &app)
{
    CategoryItem &categoryItem = d->categoryData[parseAppCategory(app)];
    categoryItem.label.setDisable(false);
    categoryItem.apps.append(app);
}

void AppCategoryPlugin::reLoadApps()
{
    QMutexLocker locker(&m_mutex);

    for (auto &item : d->categoryData) {
        item.label.setDisable(true);
        item.apps.clear();
    }

    QList<DataEntity> apps = AppDataManager::instance()->normalApps();
    for (const auto &app : apps) {
        addAppToCategoryList(app);
    }
}

void AppCategoryPlugin::generateData(QVector<DataEntity> &data)
{
    QMutexLocker locker(&m_mutex);
    for (const auto &item : d->categoryData) {
        // TODO: 显示不包含应用的分类？
        if (item.apps.isEmpty()) {
            continue;
        }

        DataEntity label;
        label.setComment(tr("Open Function Sort Menu"));
        label.setType(DataType::Label);
        label.setId(item.label.id());
        label.setName(item.label.displayName());

        data.append(label);
        // TODO: 性能优化
//        data.append(item.apps.toVector());
        for (const auto &app : item.apps) {
            data.append(app);
        }
    }
}

void AppCategoryPlugin::updateData()
{
    Q_EMIT dataChanged(data());
}

void AppCategoryPlugin::onAppAdded(const QList<DataEntity>& apps)
{
    {
        QMutexLocker locker(&m_mutex);
        for (const auto &app : apps) {
            addAppToCategoryList(app);
        }
    }
    updateData();
}

void AppCategoryPlugin::onAppDeleted(const QStringList& idList)
{
    reLoadApps();
    updateData();
    Q_EMIT labelChanged();
}

void AppCategoryPlugin::onAppUpdated(const QList<DataEntity> &apps, bool totalUpdate)
{
    if (totalUpdate) {
        reLoadApps();
        updateData();
    } else {
        Q_EMIT dataChanged(apps.toVector(), DataUpdateMode::Update);
    }
}

} // LingmoUIMenu
