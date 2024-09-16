// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appslistmodel.h"
#include "appsmanager.h"
#include "calculate_util.h"
#include "constants.h"
#include "dbusvariant/iteminfo.h"
#include "util.h"

#include <QSize>
#include <QDebug>
#include <QPixmap>
#include <QSettings>
#include <QGSettings>
#include <QVariant>

#include <DHiDPIHelper>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static QString ChainsProxy_path = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
        + "/deepin/proxychains.conf";

static QMap<int, AppsListModel::AppCategory> CateGoryMap {
    { 0,  AppsListModel::Internet    },
    { 1,  AppsListModel::Chat        },
    { 2,  AppsListModel::Music       },
    { 3,  AppsListModel::Video       },
    { 4,  AppsListModel::Graphics    },
    { 5,  AppsListModel::Game        },
    { 6,  AppsListModel::Office      },
    { 7,  AppsListModel::Reading     },
    { 8,  AppsListModel::Development },
    { 9,  AppsListModel::System      },
    { 10, AppsListModel::Others      }
};

const QStringList sysHideUseProxyPackages();
const QStringList sysCantUseProxyPackages();

static QGSettings *gSetting = SettingsPtr("com.deepin.dde.launcher", "/com/deepin/dde/launcher/");
static QStringList hideUseProxyPackages(sysHideUseProxyPackages());
static QStringList cantUseProxyPackages(sysCantUseProxyPackages());

const QStringList sysHideOpenPackages()
{
    QStringList hideOpen_list;
    //从gschema读取隐藏打开功能软件列表
    if (gSetting && gSetting->keys().contains("appsHideOpenList")) {
        hideOpen_list << gSetting->get("apps-hide-open-list").toStringList();
    }

    return hideOpen_list;
}

const QStringList sysHideSendToDesktopPackages()
{
    QStringList hideSendToDesktop_list;
    //从gschema读取隐藏发送到桌面功能软件列表
    if (gSetting && gSetting->keys().contains("appsHideSendToDesktopList")) {
        hideSendToDesktop_list << gSetting->get("apps-hide-send-to-desktop-list").toStringList();
    }

    return hideSendToDesktop_list;
}

const QStringList sysHideSendToDockPackages()
{
    QStringList hideSendToDock_list;
    //从gschema读取隐藏发送到Ｄock功能软件列表
    if (gSetting && gSetting->keys().contains("appsHideSendToDockList")) {
        hideSendToDock_list << gSetting->get("apps-hide-send-to-dock-list").toStringList();
    }

    return hideSendToDock_list;
}

const QStringList sysHideStartUpPackages()
{
    QStringList hideStartUp_list;
    //从gschema读取隐藏开机启动功能软件列表
    if (gSetting && gSetting->keys().contains("appsHideStartUpList")) {
        hideStartUp_list << gSetting->get("apps-hide-start-up-list").toStringList();
    }

    return hideStartUp_list;
}

const QStringList sysHideUninstallPackages()
{
    QStringList hideUninstall_list;
    //从gschema读取隐藏开机启动功能软件列表
    if (gSetting && gSetting->keys().contains("appsHideUninstallList")) {
        hideUninstall_list << gSetting->get("apps-hide-uninstall-list").toStringList();
    }

    return hideUninstall_list;
}

const QStringList sysHideUseProxyPackages()
{
    QStringList hideUseProxy_list;
    //从gschema读取隐藏使用代理功能软件列表
    if (gSetting && gSetting->keys().contains("appsHideUseProxyList")) {
        hideUseProxy_list << gSetting->get("apps-hide-use-proxy-list").toStringList();
    }

    QObject::connect(gSetting, &QGSettings::changed, [ & ](const QString &key) {
        if (!key.compare("appsHideUseProxyList"))
            hideUseProxyPackages = sysHideUseProxyPackages();
    });

    return hideUseProxy_list;
}

const QStringList sysCantUseProxyPackages()
{
    QStringList cantUseProxy_list;
    //从gschema读取隐藏使用代理功能软件列表
    if (gSetting && gSetting->keys().contains("appsCanNotUseProxyList")) {
        cantUseProxy_list << gSetting->get("apps-can-not-use-proxy-list").toStringList();
    }

    QObject::connect(gSetting, &QGSettings::changed, [ & ](const QString &key) {
        if (!key.compare("appsCanNotUseProxyList"))
            cantUseProxyPackages = sysCantUseProxyPackages();
    });

    return cantUseProxy_list;
}

const QStringList sysCantOpenPackages()
{
    QStringList cantOpen_list;
    //从gschema读取不可打开软件列表
    if (gSetting && gSetting->keys().contains("appsCanNotOpenList")) {
        cantOpen_list << gSetting->get("apps-can-not-open-list").toStringList();
    }

    return cantOpen_list;
}

const QStringList sysCantSendToDesktopPackages()
{
    QStringList cantSendToDesktop_list;
    //从gschema读取不可发送到桌面软件列表
    if (gSetting && gSetting->keys().contains("appsCanNotSendToDesktopList")) {
        cantSendToDesktop_list << gSetting->get("apps-can-not-send-to-desktop-list").toStringList();
    }

    return cantSendToDesktop_list;
}

const QStringList sysCantSendToDockPackages()
{
    QStringList cantSendToDock_list;
    //从gschema读取不可发送到Dock软件列表
    if (gSetting && gSetting->keys().contains("appsCanNotSendToDockList")) {
        cantSendToDock_list << gSetting->get("apps-can-not-send-to-dock-list").toStringList();
    }

    return cantSendToDock_list;
}

const QStringList sysCantStartUpPackages()
{
    QStringList cantStartUp_list;
    //从gschema读取不可自动启动软件列表
    if (gSetting &&gSetting->keys().contains("appsCanNotStartUpList")) {
        cantStartUp_list << gSetting->get("apps-can-not-start-up-list").toStringList();
    }

    return cantStartUp_list;
}

const QStringList sysHoldPackages()
{
    //从先/etc/deepin-installer.conf读取不可卸载软件列表
    const QSettings settings("/etc/deepin-installer.conf", QSettings::IniFormat);
    auto holds_list = settings.value("dde_launcher_hold_packages").toStringList();

    //再从gschema读取不可卸载软件列表
    if (gSetting && gSetting->keys().contains("appsHoldList")) {
        holds_list << gSetting->get("apps-hold-list").toStringList();
    }

    return holds_list;
}

AppsListModel::AppsListModel(const AppCategory &category, QObject *parent)
    : QAbstractListModel(parent)
    , m_appsManager(AppsManager::instance())
    , m_actionSettings(SettingsPtr("com.deepin.dde.launcher.menu", "/com/deepin/dde/launcher/menu/", this))
    , m_calcUtil(CalculateUtil::instance())
    , m_hideOpenPackages(sysHideOpenPackages())
    , m_hideSendToDesktopPackages(sysHideSendToDesktopPackages())
    , m_hideSendToDockPackages(sysHideSendToDockPackages())
    , m_hideStartUpPackages(sysHideStartUpPackages())
    , m_hideUninstallPackages(sysHideUninstallPackages())
    , m_cantOpenPackages(sysCantOpenPackages())
    , m_cantSendToDesktopPackages(sysCantSendToDesktopPackages())
    , m_cantSendToDockPackages(sysCantSendToDockPackages())
    , m_cantStartUpPackages(sysCantStartUpPackages())
    , m_holdPackages(sysHoldPackages())
    , m_category(category)
    , m_drawBackground(true)
    , m_pageIndex(0)
{
    connect(m_appsManager, &AppsManager::dataChanged, this, &AppsListModel::dataChanged);
    connect(m_appsManager, &AppsManager::itemDataChanged, this, &AppsListModel::itemDataChanged);
}

void AppsListModel::setCategory(const AppsListModel::AppCategory category)
{
    m_category = category;

    emit QAbstractListModel::layoutChanged();
}

/**
 * @brief AppsListModel::setDraggingIndex 保存当前拖动的item对应的模型索引
 * @param index 拖动的item对应的模型索引
 */
void AppsListModel::setDraggingIndex(const QModelIndex &index)
{
    m_dragStartIndex = index;
    m_dragDropIndex = index;

    emit QAbstractListModel::dataChanged(index, index);
}

void AppsListModel::setDragDropIndex(const QModelIndex &index)
{
    if (m_dragDropIndex == index)
        return;

    m_dragDropIndex = index;

    emit QAbstractListModel::dataChanged(m_dragStartIndex, index);
}

/**
 * @brief AppsListModel::dropInsert 插入拖拽后的item
 * @param appKey item应用对应的key值
 * @param pos item 拖拽释放后所在的行数
 */
void AppsListModel::dropInsert(const QString &desktop, const int pos)
{
    beginInsertRows(QModelIndex(), pos, pos);
    int appPos = pos;
    if ((m_category == AppsListModel::FullscreenAll) || (m_category == AppsListModel::Dir))
        appPos = m_pageIndex * m_calcUtil->appPageItemCount(m_category) + pos;

    m_appsManager->restoreItem(desktop, m_category, appPos);
    endInsertRows();
}

/**在当前模型指定的位置插入应用
 * @brief AppsListModel::insertItem
 * @param pos 插入应用的行数
 */
void AppsListModel::insertItem(int pos)
{
    beginInsertRows(QModelIndex(), pos, pos);

    pos += m_appsManager->pageIndex() * m_calcUtil->appPageItemCount(m_appsManager->category());

    m_appsManager->insertDropItem(pos);
    endInsertRows();
}

void AppsListModel::insertItem(const ItemInfo_v1 &item, const int pos)
{
    beginInsertRows(QModelIndex(), pos, pos);
    m_appsManager->dropToCollected(item, pos);
    endInsertRows();
}

/**
 * @brief AppsListModel::dropSwap 拖拽释放后删除被拖拽的item，插入移动到新位置的item
 * @param nextPos 拖拽释放后的位置
 */
void AppsListModel::dropSwap(const int nextPos)
{
    if (!m_dragStartIndex.isValid())
        return;

    const ItemInfo_v1 &appInfo = m_dragStartIndex.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    // 从文件夹展开窗口移除应用时，文件夹本身不执行移动操作
    if (m_appsManager->dragMode() != AppsManager::DirOut) {
        removeRows(m_dragStartIndex.row(), 1, QModelIndex());
        dropInsert(appInfo.m_desktop, nextPos);
    }

    emit QAbstractItemModel::dataChanged(m_dragStartIndex, m_dragDropIndex);

    m_dragStartIndex = m_dragDropIndex = index(nextPos);
}

/**
 * @brief AppsListModel::clearDraggingIndex
 * 重置拖拽过程中的模型索引并触发更新列表数据信号
 */
void AppsListModel::clearDraggingIndex()
{
    const QModelIndex startIndex = m_dragStartIndex;
    const QModelIndex endIndex = m_dragDropIndex;

    m_dragStartIndex = m_dragDropIndex = QModelIndex();

    emit QAbstractItemModel::dataChanged(startIndex, endIndex);
}


/**
 * @brief AppsListModel::rowCount
 * 全屏时返回当前页面item的个数
 * @param parent 父节点模式索引
 * @return 返回当前页面item的个数
 */
int AppsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    int nSize = m_appsManager->appsInfoListSize(m_category);
    int pageCount = m_calcUtil->appPageItemCount(m_category);
    int nPageCount = nSize - pageCount * m_pageIndex;
    nPageCount = nPageCount > 0 ? nPageCount : 0;

    if(!m_calcUtil->fullscreen())
        return nSize;

    if (m_category == AppsListModel::Favorite || (m_category == AppsListModel::Search)
                              || (m_category == AppsListModel::PluginSearch))
        return nSize;

    return qMin(pageCount, nPageCount);
}

/**
 * @brief AppsListModel::indexAt 根据appkey值返回app所在的模型索引
 * @param appKey app key
 * @return 根据appkey值,返回app对应的模型索引
 */
const QModelIndex AppsListModel::indexAt(const QString &desktopPath) const
{
    int i = 0;
    const int count = rowCount();
    while (i < count) {
        if (index(i).data(AppDesktopRole).toString() == desktopPath)
            return index(i);

        ++i;
    }

    return QModelIndex();
}

void AppsListModel::setDrawBackground(bool draw)
{
    if (draw == m_drawBackground) return;

    m_drawBackground = draw;

    emit QAbstractItemModel::dataChanged(QModelIndex(), QModelIndex());
}

void AppsListModel::updateModelData(const QModelIndex dragIndex, const QModelIndex dropIndex)
{
    // 保存数据到本地列表
    m_appsManager->updateUsedSortData(dragIndex, dropIndex);

    // 保存列表数据到配置文件
    m_appsManager->saveFullscreenUsedSortedList();

    // 2022年 10月 18日 星期二 11:41:36 CST
    // appgridview.cpp中本来就有通过信号槽方式连接的逻辑，先移除后插入。无须单独安排接口处理。

    // 通知界面更新翻页控件
    emit m_appsManager->dataChanged(AppsListModel::FullscreenAll);
    emit QAbstractItemModel::dataChanged(dragIndex, dropIndex);
}

/**
 * @brief AppsListModel::removeRows 从模式中移除1个item
 * @param row item所在的行
 * @param count 移除的item个数
 * @param parent 父节点的模型索引
 * @return 返回移除状态标识
 */
bool AppsListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)

    if (count > 1) {
        qDebug() << "AppsListModel does't support removing multiple rows!";
        return false;
    }

    beginRemoveRows(parent, row, row);
    m_appsManager->dragdropStashItem(index(row), m_category);
    endRemoveRows();

    return true;
}

/**
 * @brief AppsListModel::canDropMimeData 在搜索模式和无效的拖动模式下item不支持拖拽
 * @param data 当前拖拽的item的mime类型数据
 * @param action 拖拽实现的动作
 * @param row 当前拖拽的item所在行
 * @param column 当前拖拽的item所在列
 * @param parent 当前拖拽的item父节点模型索引
 * @return 返回是否item是否支持拖动的标识
 */
bool AppsListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    // disable invalid drop
    if (data->data("RequestDock").isEmpty())
        return false;

    // 全屏搜索模式、小窗口所有应用列表不支持drop
    if (m_category == Search || m_category == WindowedAll) {
        return  false;
    }

    return true;
}

/**
 * @brief AppsListModel::mimeData 给拖动的item设置mine类型数据
 * @param indexes 拖动的item对应的模型索引
 * @return 返回拖动的item的mine类型数据
 */
QMimeData *AppsListModel::mimeData(const QModelIndexList &indexes) const
{
    // only allow drag 1 item
    Q_ASSERT(indexes.size() == 1);

    const QModelIndex index = indexes.first();

    QMimeData *mime = new QMimeData;

    // 拖动应用到任务栏驻留针对不同应用提供配置功能, 默认为启用
    const QString &appKey = index.data(AppKeyRole).toString();

    if (!ConfigWorker::getValue(DLauncher::UNABLE_TO_DOCK_LIST, QStringList()).toStringList().contains(appKey))
        mime->setData("RequestDock", index.data(AppDesktopRole).toByteArray());

    mime->setData("DesktopPath", index.data(AppDesktopRole).toByteArray());
    mime->setImageData(index.data(AppsListModel::AppRawItemInfoRole));
    if (index.data(AppIsRemovableRole).toBool())
        mime->setData("Removable", "");

    // this object will be delete in drag event finished.
    return mime;
}

/**
 * @brief AppsListModel::data 获取给定模型索引和数据角色的item数据
 * @param index item对应的模型索引
 * @param role item对应的数据角色
 * @return 返回item相关的数据
 */
QVariant AppsListModel::data(const QModelIndex &index, int role) const
{
    ItemInfo_v1 itemInfo = ItemInfo_v1();
    int nSize = m_appsManager->appsInfoListSize(m_category);
    int nFixCount = m_calcUtil->appPageItemCount(m_category);
    int pageCount = qMin(nFixCount, nSize - nFixCount * m_pageIndex);

    if(!m_calcUtil->fullscreen()) {
        pageCount = nSize;
        if (m_category == TitleMode)
            itemInfo = m_appsManager->appsCategoryListIndex(index.row());
        else if (m_category == LetterMode) {
            itemInfo = m_appsManager->appsLetterListIndex(index.row());
        } else if (m_category == WindowedAll) {
            itemInfo = m_appsManager->appsInfoListIndex(m_category, index.row());
        } else if (m_category == Favorite) {
            itemInfo = m_appsManager->appsInfoListIndex(m_category, index.row());
        } else if ((m_category == Search) || (m_category == PluginSearch)) {
            itemInfo = m_appsManager->appsInfoListIndex(m_category, index.row());
        }
    } else {
        if ((m_category == Search) || (m_category == PluginSearch)) {
            // 保证搜索原数据模型中的数据未经过翻页处理
            pageCount = nSize;
            itemInfo = m_appsManager->appsInfoListIndex(m_category, index.row());
        } else {
            int start = nFixCount * m_pageIndex;
            itemInfo = m_appsManager->appsInfoListIndex(m_category, start + index.row());
        }
    }

    if (!index.isValid() || index.row() >= pageCount)
        return QVariant();

    switch (role) {
    case AppRawItemInfoRole:
        return QVariant::fromValue(itemInfo);
    case AppNameRole:
        return m_appsManager->appName(itemInfo, 240);
    case AppDesktopRole:
        return itemInfo.m_desktop;
    case AppKeyRole:
        return itemInfo.m_key;
    case AppGroupRole:
        return QVariant::fromValue(m_category);
    case AppAutoStartRole:
        return m_appsManager->appIsAutoStart(itemInfo.m_desktop);
    case AppIsOnDesktopRole:
        return m_appsManager->appIsOnDesktop(itemInfo.m_key);
    case AppIsOnDockRole:
        return m_appsManager->appIsOnDock(itemInfo.m_desktop);
    case AppIsRemovableRole:
        return !m_holdPackages.contains(itemInfo.m_key);
    case AppIsProxyRole:
        return m_appsManager->appIsProxy(itemInfo.m_key);
    case AppEnableScalingRole:
        return m_appsManager->appIsEnableScaling(itemInfo.m_key);
    case AppNewInstallRole:
        return m_appsManager->appIsNewInstall(itemInfo.m_key);
    case AppIconRole:
        return m_appsManager->appIcon(itemInfo, m_calcUtil->appIconSize(m_category).width());
    case AppDialogIconRole:
        return m_appsManager->appIcon(itemInfo, DLauncher::APP_DLG_ICON_SIZE);
    case AppDragIconRole:
        return m_appsManager->appIcon(itemInfo, m_calcUtil->appIconSize(m_category).width() * 1.2);
    case AppListIconRole: {
        QSize iconSize = m_calcUtil->appIconSize(m_category);
        return m_appsManager->appIcon(itemInfo, iconSize.width());
    }
    case ItemSizeHintRole:
        return m_calcUtil->appItemSize();
    case AppIconSizeRole:
        return m_calcUtil->appIconSize(m_category);
    case AppFontSizeRole:
        return DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6);
    case AppItemIsDraggingRole:
        return indexDragging(index);
    case DrawBackgroundRole:
        return m_drawBackground;
    case AppHideOpenRole:
        return (m_actionSettings && !m_actionSettings->get("open").toBool()) || m_hideOpenPackages.contains(itemInfo.m_key);
    case AppHideSendToDesktopRole:
        return (m_actionSettings && !m_actionSettings->get("send-to-desktop").toBool()) || m_hideSendToDesktopPackages.contains(itemInfo.m_key);
    case AppHideSendToDockRole:
        return (m_actionSettings && !m_actionSettings->get("send-to-dock").toBool()) || m_hideSendToDockPackages.contains(itemInfo.m_key);
    case AppHideStartUpRole:
        return (m_actionSettings && !m_actionSettings->get("auto-start").toBool()) || m_hideStartUpPackages.contains(itemInfo.m_key);
    case AppHideUninstallRole:
        return (m_actionSettings && !m_actionSettings->get("uninstall").toBool()) || m_hideUninstallPackages.contains(itemInfo.m_key);
    case AppHideUseProxyRole: {
        bool hideUse = ((m_actionSettings && !m_actionSettings->get("use-proxy").toBool()) || hideUseProxyPackages.contains(itemInfo.m_key));
        return DSysInfo::isCommunityEdition() ? hideUse : (!QFile::exists(ChainsProxy_path) || hideUse);
    }
    case AppCanOpenRole:
        return !m_cantOpenPackages.contains(itemInfo.m_key);
    case AppCanSendToDesktopRole:
        return !m_cantSendToDesktopPackages.contains(itemInfo.m_key);
    case AppCanSendToDockRole:
        return !m_cantSendToDockPackages.contains(itemInfo.m_key);
    case AppCanStartUpRole:
        return !m_cantStartUpPackages.contains(itemInfo.m_key);
    case AppCanOpenProxyRole:
        return !cantUseProxyPackages.contains(itemInfo.m_key);
    case ItemIsDirRole:
        return itemInfo.m_isDir;
    case DirItemInfoRole:
        return QVariant::fromValue(itemInfo.m_appInfoList);
    case DirAppIconsRole: {
        QList<QPixmap> pixmapList = m_appsManager->getDirAppIcon(index);
        return QVariant::fromValue(pixmapList);
    }
    case AppItemTitleRole:
        return itemInfo.m_iconKey.isEmpty();
    case DirNameRole: {
        const ItemInfo_v1 info = m_appsManager->createOfCategory(itemInfo.m_categoryId);
        return QVariant::fromValue(info);
    }
    case AppItemStatusRole:
        return itemInfo.m_status;
    case AppIsInFavoriteRole: {
        const ItemInfoList_v1 list = m_appsManager->appsInfoList(AppsListModel::Favorite);
        return list.contains(itemInfo);
    }
    default:
        break;
    }

    return QVariant();
}

/**
 * @brief AppsListModel::flags 获取给定模型索引的item的属性
 * @param index item对应的模型索引
 * @return 返回模型索引对应的item的属性信息
 */
Qt::ItemFlags AppsListModel::flags(const QModelIndex &index) const
{
    const Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    // 全屏、收藏列表支持拖拽
    if (m_category == FullscreenAll || m_category == Favorite)
        return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;

    return defaultFlags;
}

///
/// \brief AppsListModel::dataChanged tell view the appManager data is changed
/// \param category data category
///
void AppsListModel::dataChanged(const AppCategory category)
{
    if (category == FullscreenAll || category == m_category)
        emit QAbstractItemModel::layoutChanged();
}

///
/// \brief AppsListModel::layoutChanged tell view the app layout is changed, such as appItem size, icon size, etc.
/// \param category data category
///
void AppsListModel::layoutChanged(const AppsListModel::AppCategory category)
{
    if (category == FullscreenAll || category == m_category)
        emit QAbstractItemModel::dataChanged(QModelIndex(), QModelIndex());
}

/**
 * @brief AppsListModel::indexDragging 区分无效拖动和有效拖动
 * @param index 拖动的item对应的模型索引
 * @return 返回item拖动有效性的标识
 */
bool AppsListModel::indexDragging(const QModelIndex &index) const
{
    if (!m_dragStartIndex.isValid() || !m_dragDropIndex.isValid())
        return false;

    const int start = m_dragStartIndex.row();
    const int end = m_dragDropIndex.row();
    const int current = index.row();

    return (start <= end && current >= start && current <= end) ||
            (start >= end && current <= start && current >= end);
}

/**
 * @brief AppsListModel::itemDataChanged item数据变化时触发模型内部信号
 * @param info 数据发生变化的item信息
 */
void AppsListModel::itemDataChanged(const ItemInfo_v1 &info)
{
    int i = 0;
    const int count = rowCount();
    while (i != count) {
        if (index(i).data(AppKeyRole).toString() == info.m_key) {
            const QModelIndex modelIndex = index(i);
            emit QAbstractItemModel::dataChanged(modelIndex, modelIndex);
            return;
        }
        ++i;
    }
}
