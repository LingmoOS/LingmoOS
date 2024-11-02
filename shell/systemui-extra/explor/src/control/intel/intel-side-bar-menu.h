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

#ifndef INTEL_SIDEBARMENU_H
#define INTEL_SIDEBARMENU_H

#include <QMenu>
#include "explor-core_global.h"

namespace Peony {

class SideBar;

namespace Intel {

class SideBarAbstractItem;

class PEONYCORESHARED_EXPORT SideBarMenu : public QMenu
{
    Q_OBJECT
public:
    explicit SideBarMenu(SideBarAbstractItem *item, SideBar *sideBar, QWidget *parent = nullptr);

    void gotoAboutComputer();

protected:
    const QList<QAction *> constructFavoriteActions();
    const QList<QAction *> constructPersonalActions();
    const QList<QAction *> constructFileSystemItemActions();
    const QList<QAction *> constructNetWorkItemActions();
private:
    SideBarAbstractItem *m_item;
    SideBar *m_side_bar;
    QString m_uri;
};

}

}

#endif // SIDEBARMENU_H
