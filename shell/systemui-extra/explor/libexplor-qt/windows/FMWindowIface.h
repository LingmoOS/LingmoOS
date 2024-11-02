/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef FMWINDOWIFACE_H
#define FMWINDOWIFACE_H

#include "explor-core_global.h"
#include <QStringList>
#include "file-info.h"

namespace Peony {

class DirectoryViewContainer;
class FMWindowFactory;

class PEONYCORESHARED_EXPORT FMWindowIface
{
public:
    /*!
     * \brief getFactory
     * \return
     * \deprecated
     */
    virtual FMWindowFactory *getFactory() {
        return nullptr;
    }

    virtual FMWindowIface *create(const QString &uri) = 0;
    virtual FMWindowIface *create(const QStringList &uris) = 0;

    virtual const QString getCurrentUri() = 0; //do not allow initialize this iface directly
    virtual const QString getLastNonSearchUri() {
        return nullptr;
    }
    virtual const QStringList getCurrentSelections() {
        return QStringList();
    }
    virtual const QStringList getCurrentAllFileUris() {
        return QStringList();
    }
    virtual const QList<std::shared_ptr<FileInfo>> getCurrentSelectionFileInfos() {
        return QList<std::shared_ptr<FileInfo>>();
    }
    virtual DirectoryViewContainer *getCurrentPage() {
        return nullptr;
    }

    virtual Qt::SortOrder getCurrentSortOrder() {
        return Qt::AscendingOrder;
    }
    virtual int getCurrentSortColumn() {
        return 0;
    }

    virtual bool getWindowShowHidden() {
        return false;
    }
    virtual bool getWindowUseDefaultNameSortOrder() {
        return false;
    }
    virtual bool getWindowSortFolderFirst() {
        return false;
    }

    virtual const QString getCurrentPageViewType() {
        return nullptr;
    }

    virtual int currentViewZoomLevel() {
        return -1;
    }
    virtual bool currentViewSupportZoom() {
        return false;
    }

    //slot
    virtual void goToUri(const QString &uri, bool addHistory, bool forceUpdate = false) {}
    virtual void addNewTabs(const QStringList &uris) {}

    virtual void beginSwitchView(const QString &viewId) {}

    virtual void refresh() {}
    virtual void forceStopLoading() {}
    virtual void advanceSearch() {}
    virtual void clearRecord() {}
    virtual void searchFilter(QString target_path, QString keyWord, bool search_file_name, bool search_content) {}
    virtual void filterUpdate(int type_index=0, int time_index=0, int size_index=0) {}

    virtual void setShowHidden(bool showHidden) {}
    virtual void setUseDefaultNameSortOrder(bool use) {}
    virtual void setSortFolderFirst(bool set) {}

    virtual void onPreviewPageSwitch(const QString &uri) {}

    virtual void setCurrentSelectionUris(const QStringList &uris) {}
    virtual void setCurrentSortOrder (Qt::SortOrder order) {}
    virtual void setCurrentSortColumn (int sortColumn) {}

    virtual void editUri(const QString &uri) {}
    virtual void editUris(const QStringList &uris) {}

    virtual void setCurrentViewZoomLevel(int zoomLevel) {}
    virtual bool getFilterWorking() {return false;}
};
}


#endif // FMWINDOWIFACE_H
