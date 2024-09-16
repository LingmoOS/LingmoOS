// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "traysortordermodel.h"

#include <QDebug>
#include <DConfig>

namespace docktray {

const QString SECTION_TRAY_ACTION = QLatin1String("tray-action");
const QString SECTION_STASHED = QLatin1String("stashed");
const QString SECTION_COLLAPSABLE = QLatin1String("collapsable");
const QString SECTION_FIXED = QLatin1String("fixed");
const QString SECTION_PINNED = QLatin1String("pinned");

TraySortOrderModel::TraySortOrderModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_dconfig(Dtk::Core::DConfig::create("org.deepin.dde.shell", "org.deepin.ds.dock.tray"))
{
    QHash<int, QByteArray> defaultRoleNames = roleNames();
    defaultRoleNames.insert({
        {TraySortOrderModel::SurfaceIdRole, QByteArrayLiteral("surfaceId")},
        {TraySortOrderModel::VisibilityRole, QByteArrayLiteral("visibility")},
        {TraySortOrderModel::SectionTypeRole, QByteArrayLiteral("sectionType")},
        {TraySortOrderModel::VisualIndexRole, QByteArrayLiteral("visualIndex")},
        {TraySortOrderModel::DelegateTypeRole, QByteArrayLiteral("delegateType")},
        {TraySortOrderModel::ForbiddenSectionsRole, QByteArrayLiteral("forbiddenSections")}
    });
    setItemRoleNames(defaultRoleNames);

    // init sort order data and hidden list data
    loadDataFromDConfig();

    // internal tray actions
    appendRow(createTrayItem("internal/action-stash-placeholder", SECTION_STASHED, "action-stash-placeholder"));
    appendRow(createTrayItem("internal/action-show-stash", SECTION_TRAY_ACTION, "action-show-stash"));
    appendRow(createTrayItem("internal/action-toggle-collapse", SECTION_TRAY_ACTION, "action-toggle-collapse"));
    appendRow(createTrayItem("internal/action-toggle-quick-settings", SECTION_TRAY_ACTION, "action-toggle-quick-settings"));

    connect(m_dconfig.get(), &Dtk::Core::DConfig::valueChanged, this, [this](const QString &key){
        if (key == QLatin1String("hiddenSurfaceIds")) {
            loadDataFromDConfig();
            updateVisualIndexes();
        }
    });

    connect(this, &TraySortOrderModel::availableSurfacesChanged, this, &TraySortOrderModel::onAvailableSurfacesChanged);

    connect(this, &TraySortOrderModel::collapsedChanged, this, [this](){
        qDebug() << "collapsedChanged";
        updateVisualIndexes();
        saveDataToDConfig();
    });
    connect(this, &TraySortOrderModel::actionsAlwaysVisibleChanged, this, [this](){
        qDebug() << "actionsAlwaysVisibleChanged";
        updateVisualIndexes();
    });
    updateVisualIndexes();
}

TraySortOrderModel::~TraySortOrderModel()
{
    // dtor
}

bool TraySortOrderModel::dropToStashTray(const QString &draggedSurfaceId, int dropVisualIndex, bool isBefore)
{
    // Check if the dragged tray surfaceId exists. Reject if not the case
    QList<QStandardItem *> draggedItems = findItems(draggedSurfaceId);
    if (draggedItems.isEmpty()) return false;
    Q_ASSERT(draggedItems.count() == 1);
    QStandardItem * draggedItem = draggedItems[0];
    if (draggedItem->data(ForbiddenSectionsRole).toStringList().contains(SECTION_STASHED)) return false;
    QStringList * sourceSection = getSection(draggedItem->data(SectionTypeRole).toString());

    // Ensure position adjustment will be saved at last
    auto deferSaveSortOrder = qScopeGuard([this](){saveDataToDConfig();});

    // drag inside the stashed section
    if (sourceSection == &m_stashedIds) {
        return false;
    } else {
        sourceSection->removeOne(draggedSurfaceId);
        m_stashedIds.append(draggedSurfaceId);
        updateVisualIndexes();
        return true;
    }
}

// drop existing item to tray, return true if drop attempt is accepted, false if rejected
bool TraySortOrderModel::dropToDockTray(const QString &draggedSurfaceId, int dropVisualIndex, bool isBefore)
{
    // Check if the dragged tray surfaceId exists. Reject if not the case
    QList<QStandardItem *> draggedItems = findItems(draggedSurfaceId);
    if (draggedItems.isEmpty()) return false;
    Q_ASSERT(draggedItems.count() == 1);
    QStringList * sourceSection = getSection(draggedItems[0]->data(SectionTypeRole).toString());
    QStringList forbiddenSections(draggedItems[0]->data(ForbiddenSectionsRole).toStringList());

    // Find the item attempted to drop on
    QStandardItem * dropOnItem = findItemByVisualIndex(dropVisualIndex, DockTraySection);
    if (!dropOnItem) return false;
    QString dropOnSurfaceId(dropOnItem->data(SurfaceIdRole).toString());

    // Ensure position adjustment will be saved at last
    auto deferSaveSortOrder = qScopeGuard([this](){saveDataToDConfig();});

    // If it's hidden, remove it from the hidden list. updateVisualIndexes() will update the
    // item's VisibilityRole property.
    // This is mainly for the drag item from quick settings panel feature.
    auto deferUpdateVisualIndex = qScopeGuard([this](){updateVisualIndexes();});
    if (m_hiddenIds.contains(draggedSurfaceId)) {
        m_hiddenIds.removeOne(draggedSurfaceId);
    }

    if (dropOnSurfaceId == QLatin1String("internal/action-show-stash")) {
        // show stash action is always the first action, drop before it consider as drop into stashed area
        if (sourceSection != &m_stashedIds) {
            sourceSection->removeOne(draggedSurfaceId);
            m_stashedIds.append(draggedSurfaceId);
            return true;
        } else {
            // already in the stashed tray
            return false;
        }
        if (sourceSection == &m_collapsableIds) {
            // same-section move
            m_collapsableIds.move(m_collapsableIds.indexOf(draggedSurfaceId), 0);
        } else {
            // prepend to collapsable section
            sourceSection->removeOne(draggedSurfaceId);
            m_collapsableIds.prepend(draggedSurfaceId);
        }
        return true;
    }

    if (dropOnSurfaceId == QLatin1String("internal/action-toggle-collapse")) {
        QStringList * targetSection = isBefore ? &m_collapsableIds : &m_pinnedIds;
        if (isBefore) {
            // move to the end of collapsable section
            if (forbiddenSections.contains(SECTION_COLLAPSABLE)) return false;
            if (targetSection == sourceSection) {
                m_collapsableIds.move(m_collapsableIds.indexOf(draggedSurfaceId), m_collapsableIds.count() - 1);
            } else {
                sourceSection->removeOne(draggedSurfaceId);
                m_collapsableIds.append(draggedSurfaceId);
            }
        } else {
            // move to the beginning of pinned section
            if (forbiddenSections.contains(SECTION_PINNED)) return false;
            if (targetSection == sourceSection) {
                m_pinnedIds.move(m_pinnedIds.indexOf(draggedSurfaceId), 0);
            } else {
                sourceSection->removeOne(draggedSurfaceId);
                m_pinnedIds.prepend(draggedSurfaceId);
            }
        }
        return true;
    }

    if (dropOnSurfaceId == QLatin1String("internal/action-toggle-quick-settings")) {
        return false;
    }

    if (dropOnSurfaceId == draggedSurfaceId) {
        // same item, don't need to do anything
        return false;
    }

    QString targetSectionName(dropOnItem->data(SectionTypeRole).toString());
    if (forbiddenSections.contains(targetSectionName)) return false;
    QStringList * targetSection = getSection(targetSectionName);
    if (targetSection == sourceSection) {
        int sourceIndex = targetSection->indexOf(draggedSurfaceId);
        int targetIndex = targetSection->indexOf(dropOnSurfaceId);
        if (isBefore) targetIndex--;
        if (targetIndex < 0) targetIndex = 0;
        if (sourceIndex == targetIndex) {
            // same item (draggedSurfaceId != dropOnSurfaceId caused by isBefore).
            return false;
        }
        targetSection->move(sourceIndex, targetIndex);
    } else {
        int targetIndex = targetSection->indexOf(dropOnSurfaceId);
        if (!isBefore) targetIndex++;
        sourceSection->removeOne(draggedSurfaceId);
        targetSection->insert(targetIndex, draggedSurfaceId);
    }
    return true;
}

void TraySortOrderModel::setSurfaceVisible(const QString &surfaceId, bool visible)
{
    if (visible) {
        if (m_hiddenIds.contains(surfaceId)) {
            m_hiddenIds.removeOne(surfaceId);
        }
    } else {
        if (!m_hiddenIds.contains(surfaceId)) {
            m_hiddenIds.append(surfaceId);
        }
    }
    updateVisualIndexes();
}

QStandardItem *TraySortOrderModel::findItemByVisualIndex(int visualIndex, VisualSections visualSection) const
{
    QStandardItem * result = nullptr;
    const QModelIndexList matched = match(index(0, 0), VisualIndexRole, visualIndex, -1, Qt::MatchExactly);
    for (const QModelIndex & index : matched) {
        QString section(data(index, SectionTypeRole).toString());
        if (visualSection == DockTraySection) {
            if (section == SECTION_STASHED) continue;
            if (m_collapsed && section == SECTION_COLLAPSABLE) continue;
        } else {
            if (section != SECTION_STASHED) continue;
        }
        if (!data(index, VisibilityRole).toBool()) continue;

        result = itemFromIndex(index);
        break;
    }
    return result;
}

QStringList *TraySortOrderModel::getSection(const QString &sectionType)
{
    if (sectionType == SECTION_PINNED) {
        return &m_pinnedIds;
    } else if (sectionType == SECTION_COLLAPSABLE) {
        return &m_collapsableIds;
    } else if (sectionType == SECTION_STASHED) {
        return &m_stashedIds;
    } else if (sectionType == SECTION_FIXED) {
        return &m_fixedIds;
    }

    return nullptr;
}

QString TraySortOrderModel::findSection(const QString & surfaceId, const QString & fallback, const QStringList & forbiddenSections, bool isForceDock)
{
    QStringList * found = nullptr;
    QString result(fallback);

    if (m_pinnedIds.contains(surfaceId)) {
        found = &m_pinnedIds;
        result = SECTION_PINNED;
    } else if (m_collapsableIds.contains(surfaceId)) {
        found = &m_collapsableIds;
        result = SECTION_COLLAPSABLE;
    } else if (m_stashedIds.contains(surfaceId)) {
        found = &m_stashedIds;
        result = SECTION_STASHED;
    } else if (m_fixedIds.contains(surfaceId)) {
        found = &m_fixedIds;
        result = SECTION_FIXED;
    }

    // 设置默认隐藏
    if (!found &&                   // 不在列表中
        !isForceDock &&             // 非 forceDock
        result != SECTION_FIXED &&  // 非固定位置插件（时间）
        !surfaceId.startsWith("internal/") &&       // 非内置插件
        !surfaceId.startsWith("application-tray::") // 非托盘图标
        ) {
        if (!m_hiddenIds.contains(surfaceId)) {
            m_hiddenIds.append(surfaceId);
        }
    }

    if (forbiddenSections.contains(result)) {
        Q_ASSERT(result != fallback);
        if (found) {
            found->removeOne(surfaceId);
            result = fallback;
        }
    }

    return result;
}

void TraySortOrderModel::registerToSection(const QString & surfaceId, const QString & sectionType)
{
    QStringList * section = getSection(sectionType);

    if (section == nullptr) {
        Q_ASSERT(sectionType == SECTION_TRAY_ACTION);
        return;
    }

    if (!section->contains(surfaceId)) {
        section->prepend(surfaceId);
    }
}

QStandardItem * TraySortOrderModel::createTrayItem(const QString & name, const QString & sectionType,
                                                  const QString & delegateType, const QStringList &forbiddenSections,
                                                  bool isForceDock)
{
    QString actualSectionType = findSection(name, sectionType, forbiddenSections, isForceDock);
    registerToSection(name, actualSectionType);

    qDebug() << actualSectionType << name << delegateType;

    QStandardItem * item = new QStandardItem(name);
    item->setData(name, TraySortOrderModel::SurfaceIdRole);
    item->setData(true, TraySortOrderModel::VisibilityRole);
    item->setData(actualSectionType, TraySortOrderModel::SectionTypeRole);
    item->setData(delegateType, TraySortOrderModel::DelegateTypeRole);
    item->setData(forbiddenSections, TraySortOrderModel::ForbiddenSectionsRole);
    item->setData(-1, TraySortOrderModel::VisualIndexRole);
    item->setData(isForceDock, TraySortOrderModel::IsForceDockRole);

    return item;
}

void TraySortOrderModel::updateVisualIndexes()
{
    for (int i = 0; i < rowCount(); i++) {
        item(i)->setData(-1, TraySortOrderModel::VisualIndexRole);
    }

    // stashed action
    // "internal/action-stash-placeholder"
    QList<QStandardItem *> results = findItems("internal/action-stash-placeholder");
    Q_ASSERT(!results.isEmpty());
    QStandardItem * stashPlaceholder = results[0];

    // the visual index of stashed items are also for their sort order, but the index
    // number is independently from these non-stashed items.
    int stashedVisualIndex = 0;
    bool showStashActionVisible = m_actionsAlwaysVisible;
    for (const QString & id : std::as_const(m_stashedIds)) {
        QList<QStandardItem *> results = findItems(id);
        if (results.isEmpty()) continue;
        if (results[0]->data(TraySortOrderModel::VisualIndexRole).toInt() != -1) continue;
        if (stashPlaceholder == results[0]) continue;
        bool itemVisible = results[0]->data(TraySortOrderModel::IsForceDockRole).toBool() || !m_hiddenIds.contains(id);
        results[0]->setData(SECTION_STASHED, TraySortOrderModel::SectionTypeRole);
        if (itemVisible) {
            showStashActionVisible = true;
            results[0]->setData(stashedVisualIndex, TraySortOrderModel::VisualIndexRole);
            stashedVisualIndex++;
        }
    }

    stashPlaceholder->setData(stashedVisualIndex == 0 && showStashActionVisible, TraySortOrderModel::VisibilityRole);

    int currentVisualIndex = 0;
    // "internal/action-show-stash"
    results = findItems("internal/action-show-stash");
    Q_ASSERT(!results.isEmpty());
    results[0]->setData(showStashActionVisible, TraySortOrderModel::VisibilityRole);
    if (showStashActionVisible) {
        results[0]->setData(currentVisualIndex, TraySortOrderModel::VisualIndexRole);
        currentVisualIndex++;
    }

    // collapsable
    bool toogleCollapseActionVisible = m_actionsAlwaysVisible;
    for (const QString & id : std::as_const(m_collapsableIds)) {
        QList<QStandardItem *> results = findItems(id);
        if (results.isEmpty()) continue;
        if (results[0]->data(TraySortOrderModel::VisualIndexRole).toInt() != -1) continue;
        bool itemVisible = results[0]->data(TraySortOrderModel::IsForceDockRole).toBool() || !m_hiddenIds.contains(id);
        results[0]->setData(SECTION_COLLAPSABLE, TraySortOrderModel::SectionTypeRole);
        results[0]->setData(itemVisible, TraySortOrderModel::VisibilityRole);
        if (itemVisible) {
            toogleCollapseActionVisible = true;
            if (!m_collapsed) {
                results[0]->setData(currentVisualIndex++, TraySortOrderModel::VisualIndexRole);
            } else {
                results[0]->setData(currentVisualIndex-1, TraySortOrderModel::VisualIndexRole);
            }
        }
    }

    // "internal/action-toggle-collapse"
    results = findItems("internal/action-toggle-collapse");
    Q_ASSERT(!results.isEmpty());
    results[0]->setData(toogleCollapseActionVisible, TraySortOrderModel::VisibilityRole);
    if (toogleCollapseActionVisible) {
        results[0]->setData(currentVisualIndex, TraySortOrderModel::VisualIndexRole);
        currentVisualIndex++;
    }

    // pinned
    for (const QString & id : std::as_const(m_pinnedIds)) {
        QList<QStandardItem *> results = findItems(id);
        if (results.isEmpty()) continue;
        if (results[0]->data(TraySortOrderModel::VisualIndexRole).toInt() != -1) continue;
        bool itemVisible = results[0]->data(TraySortOrderModel::IsForceDockRole).toBool() || !m_hiddenIds.contains(id);
        results[0]->setData(SECTION_PINNED, TraySortOrderModel::SectionTypeRole);
        results[0]->setData(itemVisible, TraySortOrderModel::VisibilityRole);
        if (itemVisible) {
            results[0]->setData(currentVisualIndex, TraySortOrderModel::VisualIndexRole);
            currentVisualIndex++;
        }
    }

    // "internal/action-toggle-quick-settings"
    results = findItems("internal/action-toggle-quick-settings");
    Q_ASSERT(!results.isEmpty());
    results[0]->setData(currentVisualIndex, TraySortOrderModel::VisualIndexRole);
    currentVisualIndex++;

    // fixed (not actually 'fixed' since it's just a section next to pinned)
    // By design, fixed items can be rearranged within the fixed section, but cannot
    // move to other sections. We archive that by setting the 'forbiddenSections' property
    // to the items in fixed sections.
    for (const QString & id : std::as_const(m_fixedIds)) {
        QList<QStandardItem *> results = findItems(id);
        if (results.isEmpty()) continue;
        if (results[0]->data(TraySortOrderModel::VisualIndexRole).toInt() != -1) continue;
        bool itemVisible = results[0]->data(TraySortOrderModel::IsForceDockRole).toBool() || !m_hiddenIds.contains(id);
        results[0]->setData(SECTION_FIXED, TraySortOrderModel::SectionTypeRole);
        results[0]->setData(itemVisible, TraySortOrderModel::VisibilityRole);
        if (itemVisible) {
            results[0]->setData(currentVisualIndex, TraySortOrderModel::VisualIndexRole);
            currentVisualIndex++;
        }
    }

    // update visible item count property
    setProperty("visualItemCount", currentVisualIndex);

    qDebug() << "update" << m_visualItemCount << currentVisualIndex;
}

QString TraySortOrderModel::registerSurfaceId(const QVariantMap & surfaceData)
{
    QString surfaceId(surfaceData.value("surfaceId").toString());
    QString delegateType(surfaceData.value("delegateType", "legacy-tray-plugin").toString());
    QString preferredSection(surfaceData.value("sectionType", "collapsable").toString());
    QStringList forbiddenSections(surfaceData.value("forbiddenSections").toStringList());
    bool isForceDock(surfaceData.value("isForceDock").toBool());

    QList<QStandardItem *> results = findItems(surfaceId);
    if (!results.isEmpty()) {
        QStandardItem * result = results[0];
        // check if the item is currently in a forbidden zone
        QString currentSection(result->data(SectionTypeRole).toString());
        if (forbiddenSections.contains(currentSection)) {
            result->setData(findSection(surfaceId, preferredSection, forbiddenSections, isForceDock), SectionTypeRole);
        }
        return surfaceId;
    }

    appendRow(createTrayItem(surfaceId, preferredSection, delegateType, forbiddenSections, isForceDock));
    return surfaceId;
}

void TraySortOrderModel::loadDataFromDConfig()
{
    m_stashedIds = m_dconfig->value("stashedSurfaceIds").toStringList();
    m_collapsableIds = m_dconfig->value("collapsableSurfaceIds").toStringList();
    m_pinnedIds = m_dconfig->value("pinnedSurfaceIds").toStringList();
    m_hiddenIds = m_dconfig->value("hiddenSurfaceIds").toStringList();
    m_collapsed = m_dconfig->value("isCollapsed").toBool();
}

void TraySortOrderModel::saveDataToDConfig()
{
    m_dconfig->setValue("stashedSurfaceIds", m_stashedIds);
    m_dconfig->setValue("collapsableSurfaceIds", m_collapsableIds);
    m_dconfig->setValue("pinnedSurfaceIds", m_pinnedIds);
    m_dconfig->setValue("hiddenSurfaceIds", m_hiddenIds);
    m_dconfig->setValue("isCollapsed", m_collapsed);
}

void TraySortOrderModel::onAvailableSurfacesChanged()
{
    QStringList availableSurfaceIds;
    // check if there is any new tray item needs to be registered, and register it.
    for (const QVariantMap & surface : m_availableSurfaces) {
        // already registered items will be checked so this is fine
        const QString surfaceId(registerSurfaceId(surface));
        availableSurfaceIds << surfaceId;
    }

    // check if there are tray items no longer availabled, and remove them.
    for (int i = rowCount() - 1; i >= 0; i--) {
        const QString surfaceId(data(index(i, 0), TraySortOrderModel::SurfaceIdRole).toString());
        if (availableSurfaceIds.contains(surfaceId)) continue;
        if (surfaceId.startsWith("internal/")) continue;
        removeRow(i);
    }
    // finally, update visual index
    updateVisualIndexes();
    // and also save the current sort order
    saveDataToDConfig();
}

}
