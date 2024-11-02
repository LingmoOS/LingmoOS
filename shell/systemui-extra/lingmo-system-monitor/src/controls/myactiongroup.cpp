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

#include "myactiongroup.h"
#include <QAction>
#include <QList>
#include <QWidget>

MyActionGroup::MyActionGroup( QObject * parent ) : QActionGroup(parent)
{
    setExclusive(true);
    connect(this, SIGNAL(triggered(QAction *)), this, SLOT(itemTriggered(QAction *)) );
}

void MyActionGroup::setChecked(int ID) {
    QList <QAction *> l = actions();
    for (int n=0; n < l.count(); n++) {
        if ( (!l[n]->isSeparator()) && (l[n]->data().toInt() == ID) ) {
            l[n]->setChecked(true);
            return;
        }
    }
}

int MyActionGroup::checked() {
    QAction * a = checkedAction();
    if (a)
        return a->data().toInt();
    else
        return -1;
}

void MyActionGroup::uncheckAll() {
    QList <QAction *> l = actions();
    for (int n=0; n < l.count(); n++) {
        l[n]->setChecked(false);
    }
}

void MyActionGroup::setActionsEnabled(bool b) {
    QList <QAction *> l = actions();
    for (int n=0; n < l.count(); n++) {
        l[n]->setEnabled(b);
    }
}

void MyActionGroup::clear(bool remove) {
    while (actions().count() > 0) {
        QAction * a = actions()[0];
        if (a) {
            removeAction(a);
            if (remove) a->deleteLater();
        }
    }
}

void MyActionGroup::itemTriggered(QAction *a) {
    int value = a->data().toInt();

    emit activated(value);
}

void MyActionGroup::addTo(QWidget *w) {
    w->addActions( actions() );
}

void MyActionGroup::removeFrom(QWidget *w) {
    for (int n=0; n < actions().count(); n++) {
        w->removeAction( actions()[n] );
    }
}
