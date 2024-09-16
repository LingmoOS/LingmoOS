// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickpanelproxymodel.h"

#include <QDebug>
#include <DConfig>
#include <DDBusSender>
DCORE_USE_NAMESPACE

namespace dock {
namespace {
enum {
    PluginId = Qt::UserRole + 10,
    PluginDisplayName,
    QuickSurface,
    QuickSurfaceLayoutType,
    QuickSurfaceItemKey,
    TraySurface,
    TraySurfaceItemKey
} ProxyRole;
}
QuickPanelProxyModel::QuickPanelProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    updateQuickPlugins();
    sort(0);
}

QString QuickPanelProxyModel::getTitle(const QString &pluginId) const
{
    const auto index = surfaceIndex(pluginId);
    return surfaceDisplayName(index);
}

bool QuickPanelProxyModel::isQuickPanelPopup(const QString &pluginId, const QString &itemKey) const
{
    const auto index = surfaceIndex(pluginId);
    if (!index.isValid())
        return false;

    const auto item = surfaceItemKey(index);
    return item == itemKey;
}

void QuickPanelProxyModel::openSystemSettings()
{
    DDBusSender()
        .service("org.deepin.dde.ControlCenter1")
        .interface("org.deepin.dde.ControlCenter1")
        .path("/org/deepin/dde/ControlCenter1")
        .method(QString("Show"))
        .call();
}

void QuickPanelProxyModel::openShutdownScreen()
{
    DDBusSender()
        .service("org.deepin.dde.ShutdownFront1")
        .interface("org.deepin.dde.ShutdownFront1")
        .path("/org/deepin/dde/ShutdownFront1")
        .method("Show")
        .call();
}

QVariant QuickPanelProxyModel::data(const QModelIndex &index, int role) const
{
    const auto sourceIndex = mapToSource(index);
    switch (role) {
    case PluginId:
        return surfacePluginId(sourceIndex);
    case PluginDisplayName:
        return surfaceDisplayName(sourceIndex);
    case QuickSurfaceItemKey:
        return surfaceItemKey(sourceIndex);
    case QuickSurfaceLayoutType:
        return surfaceType(sourceIndex);
    case QuickSurface:
        return QVariant::fromValue(surfaceObject(sourceIndex));
    case TraySurface: {
        const auto id = surfacePluginId(sourceIndex);
        return QVariant::fromValue(traySurfaceObject(id));
    }
    case TraySurfaceItemKey: {
        const auto id = surfacePluginId(sourceIndex);
        return traySurfaceItemKey(id);
    }
    }
    return {};
}

QHash<int, QByteArray> QuickPanelProxyModel::roleNames() const
{
    const QHash<int, QByteArray> roles {
        {PluginId, "pluginId"}, // plugin's id.
        {PluginDisplayName, "displayName"}, // plugin's displayName.
        {QuickSurface, "surface"}, // quick surface item.
        {QuickSurfaceLayoutType, "surfaceLayoutType"}, // quick surface's layout type. (1, signal), (2, multi), (4, full)
        {QuickSurfaceItemKey, "surfaceItemKey"}, // quick surface's itemKey.
        {TraySurface, "traySurface"},// tray surface item.
        {TraySurfaceItemKey, "traySurfaceItemKey"},// tray surface itemKey.
    };
    return roles;
}

bool QuickPanelProxyModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    auto leftOrder = pluginOrder(sourceLeft);
    auto rightOrder = pluginOrder(sourceRight);

    return leftOrder < rightOrder;
}

bool QuickPanelProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const auto index = this->sourceModel()->index(sourceRow, 0, sourceParent);
    if (!index.isValid())
        return false;
    if (m_quickPlugins.isEmpty())
        return true;
    const auto &id = surfacePluginId(index);
    return m_quickPlugins.contains(id);
}

void QuickPanelProxyModel::updateQuickPlugins()
{
    std::unique_ptr<DConfig> dconfig(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.dock.tray"));
    m_quickPlugins = dconfig->value("quickPlugins").toStringList();
    qDebug() << "Fetched QuickPanel's plugin by DConfig,"
             << "plugin list size:" << m_quickPlugins.size();
    invalidate();
}

void QuickPanelProxyModel::watchingCountChanged()
{
    static const struct {
        const char *signalName;
        const char *slotName;
    } connectionTable[] = {
                           { SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(updateTrayItemSurface()) },
                           { SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(updateTrayItemSurface()) },
                           };

    for (const auto &c : connectionTable) {
        disconnect(m_trayPluginModel, c.signalName, this, c.slotName);
    }
    for (const auto &c : connectionTable) {
        connect(m_trayPluginModel, c.signalName, this, c.slotName);
    }
}

int QuickPanelProxyModel::pluginOrder(const QModelIndex &index) const
{
    const auto id = surfacePluginId(index);
    auto ret = m_quickPlugins.indexOf(id);
    auto order = surfaceOrder(index);
    if (order > 0) {
        ret = order;
    }
    auto type = surfaceType(index);
    const QMap<int, int> OrderOffset {
        {1, 2000},
        {2, 1000},
        {4, 4000},
    };
    ret += OrderOffset.value(type);

    return ret;
}

int QuickPanelProxyModel::surfaceType(const QModelIndex &index) const
{
    // Quick_Panel_Single = 0x40, Quick_Panel_Multi = 0x80, Quick_Panel_Full = 0x100
    auto flags = surfaceValue(index, "pluginFlags").toInt();
    if (flags & 0x100)
        return 4;
    if (flags & 0x80)
        return 2;
    return 1;
}

int QuickPanelProxyModel::surfaceOrder(const QModelIndex &index) const
{
    return surfaceValue(index, "order").toInt();
}

QString QuickPanelProxyModel::surfacePluginId(const QModelIndex &index) const
{
    return surfaceValue(index, "pluginId").toString();
}

QString QuickPanelProxyModel::surfaceItemKey(const QModelIndex &index) const
{
    return surfaceValue(index, "itemKey").toString();
}

QString QuickPanelProxyModel::surfaceDisplayName(const QModelIndex &index) const
{
    return surfaceValue(index, "displayName").toString();
}

QVariant QuickPanelProxyModel::surfaceValue(const QModelIndex &index, const QByteArray &roleName) const
{
    if (auto modelData = surfaceObject(index))
        return modelData->property(roleName);

    return {};
}

QModelIndex QuickPanelProxyModel::surfaceIndex(const QString &pluginId) const
{
    const auto targetModel = surfaceModel();
    if (!targetModel)
        return {};
    for (int i = 0; i < targetModel->rowCount(); i++) {
        const auto index = targetModel->index(i, 0);
        const auto id = surfacePluginId(index);
        if (id == pluginId)
            return index;
    }
    return {};
}

QObject *QuickPanelProxyModel::surfaceObject(const QModelIndex &index) const
{
    const auto modelDataRole = roleByName("shellSurface");
    if (modelDataRole >= 0)
        return surfaceModel()->data(index, modelDataRole).value<QObject *>();

    return nullptr;
}

QObject *QuickPanelProxyModel::traySurfaceObject(const QString &pluginId) const
{
    const auto targetModel = m_trayPluginModel;
    if (!targetModel)
        return nullptr;

    const auto roleNames = targetModel->roleNames();
    const auto modelDataRole = roleNames.key("shellSurface", -1);
    if (modelDataRole < 0)
        return nullptr;
    for (int i = 0; i < targetModel->rowCount(); i++) {
        const auto index = targetModel->index(i, 0);
        const auto item = index.data(modelDataRole).value<QObject *>();
        if (!item)
            return nullptr;
        const auto id = item->property("pluginId").toString();
        if (id == pluginId)
            return item;
    }
    return nullptr;
}

QString QuickPanelProxyModel::traySurfaceItemKey(const QString &pluginId) const
{
    if (auto object = traySurfaceObject(pluginId))
        return object->property("itemKey").toString();

    return QString();
}

int QuickPanelProxyModel::roleByName(const QByteArray &roleName) const
{
    if (!surfaceModel())
        return -1;
    const auto roleNames = surfaceModel()->roleNames();
    return roleNames.key(roleName, -1);
}

QAbstractListModel *QuickPanelProxyModel::surfaceModel() const
{
    return qobject_cast<QAbstractListModel *>(sourceModel());
}

void QuickPanelProxyModel::updateTrayItemSurface()
{
    emit trayItemSurfaceChanged();
    if (rowCount() > 0)
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {TraySurface, TraySurfaceItemKey});
}

void QuickPanelProxyModel::classBegin()
{
}

void QuickPanelProxyModel::componentComplete()
{
    updateTrayItemSurface();
}

QObject *QuickPanelProxyModel::trayItemSurface() const
{
    if (m_trayItemPluginId.isEmpty())
        return nullptr;

    return traySurfaceObject(m_trayItemPluginId);
}

QString QuickPanelProxyModel::trayItemPluginId() const
{
    return m_trayItemPluginId;
}

void QuickPanelProxyModel::setTrayItemPluginId(const QString &newTrayItemPluginId)
{
    if (m_trayItemPluginId == newTrayItemPluginId)
        return;
    m_trayItemPluginId = newTrayItemPluginId;
    emit trayItemPluginIdChanged();
    updateTrayItemSurface();
}

QAbstractItemModel *QuickPanelProxyModel::trayPluginModel() const
{
    return m_trayPluginModel;
}

void QuickPanelProxyModel::setTrayPluginModel(QAbstractItemModel *newTrayPluginModel)
{
    if (m_trayPluginModel == newTrayPluginModel)
        return;
    m_trayPluginModel = newTrayPluginModel;
    watchingCountChanged();
    emit trayPluginModelChanged();
}

}
