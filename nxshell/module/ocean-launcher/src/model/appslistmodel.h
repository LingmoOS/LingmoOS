// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include <QAbstractListModel>
#define MAXIMUM_POPULAR_ITEMS 11

class AppsManager;
class CalculateUtil;
class ItemInfo_v1;
class QGSettings;
class AppsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AppItemRole {
        ItemSizeHintRole = Qt::SizeHintRole,
        AppNameRole = Qt::DisplayRole,
        AppIconRole = Qt::DecorationRole,
        AppReserveRole = Qt::UserRole,

        // custom role start
        AppRawItemInfoRole,
        AppDialogIconRole,
        AppListIconRole,
        AppKeyRole,
        AppDesktopRole,
        AppGroupRole,
        AppAutoStartRole,
        AppNewInstallRole,
        AppIsOnDesktopRole,
        AppIsOnDockRole,
        AppIsRemovableRole,
        AppIsProxyRole,
        AppEnableScalingRole,
        AppIsInFavoriteRole,
        AppIconSizeRole,
        AppFontSizeRole,
        AppItemIsDraggingRole,
        ItemIsDirRole,
        DirItemInfoRole,
        DirAppIconsRole,
        DirNameRole,
        AppDragIconRole,
        AppItemTitleRole,
        AppItemStatusRole,
        ItemRatio,
        DrawBackgroundRole,
        AppHideOpenRole,
        AppHideSendToDesktopRole,
        AppHideSendToDockRole,
        AppHideStartUpRole,
        AppHideUninstallRole,
        AppHideUseProxyRole,
        AppCanOpenRole,
        AppCanSendToDesktopRole,
        AppCanSendToDockRole,
        AppCanStartUpRole,
        AppCanOpenProxyRole,
    };

    enum AppCategory {
        WindowedAll,            // 小窗口所有应用列表
        FullscreenAll,          // 全屏所有应用列表
        TitleMode,              // 标题模式
        LetterMode,             // 字母模式
        Search,                 // 本地搜索模式
        Favorite,               // 收藏应用
        Dir,                    // 应用抽屉
        PluginSearch,           // 插件搜索

        // apps category
        Internet,               // 网络模式
        Chat,                   // 社交模式
        Music,                  // 音乐模式
        Video,                  // 视频模式
        Graphics,               // 图形图像
        Game,                   //
        Office,                 // 办公模式
        Reading,                // 阅读模式
        Development,            // 编程开发模式
        System,                 // 系统管理模式
        Others,
    };

public:
    explicit AppsListModel(const AppCategory& category, QObject *parent = nullptr);
    void setPageIndex(int pageIndex) { m_pageIndex = pageIndex; }
    int getPageIndex() const { return m_pageIndex; }

    inline AppCategory category() const {return m_category;}
    void setDraggingIndex(const QModelIndex &index);
    void setDragDropIndex(const QModelIndex &index);
    void dropInsert(const QString &desktop, const int pos);
    void insertItem(int pos);
    void insertItem(const ItemInfo_v1 &item, const int pos);
    void dropSwap(const int nextPos);
    inline QModelIndex dragDropIndex() const {return m_dragDropIndex;}

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    const QModelIndex indexAt(const QString &appKey) const;

    void setDrawBackground(bool draw);

    void updateModelData(const QModelIndex dragIndex, const QModelIndex dropIndex);

public slots:
    void clearDraggingIndex();
    void setCategory(const AppCategory category);

protected:
    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void dataChanged(const AppsListModel::AppCategory category);
    void layoutChanged(const AppsListModel::AppCategory category);
    bool indexDragging(const QModelIndex &index) const;
    void itemDataChanged(const ItemInfo_v1 &info);

private:
    AppsManager *m_appsManager;
    QGSettings *m_actionSettings;
    CalculateUtil *m_calcUtil;

    QList<ItemInfo_v1> m_itemList;

    QStringList m_hideOpenPackages;
    QStringList m_hideSendToDesktopPackages;
    QStringList m_hideSendToDockPackages;
    QStringList m_hideStartUpPackages;
    QStringList m_hideUninstallPackages;

    QStringList m_cantOpenPackages;
    QStringList m_cantSendToDesktopPackages;
    QStringList m_cantSendToDockPackages;
    QStringList m_cantStartUpPackages;
    QStringList m_cantUseProxyPackages;
    QStringList m_holdPackages;

    QModelIndex m_dragStartIndex = QModelIndex();
    QModelIndex m_dragDropIndex = QModelIndex();
    AppCategory m_category;

    bool m_drawBackground;
    int m_pageIndex;
};
typedef QList<AppsListModel *> PageAppsModelist;

Q_DECLARE_METATYPE(AppsListModel::AppCategory)

#endif // APPSLISTMODEL_H
