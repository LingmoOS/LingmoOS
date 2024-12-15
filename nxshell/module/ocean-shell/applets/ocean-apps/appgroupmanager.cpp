// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appgroupmanager.h"
#include "appgroup.h"
#include "itemspage.h"
#include "amappitemmodel.h"
#include "amappitem.h"

#define TOPLEVEL_FOLDERID 0

namespace apps {

AppGroupManager::AppGroupManager(AMAppItemModel * referenceModel, QObject *parent)
    : QStandardItemModel(parent)
    , m_referenceModel(referenceModel)
    , m_config(Dtk::Core::DConfig::create("org.lingmo.ocean.shell", "org.lingmo.ds.ocean-apps", "", this))
    , m_dumpTimer(new QTimer(this))
{
    m_dumpTimer->setSingleShot(true);
    m_dumpTimer->setInterval(1000);

    launchpadArrangementConfigMigration();
    loadAppGroupInfo();

    connect(m_referenceModel, &AMAppItemModel::rowsInserted, this, [this](){
        onReferenceModelChanged();
        saveAppGroupInfo();
    });
    connect(m_referenceModel, &AMAppItemModel::rowsRemoved, this, [this](){
        onReferenceModelChanged();
        saveAppGroupInfo();
    });
    connect(m_dumpTimer, &QTimer::timeout, this, [this](){
        saveAppGroupInfo();
    });
    connect(this, &AppGroupManager::dataChanged, this, &AppGroupManager::saveAppGroupInfo);
}

QVariant AppGroupManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == GroupIdRole) {
        return index.row();
    }

    return QStandardItemModel::data(index, role);
}

// Find the item's location. If folderId is -1, search all folders.
ItemPosition AppGroupManager::findItem(const QString &appId, int folderId)
{
    int page, idx;

    for (int i = 0; i < rowCount(); i++) {
        auto group = static_cast<AppGroup*>(itemFromIndex(index(i, 0)));
        if (folderId >= 0 && group->folderId() != folderId) {
            continue;
        }
        std::tie(page, idx) = group->itemsPage()->findItem(appId);
        if (page != -1) {
            return ItemPosition(group->folderId(), page, idx);
        }
    }

    return ItemPosition();
}

void AppGroupManager::appendItemToGroup(const QString &appId, int groupId)
{
    auto folder = group(groupId);
    Q_CHECK_PTR(folder);
    folder->itemsPage()->appendItem(appId);
}

bool AppGroupManager::removeItemFromGroup(const QString &appId, int groupId)
{
    auto folder = group(groupId);
    Q_CHECK_PTR(folder);
    return folder->itemsPage()->removeItem(appId);
}

QModelIndex AppGroupManager::groupIndexById(int groupId)
{
    for (int i = 0; i < rowCount(); i++) {
        auto groupIndex = index(i, 0);
        auto data = groupIndex.data(GroupIdRole);
        if (data.toInt() == groupId) {
            return groupIndex;
        }
    }
    return QModelIndex();
}

AppGroup * AppGroupManager::group(int groupId)
{
    auto groupIndex = groupIndexById(groupId);
    if (!groupIndex.isValid()) {
        return nullptr;
    }
    auto group = static_cast<AppGroup*>(itemFromIndex(groupIndex));
    return group;
}

AppGroup * AppGroupManager::group(QModelIndex idx)
{
    if (!idx.isValid()) {
        return nullptr;
    }
    auto group = static_cast<AppGroup*>(itemFromIndex(idx));
    return group;
}

ItemsPage * AppGroupManager::groupPages(int groupId)
{
    AppGroup * folder = group(groupId);
    if (!folder) return nullptr;
    return folder->itemsPage();
}

void AppGroupManager::bringToFromt(const QString & id)
{
    const ItemPosition origPos = findItem(id);

    // can only bring top-level item to front
    Q_ASSERT(origPos.group() != TOPLEVEL_FOLDERID);
    if (origPos.group() != TOPLEVEL_FOLDERID) {
        return;
    }

    // already at front
    if (origPos.page() == 0 && origPos.pos() == 0) {
        return;
    }

    auto * topLevel = groupPages(0);
    topLevel->moveItemPosition(origPos.page(), origPos.pos(), 0, 0, false);

    saveAppGroupInfo();

    // TODO: emit signal to refresh the view
}

void AppGroupManager::commitRearrangeOperation(const QString & dragId, const QString & dropId, DndOperation operation, int pageHint)
{
    if (dragId == dropId) return;

    const ItemPosition dragOrigPos = findItem(dragId);
    const ItemPosition dropOrigPos = findItem(dropId);

    Q_ASSERT(dragOrigPos.group() != -1);
    if (dragOrigPos.group() == -1) {
        qWarning() << "Cannot found" << dragId << "in current item arrangement.";
        return;
    }

    if (operation != DndOperation::DndJoin) {
        // move to dropId's front or back
        if (dragOrigPos.group() == dropOrigPos.group()) {
            // same folder item re-arrangement
            ItemsPage * folder = groupPages(dropOrigPos.group());
            folder->moveItemPosition(dragOrigPos.page(), dragOrigPos.pos(), dropOrigPos.page(), dropOrigPos.pos(), operation == DndOperation::DndAppend);
        } else {
            // different folder item arrangement
            ItemsPage * srcFolder = groupPages(dragOrigPos.group());
            ItemsPage * dstFolder = groupPages(dropOrigPos.group());
            srcFolder->removeItem(dragId);
            if (srcFolder->pageCount() == 0 && srcFolder != dstFolder) {
                removeGroup(dragOrigPos.group());
            }
            dstFolder->insertItem(dropId, dropOrigPos.page(), dropOrigPos.pos());
        }
    } else {
        if (AppGroup::idIsFolder(dragId) && dropId != "internal/folders/0") return; // cannot drag folder onto something
        if (dropOrigPos.group() != TOPLEVEL_FOLDERID && dropId != "internal/folders/0") return; // folder inside folder is not allowed

        // the source item will be inside a new folder anyway.
        ItemsPage * srcFolder = groupPages(dragOrigPos.group());
    
        if (AppGroup::idIsFolder(dropId)) {
            // drop into existing folder
            ItemsPage * dstFolder = groupPages(AppGroup::parseGroupId(dropId));
            const int fromPage = dragOrigPos.page();
            const int toPage = pageHint;
            if (srcFolder == dstFolder) {
                if (srcFolder->pageCount() == 1 ||
                    (fromPage == toPage && srcFolder->itemCount(fromPage) == 1)) { // dnd the only item to the same page
                    return;
                }
            }

            // hold the empty page avoid access out of page range!
            srcFolder->removeItem(dragId, false);
            if (srcFolder->itemCount() == 0 && srcFolder != dstFolder) {
                removeGroup(dragOrigPos.group());
            }
            dstFolder->insertItemToPage(dragId, toPage);

            // clear empty page
            srcFolder->removeEmptyPages();
        } else {
            srcFolder->removeItem(dragId);
            // we need the app info for the new folder name...
            AMAppItem * item = m_referenceModel->appItem(dropId);
            AppItemModel::OCEANCategories dropCategories = item ? item->oceanCategories() : AppItemModel::Others;
            // make a new folder, move two items into the folder
            QString dstFolderId = assignGroupId();
            appendGroup(dstFolderId, "internal/category/" + QString::number(dropCategories), {{dragId, dropId}});
            if (srcFolder->pageCount() == 0 && dragOrigPos.group() != 0) {
                removeGroup(dragOrigPos.group());
            }
            auto * topLevel = groupPages(0);
            topLevel->insertItem(dstFolderId, dropOrigPos.page(), dropOrigPos.pos());
            topLevel->removeItem(dropId);
        }
    }

    saveAppGroupInfo();

    // TODO: emit signal to refresh the view
}

QVariantList AppGroupManager::fromListOfStringList(const QList<QStringList> & list)
{
    QVariantList data;
    std::transform(list.begin(), list.end(), std::back_inserter(data), [](const QStringList &items) {
        QVariantList tmp;
        std::transform(items.begin(), items.end(), std::back_inserter(tmp), [](const auto &item) {
            return QVariant::fromValue(item);
        });
        return items;
    });

    return data;
}

// On AM model changed, add newly installed apps to group (if any) and remove apps that are no longer exists.
void AppGroupManager::onReferenceModelChanged()
{
    // Avoid remove all existing records when first time (AM model is not ready).
    if (m_referenceModel->rowCount() == 0) {
        qDebug() << "referenceModel not ready, wait for next time";
        return;
    }

    QSet<QString> appSet;
    for (int i = 0; i < m_referenceModel->rowCount(); i++) {
        const auto modelIndex = m_referenceModel->index(i, 0);
        const bool noDisplay = m_referenceModel->data(modelIndex, AppItemModel::NoDisplayRole).toBool();
        if (noDisplay) {
            continue;
        }
        const QString & desktopId = m_referenceModel->data(m_referenceModel->index(i, 0), AppItemModel::DesktopIdRole).toString();
        appSet.insert(desktopId);
        // add all existing ones if they are not already in
        ItemPosition itemPos = findItem(desktopId);
        if (itemPos.group() == -1) {
            appendItemToGroup(desktopId, TOPLEVEL_FOLDERID);
        }
    }

    // remove all non-existing ones
    for (int i = rowCount() - 1; i >= 0; i--) {
        auto folder = group(index(i, 0));
        Q_CHECK_PTR(folder);
        folder->itemsPage()->removeItemsNotIn(appSet);
        // check if group itself is also empty, remove them too.
        if (folder->itemsPage()->itemCount() == 0 && folder->folderId() != TOPLEVEL_FOLDERID) {
            QString groupId = folder->appId();
            removeRow(i);
            removeItemFromGroup(groupId, TOPLEVEL_FOLDERID);
        }
    }

    // TODO: emit datachanged signal?
    // TODO: save item arrangement to user data?
}

void AppGroupManager::launchpadArrangementConfigMigration()
{
    const QString arrangementSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
    const QString arrangementSettingPath(QDir(arrangementSettingBasePath).absoluteFilePath("lingmo/ocean-launchpad/item-arrangement.ini"));
    if (!QFile::exists(arrangementSettingPath)) return;
    QSettings itemArrangementSettings(arrangementSettingPath, QSettings::NativeFormat);

    itemArrangementSettings.beginGroup("fullscreen");
    const QStringList folderGroups(itemArrangementSettings.childGroups());

    QVariantList list;

    for (const QString & groupName : folderGroups) {
        itemArrangementSettings.beginGroup(groupName);
        QString folderName = itemArrangementSettings.value("name", QString()).toString();
        int pageCount = itemArrangementSettings.value("pageCount", 0).toInt();
        bool isTopLevel = groupName == "toplevel";

        QVariantMap valueMap;
        valueMap.insert("name", folderName);
        valueMap.insert("groupId", groupName.toInt());

        QList<QStringList> pages;
        for (int i = 0; i < pageCount; i++) {
            QStringList items = itemArrangementSettings.value(QString::asprintf("pageItems/%d", i)).toStringList();
            // check if items have ".desktop" suffix, and remove the suffix
            std::transform(items.begin(), items.end(), items.begin(), [](QString &item) {
                if (item.endsWith(".desktop")) {
                    item.remove(".desktop");
                }
                return item;
            });
            pages.append(items);
        }

        valueMap.insert("appItems", fromListOfStringList(pages));
        list.append(valueMap);

        itemArrangementSettings.endGroup();
    }

    m_config->setValue("Groups", list);

    // TODO: delete the old configuration file
}

void AppGroupManager::loadAppGroupInfo()
{
    auto groups = m_config->value("Groups").toList();

    for (int i = 0; i < groups.length(); i++) {
        auto group = groups[i].toMap();
        auto groupId = group.value("groupId", "").toString();
        auto name = group.value("name", "").toString();
        auto pages = group.value("appItems", QVariantList()).toList();
        QList<QStringList> items;

        for (int j = 0; j < pages.length(); j++) {
            auto page = pages[j].toStringList();
            items << page;
        }

        if (groupId.isEmpty()) {
            groupId = assignGroupId();
        }
        auto p = appendGroup(groupId, name, items);
    }

    // always ensure top-level group exists
    if (rowCount() == 0) {
        auto p = appendGroup(assignGroupId(), "Top Level", {});
        Q_ASSERT(p->folderId() == TOPLEVEL_FOLDERID);
    }
}

void AppGroupManager::saveAppGroupInfo()
{
    QVariantList list;
    for (int i = 0; i < rowCount(); i++) {
        auto folder = group(index(i, 0));
        QVariantMap valueMap;
        valueMap.insert("name", folder->data(AppItemModel::NameRole));
        valueMap.insert("groupId", folder->appId());
        valueMap.insert("appItems", fromListOfStringList(folder->pages()));
        list << valueMap;
    }

    m_config->setValue("Groups", list);
}

QString AppGroupManager::assignGroupId() const
{
    QStringList knownGroupIds;
    for (int i = 0; i < rowCount(); i++) {
        auto group = index(i, 0);
        knownGroupIds.append(group.data(AppItemModel::DesktopIdRole).toString());
    }

    int idNumber = 0;
    while (knownGroupIds.contains(QString("internal/group/%1").arg(idNumber))) {
        idNumber++;
    }

    return QString("internal/group/%1").arg(idNumber);
}

AppGroup * AppGroupManager::appendGroup(int groupId, QString groupName, const QList<QStringList> &appItemIDs)
{
    Q_ASSERT(!groupIndexById(groupId).isValid());
    return appendGroup(AppGroup::groupIdFromNumber(groupId), groupName, appItemIDs);
}

AppGroup * AppGroupManager::appendGroup(QString groupId, QString groupName, const QList<QStringList> &appItemIDs)
{
    auto p = new AppGroup(groupId, groupName, appItemIDs);
    appendRow(p);
    return p;
}

void AppGroupManager::removeGroup(int groupId)
{
    auto groupIndex = groupIndexById(groupId);
    Q_ASSERT(groupIndex.isValid());
    removeRow(groupIndex.row());
}

}
