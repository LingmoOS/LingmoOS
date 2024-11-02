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

#include "app-category-plugin.h"
#include "combined-list-model.h"
#include "app-category-model.h"
#include "recently-installed-model.h"
#include "data-entity.h"
#include "event-track.h"

#include <QAction>
#include <QDebug>

namespace LingmoUIMenu {

AppCategoryPlugin::AppCategoryPlugin(QObject *parent) : AppListPluginInterface(parent)
    , m_dataModel(new CombinedListModel(this)), m_labelBottle(new LabelBottle(this))
{
    m_categoryModel = new AppCategoryModel(this);
    m_recentlyModel = new RecentlyInstalledModel(this);

    m_dataModel->insertSubModel(m_recentlyModel);
    m_dataModel->insertSubModel(m_categoryModel);

    auto categoryAction = new QAction(QIcon::fromTheme("applications-utilities-symbolic"), tr("Category"), this);
    auto firstLatterAction = new QAction(QIcon::fromTheme("lingmo-capslock-symbolic"), tr("Letter Sort"), this);

    categoryAction->setCheckable(true);
    firstLatterAction->setCheckable(true);

    connect(categoryAction, &QAction::triggered, this, [=] {
        m_categoryModel->setMode(AppCategoryModel::Category);
        categoryAction->setChecked(true);
        firstLatterAction->setChecked(false);
        setTitle(categoryAction->text());
        updateLabelBottle();

        QMap<QString, QVariant> map;
        map.insert(QStringLiteral("viewName"), QStringLiteral("category"));
        EventTrack::instance()->sendClickEvent("switch_app_view", "AppView", map);
    });
    connect(firstLatterAction, &QAction::triggered, this, [=] {
        m_categoryModel->setMode(AppCategoryModel::FirstLatter);
        categoryAction->setChecked(false);
        firstLatterAction->setChecked(true);
        setTitle(firstLatterAction->text());
        updateLabelBottle();

        QMap<QString, QVariant> map;
        map.insert(QStringLiteral("viewName"), QStringLiteral("letter"));
        EventTrack::instance()->sendClickEvent("switch_app_view", "AppView", map);
    });

    connect(m_categoryModel, &AppCategoryModel::rowsInserted, this, [=] {
        Q_EMIT labelChanged();
    });
    connect(m_categoryModel, &AppCategoryModel::rowsRemoved, this, [=] {
        Q_EMIT labelChanged();
    });
    connect(m_recentlyModel, &RecentlyInstalledModel::rowsInserted, this, [=] {
        Q_EMIT labelChanged();
    });
    connect(m_recentlyModel, &RecentlyInstalledModel::rowsRemoved, this, [=] {
        Q_EMIT labelChanged();
    });

    m_actions.append(categoryAction);
    m_actions.append(firstLatterAction);

    categoryAction->setChecked(true);
    setTitle(categoryAction->text());
    m_labelBottle->setColumn(2);
}

QString AppCategoryPlugin::name()
{
    return QStringLiteral("AppCategoryPlugin");
}

QList<QAction *> AppCategoryPlugin::actions()
{
    return m_actions;
}

QAbstractItemModel *AppCategoryPlugin::dataModel()
{
    return m_dataModel;
}

AppListPluginGroup::Group AppCategoryPlugin::group()
{
    return AppListPluginGroup::Display;
}

QString AppCategoryPlugin::title()
{
    return m_title;
}

void AppCategoryPlugin::setTitle(const QString &title)
{
    if (title == m_title) {
        return;
    }

    m_title = title;
    Q_EMIT titleChanged();
}

LabelBottle *AppCategoryPlugin::labelBottle()
{
    updateLabelBottle();
    return m_labelBottle;
}

void AppCategoryPlugin::updateLabelBottle()
{
    QList<LabelItem> labels;
    if (m_recentlyModel->rowCount() > 0) {
        labels << LabelItem(tr("Recently Installed"), "document-open-recent-symbolic", LabelItem::Icon);
    }

    QHash<QString, int> groups;
    int rowCount = m_categoryModel->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QString group = m_categoryModel->index(row, 0, QModelIndex()).data(DataEntity::Group).toString();
        if (groups.contains(group)) {
            continue;
        }

        groups.insert(group, 0);
        labels.append(LabelItem(group, group));
    }

    m_labelBottle->setLabels(labels);
    m_labelBottle->setColumn(m_categoryModel->mode() == AppCategoryModel::FirstLatter ? 5 : 2);
}

} // LingmoUIMenu
