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

#ifndef _MYACTIONGROUP_H_
#define _MYACTIONGROUP_H_

#include <QActionGroup>
#include <QWidget>
#include "myaction.h"

class MyActionGroup : public QActionGroup
{
    Q_OBJECT

public:
    MyActionGroup ( QObject * parent );
    void setChecked(int ID);

    int checked();
    void clear(bool remove);
    void setActionsEnabled(bool);
    void addTo(QWidget *);
    void removeFrom(QWidget *);
    void uncheckAll();

signals:
    void activated(int);

protected slots:
    void itemTriggered(QAction *);
};

#endif
