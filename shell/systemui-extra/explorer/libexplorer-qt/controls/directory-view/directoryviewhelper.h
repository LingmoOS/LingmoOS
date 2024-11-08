/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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

#ifndef DIRECTORYVIEWHELPER_H
#define DIRECTORYVIEWHELPER_H

#include <QMap>
#include <QObject>
#include "explorer-core_global.h"
#include "directory-view-widget.h"

namespace Peony {

class PEONYCORESHARED_EXPORT DirectoryViewIface2
{
public:
    explicit DirectoryViewIface2() {}
    virtual ~DirectoryViewIface2() {}

    virtual void doMultiSelect(bool) = 0;
    virtual bool isEnableMultiSelect() = 0;

    virtual void setItemsVisible(bool visible) = 0;
    virtual const int getAllDisplayFileCount() = 0;
};

class PEONYCORESHARED_EXPORT DirectoryViewHelper : public QObject
{
    Q_OBJECT
public:
    static DirectoryViewHelper *globalInstance();
    void addIconViewWithDirectoryViewWidget(DirectoryViewIface2 *iconView, DirectoryViewWidget *widget);
    void addListViewWithDirectoryViewWidget(DirectoryViewIface2 *listView, DirectoryViewWidget *widget);
    DirectoryViewIface2 *getViewIface2ByDirectoryViewWidget(DirectoryViewWidget *widget);

Q_SIGNALS:
   void updateSelectStatus(bool status);

private:
    explicit DirectoryViewHelper(QObject *parent = nullptr);

    QMap<DirectoryViewWidget *, DirectoryViewIface2 *> m_views;
};

}

#endif // DIRECTORYVIEWHELPER_H
