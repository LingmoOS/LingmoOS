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

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include "explorer-core_global.h"

class QComboBox;
class QMenu;

namespace Peony {

class FMWindow;
class ViewFactorySortFilterModel2;
class SearchBar;

/*!
 * \brief The ToolBar class, providing a set of actions for file management.
 * \details
 * Toolbar is similar to QToolbar, the most great different is that a ToolBar
 * instance can bind with a FMWindow.
 *
 * If ToolBar bound with a FMWindow instance, the signal handler will be passed
 * to the FMWindow, for this case the ToolBar doesn't emit optionRequest signal.
 * Otherwise it would only send the signal.
 */
class ToolBar : public QToolBar
{
    Q_OBJECT
public:
    enum RequestType {
        Ivalid,
        OpenInNewWindow,
        OpenInNewTab,
        OpenInTerminal,
        SwitchView,
        Copy,
        Paste,
        Cut,
        Trash,
        Share,
        Burn,
        Archive,
        Other
    };
    Q_ENUM(RequestType)

    explicit ToolBar(FMWindow *window, QWidget *parent = nullptr);

Q_SIGNALS:
    void optionRequest(const RequestType &type);
    void updateLocationRequest(const QString &uri);

public Q_SLOTS:
    void updateLocation(const QString &uri);
    void updateStates();

protected:
    void init();

private:
    FMWindow *m_top_window;

    ViewFactorySortFilterModel2 *m_view_factory_model;
    //QComboBox *m_view_option_box;
    SearchBar *m_search_bar;

    QList<QAction *> m_file_op_actions;
    QAction *m_clean_trash_action;
    QAction *m_restore_action;

    QAction *m_view_action;
    QMenu *m_view_menu;
    QAction *m_sort_action;
    QAction *m_trash_actions_sperator;
};

}

#endif // TOOLBAR_H
