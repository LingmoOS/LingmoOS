/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef ICONVIEW_H
#define ICONVIEW_H

#include "explor-core_global.h"
#include "directory-view-plugin-iface.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"

#include "directory-view-widget.h"
#include "global-settings.h"
#include "directoryviewhelper.h"

#include <QListView>
#include <QTimer>

namespace Peony {

namespace DirectoryView {

class IconViewDelegate;

class PEONYCORESHARED_EXPORT IconView : public QListView, public DirectoryViewIface, public DirectoryViewIface2
{
    friend class IconViewDelegate;
    friend class IconViewIndexWidget;
    friend class IconView2;
    Q_OBJECT
public:

    /*!
     * \brief IconView
     * \param parent
     * \deprecated
     * We should not create a proxy in a view itself. Proxy should be created by tabpage
     * or FMWindow.
     */
    explicit IconView(QWidget *parent = nullptr);
    explicit IconView(DirectoryViewProxyIface *proxy, QWidget *parent = nullptr);
    ~IconView() override;

    bool isDraggingState() {
        return this->state() == QListView::DraggingState || this->state() == QListView::DragSelectingState;
    }

    const QString viewId() override {
        return tr("Icon View");
    }

    void bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel) override;
    void setProxy(DirectoryViewProxyIface *proxy) override;

    /*!
     * \brief setUsePeonyQtDirectoryMenu
     * \param use
     * \deprecated
     */
    void setUsePeonyQtDirectoryMenu(bool use) {
        m_use_explor_qt_directory_menu = use;
    }

    /*!
     * \brief getProxy
     * \return
     * \deprecated
     */
    DirectoryViewProxyIface *getProxy() override;

    //location
    const QString getDirectoryUri() override;

    //selections
    const QStringList getSelections() override;

    //children
    const QStringList getAllFileUris() override;

    QRect visualRect(const QModelIndex &index) const override;

    bool getDelegateEditFlag();
    bool isEnableMultiSelect();

Q_SIGNALS:
    void zoomLevelChangedRequest(bool zoomIn);
    void updateSelectStatus(bool status);

public Q_SLOTS:
    //location
    void open(const QStringList &uris, bool newWindow) override;
    void setDirectoryUri(const QString &uri) override;
    void beginLocationChange() override;
    void stopLocationChange() override;
    void closeView() override;

    //selections
    void setSelections(const QStringList &uris) override;
    void invertSelections() override;
    void scrollToSelection(const QString &uri) override;

    //clipboard
    void setCutFiles(const QStringList &uris) override;

    int getSortType() override;
    void setSortType(int sortType) override;

    int getSortOrder() override;
    void setSortOrder(int sortOrder) override;

    void editUri(const QString &uri) override;
    void editUris(const QStringList uris) override;

    void selectAll() override;

    void resort();
    void reportViewDirectoryChanged();
    void clearIndexWidget();
    void multiSelect();
    void disableMultiSelect();
    void setSearchKey(const QString &key);
    void edit(const QModelIndex &index);
    void doMultiSelect(bool isMultiSlelect);

    void setItemsVisible(bool visible) override;

    const int getAllDisplayFileCount();

protected:
    /*!
     * \brief changeZoomLevel
     * \deprecated
     */
    void changeZoomLevel();

    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

    void wheelEvent(QWheelEvent *e) override;

    void updateGeometries() override;

    void focusInEvent(QFocusEvent *e) override;

    void startDrag(Qt::DropActions supportedActions) override;

    bool getIgnore_mouse_move_event() const;
    void setIgnore_mouse_move_event(bool ignore_mouse_move_event);
    void releaseUnselect(bool select);

    bool edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event) override;

    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event) const override;

protected Q_SLOTS:
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;

private Q_SLOTS:
    void slotRename();

private:
    void traverseNode();

    QTimer m_repaint_timer;

    bool  m_editValid;
    bool  m_ctrl_key_pressed = false;

    bool  m_increase = false;

    int m_scrollMax = 0;

    QTimer* m_renameTimer;

    QTimer *m_touch_active_timer = nullptr;

    QModelIndex m_last_index;

    DirectoryViewProxyIface *m_proxy = nullptr;

    FileItemModel *m_model = nullptr;
    FileItemProxyFilterSortModel *m_sort_filter_proxy_model = nullptr;

    QString m_current_uri = nullptr;

    /*!
     * \brief m_use_explor_qt_directory_menu
     * \deprecated
     */
    bool m_use_explor_qt_directory_menu = false;

    const int BOTTOM_STATUS_MARGIN = 36;

    bool m_ignore_mouse_move_event = false;

    bool m_delegate_editing = false;

    bool m_multi_select =false;
    bool m_allow_set_index_widget = true;

    bool m_slider_bar_draging = false;
    bool m_mouse_release_unselect = false;

    bool m_noSelectOnPress = false;
};

//IconView2
class IconView2 : public DirectoryViewWidget
{
    Q_OBJECT
    //internal plugin
public:
    explicit IconView2(QWidget *parent = nullptr);
    ~IconView2();

    const QString viewId() {
        return "Icon View";
    }

    //location
    const QString getDirectoryUri() {
        return m_view->getDirectoryUri();
    }

    //selections
    const QStringList getSelections() {
        return m_view->getSelections();
    }

    //children
    const QStringList getAllFileUris() {
        return m_view->getAllFileUris();
    }

    int getSortType() {
        return m_view->getSortType();
    }
    Qt::SortOrder getSortOrder() {
        return Qt::SortOrder(m_view->getSortOrder());
    }

    int currentZoomLevel() {
        return m_zoom_level;
    }
    int minimumZoomLevel() {
        return 41;
    }
    int maximumZoomLevel() {
        return 100;
    }
    bool supportZoom() {
        return Peony::GlobalSettings::getInstance()->getValue(ZOOM_SLIDER_VISIBLE).toBool();
    }

public Q_SLOTS:
    void bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel);

    //location
    //void open(const QStringList &uris, bool newWindow);
    void setDirectoryUri(const QString &uri) {
        m_view->setDirectoryUri(uri);
    }
    void beginLocationChange() {
        m_view->beginLocationChange();
    }
    void stopLocationChange() {
        m_view->stopLocationChange();
    }

    void closeDirectoryView() {
        m_view->closeView();
    }

    //selections
    void setSelections(const QStringList &uris) {
        m_view->setSelections(uris);
    }
    void invertSelections() {
        m_view->invertSelections();
    }
    void selectAll() {
        m_view->selectAll();
    }
    void scrollToSelection(const QString &uri) {
        m_view->scrollToSelection(uri);
    }

    //clipboard
    //cut items should be drawn differently.
    void setCutFiles(const QStringList &uris) {
        m_view->setCutFiles(uris);
    }

    void setSortType(int sortType) {
        m_view->setSortType(sortType);
    }
    void setSortOrder(int sortOrder) {
        m_view->setSortOrder(sortOrder);
    }

    void editUri(const QString &uri) {
        m_view->editUri(uri);
    }
    void editUris(const QStringList uris) {
        m_view->editUris(uris);
    }

    void repaintView();

    void setCurrentZoomLevel(int zoomLevel);

    void clearIndexWidget();
    void multiSelect(){
        m_view->multiSelect();
    }
    void disableMultiSelect(){
        m_view->disableMultiSelect();
    }

private:
    IconView *m_view = nullptr;
    FileItemModel *m_model = nullptr;
    FileItemProxyFilterSortModel *m_proxy_model = nullptr;

    int m_zoom_level = 70;
    bool m_menuRequesting = false;
};

}

}

#endif // ICONVIEW_H
