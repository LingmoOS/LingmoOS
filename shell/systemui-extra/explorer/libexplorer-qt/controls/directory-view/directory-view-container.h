/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#ifndef DIRECTORYVIEWCONTAINER_H
#define DIRECTORYVIEWCONTAINER_H

#include "explorer-core_global.h"
#include <QWidget>
#include <QStack>
#include <QGSettings>
#include <QTimer>
#include <QDir>
#include <QAbstractItemView>

#include "file-item-model.h"

class QVBoxLayout;

namespace Peony {

class FileItemModel;
class FileItemProxyFilterSortModel;

class DirectoryViewProxyIface;
class DirectoryViewWidget;

/*!
 * \brief The DirectoryViewContainer class
 * \details
 * This class define a set of higher level operation for the directory view,
 * such as history functions and view switch.
 * In explorer-qt, tab-window and view-type-switch are reserved from explorer.
 * This class is mainly to archive these higher ui logic.
 *
 * A directory view can not swith it type itself because it was created
 * from its own factory, and it aslo could and should not contain more details
 * implement above the view it self. Those should be implement by a wrapper class
 * in higher level.
 * You can interpret it as the middleware of window and directory view.
 */
class PEONYCORESHARED_EXPORT DirectoryViewContainer : public QWidget
{
    Q_OBJECT
public:
    explicit DirectoryViewContainer(QWidget *parent = nullptr);
    ~DirectoryViewContainer();

    const QString getCurrentUri();
    const QStringList getCurrentSelections();
    const int getCurrentRowcount();

    const QStringList getAllFileUris();

    const QStringList getBackList();
    const QStringList getForwardList();

    bool canGoBack();
    bool canGoForward();
    bool canCdUp();

    FileItemModel::ColumnType getSortType();
    Qt::SortOrder getSortOrder();

    //DirectoryViewProxyIface *getProxy() {return m_proxy;}
    DirectoryViewWidget *getView() {
        return m_view;
    }

Q_SIGNALS:
    void viewTypeChanged();
    void directoryChanged();
    void selectionChanged();
    void viewDoubleClicked(const QString &uri);
    void updateWindowLocationRequest(const QString &uri, bool addHistory, bool forceUpdate = false);
    void signal_responseUnmounted(const QString &destUri, const QString &sourceUri);

    void updateWindowSelectionRequest(const QStringList &uris);

    void menuRequest(const QPoint &pos);

    void zoomRequest(bool zoomIn);
    void setZoomLevelRequest(int zoomLevel);
    void updateStatusBarSliderStateRequest();
    void signal_itemAdded(const QString& uri);/* 新增文件（夹），item创建完成 */
    void updatePreviewPageRequest();
    void statusBarChanged();

public Q_SLOTS:
    void goToUri(const QString &uri, bool addHistory, bool forceUpdate = false);
    void switchViewType(const QString &viewId);

    void goBack();
    void goForward();
    void cdUp();

    void refresh();
    void stopLoading();

    void tryJump(int index);
    void clearHistory() {
        m_back_list.clear();
        m_forward_list.clear();
    }

    void setSortType(FileItemModel::ColumnType type);
    void setSortOrder(Qt::SortOrder order);

    void setSortFilter(int FileTypeIndex=0, int FileMTimeIndex=0, int FileSizeIndex=0);
    void setShowHidden(bool showHidden = false);
    void setShowFileExtensions(bool show = true);
    void setUseDefaultNameSortOrder(bool use);
    void setSortFolderFirst(bool folderFirst);
    void setFilterLabelConditions(QString name);

    //mutiple filter conditions for new advance search
    void addFileNameFilter(QString key, bool updateNow = false);
    void addFilterCondition(int option, int classify, bool updateNow = false);
    void removeFilterCondition(int option, int classify, bool updateNow = false);
    void clearConditions();
    void updateFilter();

    void onViewDoubleClicked(const QString &uri);

    void addFileDialogFiltersCondition(const QStringList &mimeTypeFilters, const QStringList &nameFilters, QDir::Filters dirFilters, Qt::CaseSensitivity caseSensitivity);
    void setSelectionMode(QAbstractItemView::SelectionMode mode);

    void updateCurrentFilesThumbnails();

protected:
    /*!
     * \brief bindNewProxy
     * \param proxy
     * \deprecated
     * This method is deprecated and useless. It should be removed.
     */
    void bindNewProxy(DirectoryViewProxyIface *proxy);

private:
    QString m_current_uri;

    DirectoryViewProxyIface *m_proxy = nullptr;
    DirectoryViewWidget *m_view = nullptr;

    QStringList m_back_list;
    QStringList m_forward_list;

    QVBoxLayout *m_layout;

    FileItemModel *m_model;
    FileItemProxyFilterSortModel *m_proxy_model;
    QGSettings* m_control_center_plugin = nullptr;
};

}

#endif // DIRECTORYVIEWCONTAINER_H
