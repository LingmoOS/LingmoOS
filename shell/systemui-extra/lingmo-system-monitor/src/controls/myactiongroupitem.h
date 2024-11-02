/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MYACTIONGROUPITEM_H_
#define _MYACTIONGROUPITEM_H_

#include "myaction.h"

#include <QObject>

class MyActionGroup;

class MyActionGroupItem : public MyAction
{
public:
    MyActionGroupItem( QObject * parent, MyActionGroup *group,
                       const char * name, int data, bool autoadd = true );
};

#endif // _MYACTIONGROUPITEM_H_
