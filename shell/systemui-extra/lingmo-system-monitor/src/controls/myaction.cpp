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

#include "myaction.h"
#include <QWidget>

MyAction::MyAction (QObject * parent, const char * name, bool autoadd)
    : QAction(parent)
{
    setObjectName(name);
    if (autoadd) addActionToParent();
}


MyAction::MyAction(QObject * parent, bool autoadd)
    : QAction(parent)
{
    if (autoadd) addActionToParent();
}

MyAction::MyAction(const QString & text, QKeySequence accel,
                   QObject * parent, const char * name, bool autoadd )
    : QAction(parent)
{
    setObjectName(name);
    setText(text);
    setShortcut(accel);
    if (autoadd) addActionToParent();
}

MyAction::MyAction(QKeySequence accel, QObject * parent, const char * name,
                   bool autoadd )
    : QAction(parent)
{
    setObjectName(name);
    setShortcut(accel);
    if (autoadd) addActionToParent();
}

MyAction::~MyAction() {
}

void MyAction::addShortcut(QKeySequence key) {
    setShortcuts( shortcuts() << key);
}

void MyAction::addActionToParent() {
    if (parent()) {
        if (parent()->inherits("QWidget")) {
            QWidget *w = static_cast<QWidget*> (parent());
            w->addAction(this);
        }
    }
}

void MyAction::change(const QIcon & icon, const QString & text) {
    setIcon( icon );
    change(text);
}

void MyAction::change(const QString & text ) {
    setText( text );

    QString accel_text = shortcut().toString();

    QString s = text;
    s.replace("&","");
    if (!accel_text.isEmpty()) {
        setToolTip( s + " ("+ accel_text +")");
        setIconText( s );
    }

    /*
    if (text.isEmpty()) {
        QString s = menuText;
        s = s.replace("&","");
        setText( s );

        if (!accel_text.isEmpty())
            setToolTip( s + " ("+ accel_text +")");
    } else {
        setText( text );
        if (!accel_text.isEmpty())
            setToolTip( text + " ("+ accel_text +")");
    }
    */
}

