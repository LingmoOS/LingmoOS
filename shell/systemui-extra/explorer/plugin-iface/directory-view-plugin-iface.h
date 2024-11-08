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
 *
 */

#ifndef DIRECTORYVIEWPLUGINIFACE_H
#define DIRECTORYVIEWPLUGINIFACE_H

#include "plugin-iface.h"

#include <QString>

#define DirectoryViewPluginIface_iid "org.lingmo.explorer-qt.plugin-iface.DirectoryViewPluginInterface"

namespace Peony {

class DirectoryViewIface;
class DirectoryViewProxyIface;

class FileItemModel;
class FileItemProxyFilterSortModel;

/*!
 * \brief The DirectoryViewPluginIface class
 * \details
 * It is a few complex to implement a explorer-qt's directory view's plugin.
 * There are 3 interface class must to be implemented.
 * 1. DirectoryViewPluginIface:
 * This is the entry of a plugin, explorer-qt's application will dynamicly
 * load external view modules when it started. After a plugin loaded
 * it will be registered in explorer-qt's application globally, then we can
 * create such a view by create();
 * \see DirectoryViewFactoryManager
 *
 * 2. DirectoryViewIface:
 * This class define a set of methods a dervied class of directory view
 * must implement. a dervied view could most be a dervied QAbstractItemView,
 * such as QListView or QTreeView, but not certain.
 * the directory view must implement these virtual method and make them
 * seem worked in there own implement.
 * \see DirectoryViewIface
 *
 * 3. DirectoryViewProxyIface
 * \see DirectoryViewProxyIface
 *
 * \deprecated
 * This interface is deprecated, and it should be removed from codes.
 *
 * To make a directory view extension,
 * you have to implement DirectoryViewPluginIface2, not this.
 *
 * \see
 * DirectoryViewPluginIface2
 */
class DirectoryViewPluginIface : public PluginInterface
{
public:
    virtual ~DirectoryViewPluginIface() {}

    virtual QString viewIdentity() = 0;
    virtual QIcon viewIcon() = 0;
    virtual bool supportUri(const QString &uri) = 0;

    /*!
     * \brief zoom_level_hint
     * \return the description value of this type of view's zoom level.
     * \details
     * Peony-Qt doesn't deal the zoom level in a kind of view. Insteadly,
     * there is a value for a view for describing its zoom level. Through
     * this zoom level, Peony-Qt can realize "zoom in/out" by switch the view's
     * type.
     *
     * We usually use the Peony-Qt's default icon view. Its zoom level hint is 100.
     * If you want to implement a larger directory view extension you should set the
     * zoom level hint bigger than the default one, vice versa.
     */
    virtual int zoom_level_hint() = 0;

    /*!
     * \brief priority
     * \return the priority of view that should be show when went to a location.
     * \details
     * In Peony-Qt, there are some special view should be shown when we went to some
     * special directory, such as computer, it should be shown as a computer view.
     * Peony-Qt first judge if it should switch the view if there is a higher-priority
     * view than current view when the location change begin.
     *
     * The default icon view and details view's priority value is 0. If you want to
     * implement an extension view replace the current view in some location, you should
     * set the priority value larger than the default one.
     */
    virtual int priority(const QString &directoryUri) = 0;

    virtual DirectoryViewIface *create() = 0;
};

/*!
 * \brief The DirectoryViewIface class
 * \details
 * This class define a set of interface for view operation.
 * In explorer-qt the directory view is desgin to be extensive,
 * and every kind of view will have a global factory to create
 * the view's instance. the factories will be managed by explorer-qt
 * in global, too.
 * A explorer-qt's file item view will implement this interface.
 * \note
 * Every DirectoryViewIface instance should have a DirectoryViewProxyIface
 * instance, we usually use this proxy control the view.
 * \see DirectoryViewProxyIface
 *
 * \note
 * This interface is useless for implement of new directory view extensions, but
 * it will be keeped for fitting translations about old view (IconView & ListView) and view's factory to
 * new view (IconView2 & ListView2) and factory.
 *
 * To make a directory view extension,
 * you have to implement DirectoryViewPluginIface2, not this.
 *
 * \see
 * DirectoryViewPluginIface2
 */
class DirectoryViewIface
{

public:
    virtual ~DirectoryViewIface() {}

    virtual void bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel) = 0;
    virtual void setProxy(DirectoryViewProxyIface *proxy) = 0;

    const virtual QString viewId() = 0;

    //location
    const virtual QString getDirectoryUri() = 0;

    //selections
    const virtual QStringList getSelections() = 0;

    //children
    const virtual QStringList getAllFileUris() = 0;

    /*
    //loaction
    void openRequest(const QStringList &uri, bool newWindow);
    void viewDirectoryChanged();
    void viewSelectionChanged();

    //menu
    void menuRequest(const QPoint &pos);

    //clipboard
    void cutSelectionsRequest();
    void copySelectionsRequest();
    void pasteSelectionsRequest();

    //zoom
    void zoomedIn();
    void zoomedOut();
    */

    //location
    virtual void open(const QStringList &uris, bool newWindow) = 0;
    virtual void setDirectoryUri(const QString &uri) = 0;
    virtual void beginLocationChange() = 0;
    virtual void stopLocationChange() = 0;
    virtual void closeView() = 0;

    //selections
    virtual void setSelections(const QStringList &uris) = 0;
    virtual void invertSelections() = 0;
    virtual void scrollToSelection(const QString &uri) = 0;

    //clipboard
    virtual void setCutFiles(const QStringList &uris) = 0;

    virtual DirectoryViewProxyIface *getProxy() = 0;

    virtual int getSortType() = 0;
    virtual void setSortType(int sortType) = 0;

    virtual int getSortOrder() = 0;
    virtual void setSortOrder(int sortOrder) = 0;

    //edit
    virtual void editUri(const QString &uri) = 0;
    /*!
     * \brief editUris
     * \param uris
     * \todo
     * implement batch rename
     */
    virtual void editUris(const QStringList uris) = 0;
};

/*!
 * \brief The DirectoryViewProxyIface class
 * \deprecated
 * This interface is deprecated, and it should be removed from codes.
 * To make a directory view extension,
 * you have to implement DirectoryViewPluginIface2, not this.
 *
 * \see
 * DirectoryViewPluginIface2
 */
class DirectoryViewProxyIface: public QObject
{
    Q_OBJECT
public:
    explicit DirectoryViewProxyIface(QObject *parent = nullptr) : QObject(parent) {}
    ~DirectoryViewProxyIface() {}

    virtual void switchView(DirectoryViewIface *view) = 0;

    virtual DirectoryViewIface *getView() = 0;

    //location
    const virtual QString getDirectoryUri() = 0;

    //selections
    const virtual QStringList getSelections() = 0;

    //children
    const virtual QStringList getAllFileUris() = 0;

Q_SIGNALS:
    //loaction
    //FIXME: support open in new TAB?
    void openRequest(const QStringList &uri, bool newWindow);
    void viewDoubleClicked(const QString &uri);
    void viewDirectoryChanged();
    void viewSelectionChanged();

    //menu
    void menuRequest(const QPoint &pos);

    //window
    void updateWindowLocationRequest(const QString &uri);

public Q_SLOTS:
    //location
    virtual void open(const QStringList &uris, bool newWindow) = 0;
    virtual void setDirectoryUri(const QString &uri) = 0;
    virtual void beginLocationChange() = 0;
    virtual void stopLocationChange() = 0;

    virtual void closeProxy() = 0;

    //selections
    virtual void setSelections(const QStringList &uris) = 0;
    virtual void invertSelections() = 0;
    virtual void scrollToSelection(const QString &uri) = 0;

    //clipboard
    //cut items should be drawn differently.
    virtual void setCutFiles(const QStringList &uris) = 0;

    virtual void setSortType(int sortType) = 0;
    virtual void setSortOrder(int sortOrder) = 0;

    virtual void editUri(const QString &uri) = 0;
    virtual void editUris(const QStringList uris) = 0;

    virtual int getSortType() = 0;
    virtual int getSortOrder() = 0;
};

}

Q_DECLARE_INTERFACE(Peony::DirectoryViewPluginIface, DirectoryViewPluginIface_iid)

#endif // DIRECTORYVIEWPLUGINIFACE_H
