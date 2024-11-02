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

#include "directoryviewhelper.h"



using namespace Peony;

static DirectoryViewHelper *global_instance = nullptr;

DirectoryViewHelper *DirectoryViewHelper::globalInstance()
{
    if (!global_instance) {
        global_instance = new DirectoryViewHelper;
    }
    return global_instance;
}

void DirectoryViewHelper::addIconViewWithDirectoryViewWidget(DirectoryViewIface2 *iconView, DirectoryViewWidget *widget)
{
    m_views.insert(widget, iconView);
}

void DirectoryViewHelper::addListViewWithDirectoryViewWidget(DirectoryViewIface2 *listView, DirectoryViewWidget *widget)
{
    m_views.insert(widget, listView);
}

DirectoryViewIface2 *DirectoryViewHelper::getViewIface2ByDirectoryViewWidget(DirectoryViewWidget *widget)
{
    return m_views.value(widget, nullptr);
}

DirectoryViewHelper::DirectoryViewHelper(QObject *parent) : QObject(parent)
{

}
