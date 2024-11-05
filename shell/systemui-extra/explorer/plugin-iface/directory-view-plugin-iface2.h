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

#ifndef DIRECTORYVIEWPLUGINIFACE2_H
#define DIRECTORYVIEWPLUGINIFACE2_H

#include "plugin-iface.h"

#include <QWidget>

#define DirectoryViewPluginIface2_iid "org.lingmo.explorer-qt.plugin-iface.DirectoryViewPluginInterface2"

namespace Peony {

class DirectoryViewWidget;

/*!
 * \brief The DirectoryViewPluginIface2 class
 * \details
 * This class is a pure virtual interface class. It defines an extensionalbe
 * interface for making newly type item view for explorer.
 *
 * There are 2 kinds of internal implement at explorer, icon view and list view.
 *
 * \verbatim
 *
 * Directory View's Zoom In/Out:
 *
 * It is not cool that every view zoom in/out in itself at all.
 * The zoom of the view should be linked with the view type,
 * and the transition should be as smooth as possible.
 *
 * I defined some rules to indicate how explorer zoom its view.
 * there are 3 mainly parts:
 *
 * 1. the default zoom level hint of a view.
 * 2. the range for a view can be zoomed.
 * 3. wether a view supports zoomed.
 *
 * Zoom level has a range at 0 to 100. The hint should both contained
 * in the range and between view's min zoom level and max zoom level.
 *
 * Due to all view rebase the rules, there could be a common interface controling
 * the zoom of a window.
 *
 * For some special view, it might could not zoom, it better to indicate
 * with a clearly state for that case.
 *
 * \endverbatim
 *
 * \see IconView2, ListView2
 */
class DirectoryViewPluginIface2 : public PluginInterface
{
public:
    virtual ~DirectoryViewPluginIface2() {}

    virtual QString viewIdentity() = 0;
    virtual QString viewName() = 0;
    virtual QIcon viewIcon() = 0;
    virtual bool supportUri(const QString &uri) = 0;

    virtual int zoom_level_hint() = 0;
    virtual int minimumSupportedZoomLevel() = 0;
    virtual int maximumSupportedZoomLevel() = 0;

    virtual int priority(const QString &directoryUri) = 0;
    virtual bool supportZoom() = 0;

    //virtual void fillDirectoryView(DirectoryViewWidget *view) = 0;

    virtual DirectoryViewWidget *create() = 0;
};

}

Q_DECLARE_INTERFACE (Peony::DirectoryViewPluginIface2, DirectoryViewPluginIface2_iid)

#endif // DIRECTORYVIEWPLUGINIFACE2_H
