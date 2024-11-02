/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DESKTOPICONVIEW_H
#define DESKTOPICONVIEW_H

#include <QListView>
#include "directory-view-plugin-iface.h"
#include "explor-dbus-service.h"

#include <QStandardPaths>
#include <QTimer>

#include <QMap>

class QLabel;
class QGSettings;

namespace Peony {

class DesktopItemModel;
class DesktopItemProxyModel;
class PeonyDbusService;

class DesktopIconView : public QListView, public DirectoryViewIface
{
    friend class DesktopWindow;
    friend class DesktopIndexWidget;
    friend class DesktopIconViewDelegate;
    friend class DesktopItemModel;
    Q_OBJECT
public:
    enum ZoomLevel {
        Invalid,
        Small, //icon: 24x24; grid: 64x64; hover rect: 60x60; font: system*0.8
        Normal, //icon: 48x48; grid: 96x96; hover rect = 90x90; font: system
        Large, //icon: 64x64; grid: 115x135; hover rect = 105x118; font: system*1.2
        Huge //icon: 96x96; grid: 140x170; hover rect = 120x140; font: system*1.4

    };
    Q_ENUM(ZoomLevel)

    enum Direction {
        All,
        Right,
        Bottom
    };
    Q_ENUM(Direction)

    explicit DesktopIconView(QWidget *parent = nullptr);
    ~DesktopIconView();

    bool eventFilter(QObject *obj, QEvent *e);

    void initShoutCut();
    void initMenu();
    void initDoubleClick();

    void openFileByUri(QString uri);
    void restoreItemsPosByMetaInfo();

    void bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel) {
        Q_UNUSED(sourceModel) Q_UNUSED(proxyModel)
    }
    void setProxy(DirectoryViewProxyIface *proxy) {
        Q_UNUSED(proxy)
    }

    const QString viewId() {
        return tr("Desktop Icon View");
    }

    //location
    const QString getDirectoryUri() {
        return "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    void setId(int id) ;

    //selections
    const QStringList getSelections();

    //children
    const QStringList getAllFileUris();
    const int getAllDisplayFileCount();

    int getSortType();
    int getSortOrder();

    QRect visualRect(const QModelIndex &index) const;
    QPoint offset();
    QRect getViewRect();
    const QFont getViewItemFont(QStyleOptionViewItem *item);
    int updateBWList();
    QString getBlackAndWhiteModel();
    QSet<QString> getBWListInfo();
    bool getBlackAndWhiteListExist(QString name);

    void setRestoreInfo(QString &uri, QPoint &itemPos);
    void setAllRestoreInfo();
    void getAllRestoreInfo();
    void clearAllRestoreInfo();
    void refreshResolutionChange();

    void saveExtendItemInfo();
    void resetExtendItemInfo();
    void clearItemRect();
    bool isFull();
    void clearExtendItemPos(bool saveId = false);
    void UpdateToEditUris(QStringList uris);
    QRect getDataRect(const QModelIndex &index);

    // only used in model refresh.
    void clearCache();
    void modifyGridSize();
    void initViewport();
    bool verifyBoundaries(const QRect &rect, Direction direction);

    int radius() const;
    void setMarginsBasedOnPosition(int position, int margins);

    QRect visualRectInRightToLeft(const QModelIndex &index);
private:
    QRect getScreenArea(QScreen* screen);
    bool execSharedFileLink(const QString uri);

Q_SIGNALS:
    void zoomLevelChanged(ZoomLevel level);
    void updateView();
    void resetGridSize(const QSize &size);

public Q_SLOTS:
    //location
    void open(const QStringList &uris, bool newWindow) {}
    void setDirectoryUri(const QString &uri) {}
    void beginLocationChange() {}
    void stopLocationChange() {}
    void closeView();

    //selections
    void setSelections(const QStringList &uris);
    void invertSelections();
    void scrollToSelection(const QString &uri);

    //clipboard
    void setCutFiles(const QStringList &uris);

    DirectoryViewProxyIface *getProxy() {
        return nullptr;
    }

    void setSortType(int sortType);

    void setSortOrder(int sortOrder);

    //edit
    void editUri(const QString &uri);

    void editUris(const QStringList uris);

    void scrollTo(const QModelIndex &index, ScrollHint hint) override;

    //zoom
    void setDefaultZoomLevel(ZoomLevel level);
    ZoomLevel zoomLevel() const;
    void zoomIn();
    void zoomOut();

    void clearAllIndexWidgets(const QStringList &uris = QStringList());

    void refresh();

    void saveAllItemPosistionInfos();
    /*!
     * \brief saveItemPositionInfo
     * \param uri
     * \deprecated
     */
    void saveItemPositionInfo(const QString &uri);

    void resetAllItemPositionInfos();
    /*!
     * \brief resetItemPosistionInfo
     * \param uri
     * \deprecated
     */
    void resetItemPosistionInfo(const QString &uri);

    /*!
     * \brief updateItemPosistions
     * \param uri
     * \deprecated
     */
    void updateItemPosistions(const QString &uri = nullptr);

    QPoint getFileMetaInfoPos(const QString &uri);
    void setFileMetaInfoPos(const QString &uri, const QPoint &pos);

    /*!
     * \brief getCurrentItemRects
     * \return
     * \details used in both view and model. before we add/remove an item to
     * model, we should know current items layout.
     */
    QMap<QString, QRect> getCurrentItemRects();
    int removeItemRect(const QString &uri);

    void updateItemPosByUri(const QString &uri, const QPoint &pos);
    void ensureItemPosByUri(const QString &uri);

    void setShowHidden();
    /**
     * @brief Rearrange the desktop icon position
     * @param screenSize: The value of the screen size is the resolution minus the height of the control panel
     */
    void resolutionChange();
    void setEditFlag(bool edit);
    bool getEditFlag();

    void fileCreated(const QString &uri);

protected:
    int verticalOffset() const override;
    int horizontalOffset() const override;

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

    void startDrag(Qt::DropActions supportedActions);

    void wheelEvent(QWheelEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

    void focusOutEvent(QFocusEvent *e);

    void resizeEvent(QResizeEvent *e);

    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

    bool isItemsOverlapped();

    bool isRenaming();
    void setRenaming(bool renaming);

    const QRect getBoundingRect();

    void relayoutExsitingItems(const QStringList &uris);
    void checkItemsOver();
    bool dragToOtherScreen(QDropEvent *e);

    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event) const override;

private:
    ZoomLevel m_zoom_level = Invalid;
    QMargins m_panel_margin;

    QModelIndex m_last_index;
    QTimer m_edit_trigger_timer;

    DesktopItemModel *m_model = nullptr;
    DesktopItemProxyModel *m_proxy_model = nullptr;

    QStringList m_new_files_to_be_selected;
    QStringList m_uris_to_edit;/* 新建文件/文件夹，可编辑文件名list */
    QString m_edit_uri;    /* 正在编辑的文件，信息更新不重置*/

   // bool m_is_refreshing = false;

    bool m_real_do_edit = false;

    bool m_ctrl_or_shift_pressed = false;

    bool  m_ctrl_key_pressed = false;

    bool m_show_hidden;

    bool m_is_renaming = false;

    bool m_is_edit = false;

    bool m_initialized = false;

    QTimer m_refresh_timer;

    QModelIndexList m_drag_indexes;

    QMap<QScreen*, bool> m_screens;
    PeonyDbusService *m_explorDbusSer;
    QMap<QString, QRect> m_item_rect_hash;

    // remember items postions before resolution changed.
    QMap<QString, QRect> m_resolution_item_rect;

    QPoint m_press_pos;

    int m_id = 0;

    QStringList m_storageBox;

    int m_radius = 6;
    QGSettings *m_panelSetting = nullptr;

    bool m_noSelectOnPress = false;
};

}

#endif // DESKTOPICONVIEW_H
