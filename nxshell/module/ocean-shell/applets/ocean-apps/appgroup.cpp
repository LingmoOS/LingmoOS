// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appgroup.h"
#include "appgroupmanager.h"
#include "appitemmodel.h"
#include "itemspage.h"

#include <QLoggingCategory>
#include <algorithm>

Q_LOGGING_CATEGORY(appGroupLog, "org.lingmo.ds.ocean-apps.appgroup")

namespace apps {
AppGroup::AppGroup(const QString &groupId, const QString &name, const QList<QStringList> &appIDs)
    : AppItem(groupId, AppItemModel::FolderItemType)
    , m_itemsPage(new ItemsPage(name, groupId == QStringLiteral("internal/folder/0") ? (4 * 8) : (3 * 4)))
{
    setItemsPerPage(m_itemsPage->maxItemCountPerPage());
    setAppName(m_itemsPage->name());
    // folder id is a part of its groupId: "internal/folder/{folderId}"
    setFolderId(parseGroupId(groupId));

    for (const QStringList &items : appIDs) {
        m_itemsPage->appendPage(items);
    }
}

AppGroup::~AppGroup()
{
    delete m_itemsPage;
}

int AppGroup::folderId() const
{
    return data(AppGroupManager::GroupIdRole).toInt();
}

QList<QStringList> AppGroup::pages() const
{
    return m_itemsPage->allPagedItems();
}

ItemsPage *AppGroup::itemsPage()
{
    return m_itemsPage;
}

bool AppGroup::idIsFolder(const QString & id)
{
    return id.startsWith(QStringLiteral("internal/folder/"));
}

QString AppGroup::groupIdFromNumber(int groupId)
{
    return QStringLiteral("internal/folder/%1").arg(groupId);
}

int AppGroup::parseGroupId(const QString & id)
{
    using namespace std::string_view_literals;
    constexpr size_t len = "internal/folder/"sv.size();
    return QStringView{id}.mid(len + 1).toInt();
}

void AppGroup::setItemsPerPage(int number)
{
    return setData(number, AppGroupManager::GroupItemsPerPageRole);
}

void AppGroup::setFolderId(int folderId)
{
    setData(folderId, AppGroupManager::GroupIdRole);
}

}

