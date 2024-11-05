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

#ifndef INTEL_COMPUTERVIEWCONTAINER_H
#define INTEL_COMPUTERVIEWCONTAINER_H

#include <QWidget>
#include <explorer-qt/directory-view-plugin-iface2.h>
#include <explorer-qt/controls/directory-view/directory-view-widget.h>

#include <gio/gio.h>

namespace Intel {
class ComputerView;
}

namespace Intel {

class ComputerViewContainer : public Peony::DirectoryViewWidget
{
    Q_OBJECT
public:
    explicit ComputerViewContainer(QWidget *parent = nullptr);
    ~ComputerViewContainer();

    const virtual QString viewId() {return "Computer View";}

    //location
    const virtual QString getDirectoryUri() {return "computer:///";}

    //selections
    const virtual QStringList getSelections();

    //children
    const virtual QStringList getAllFileUris() {return QStringList();}

    virtual int getSortType() {return 0;}
    virtual Qt::SortOrder getSortOrder() {return Qt::AscendingOrder;}

    //zoom
    virtual int currentZoomLevel() {return -1;}
    virtual int minimumZoomLevel() {return -1;}
    virtual int maximumZoomLevel() {return -1;}
    virtual bool supportZoom() {return false;}

protected:
    void paintEvent(QPaintEvent *e);

    void keyPressEvent(QKeyEvent *e);

public Q_SLOTS:
    virtual void bindModel(Peony::FileItemModel *model, Peony::FileItemProxyFilterSortModel *proxyModel);

    //location
    //virtual void open(const QStringList &uris, bool newWindow) {}
    virtual void setDirectoryUri(const QString &uri) {}
    virtual void beginLocationChange();
    virtual void stopLocationChange();

    virtual void closeDirectoryView() {}

    //selections
    virtual void setSelections(const QStringList &uris) {}
    virtual void invertSelections() {}
    virtual void scrollToSelection(const QString &uri) {}

    //clipboard
    //cut items should be drawn differently.
    virtual void setCutFiles(const QStringList &uris) {}

    virtual void setSortType(int sortType) {}
    virtual void setSortOrder(int sortOrder) {}

    virtual void editUri(const QString &uri) {}
    virtual void editUris(const QStringList uris) {}

    virtual void repaintView() {}

    virtual void clearIndexWidget() {}

    //zoom
    virtual void setCurrentZoomLevel(int zoomLevel) {}

public:
    QString m_remote_uri;

private:
    Intel::ComputerView *m_view = nullptr;
    GMountOperation* m_op = nullptr;
    QAction *m_enterAction = nullptr;
    Peony::FileItemModel *m_model = nullptr;
    Peony::FileItemProxyFilterSortModel *m_proxyModel = nullptr;
};

}

#endif // COMPUTERVIEWCONTAINER_H
