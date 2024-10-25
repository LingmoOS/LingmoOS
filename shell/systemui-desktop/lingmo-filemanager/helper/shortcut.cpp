/***************************************************************************
 *   Copyright 2021 Reion Wong <reion@lingmoos.com>                      *
 *   Copyright Ken <https://stackoverflow.com/users/1568857/ken>           *
 *   Copyright 2016 Leslie Zhai <xiangzhai83@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "shortcut.h"
#include "keyboardsearchmanager.h"

#include <QKeyEvent>

ShortCut::ShortCut(QObject *parent)
    : QObject(parent)
    , m_object(parent)
{
}

void ShortCut::install(QObject *target)
{
    // No need to remove, because memory space is automatically freed in qml.
    // if (m_object) {
    //     m_object->removeEventFilter(this);
    // }

    if (target) {
        target->installEventFilter(this);
        // m_object = target;
    }
}

bool ShortCut::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        // int keyInt = keyEvent->modifiers() + keyEvent->key();

        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            emit open();
        } else if (keyEvent->key() == Qt::Key_C && keyEvent->modifiers() & Qt::ControlModifier) {
            emit copy();
        } else if (keyEvent->key() == Qt::Key_X && keyEvent->modifiers() & Qt::ControlModifier) {
            emit cut();
        } else if (keyEvent->key() == Qt::Key_W && keyEvent->modifiers() & Qt::ControlModifier) {
            emit close();
        } else if (keyEvent->key() == Qt::Key_V && keyEvent->modifiers() & Qt::ControlModifier) {
            emit paste();
        } else if (keyEvent->key() == Qt::Key_F2) {
            emit rename();
        } else if (keyEvent->key() == Qt::Key_L && keyEvent->modifiers() & Qt::ControlModifier) {
            emit openPathEditor();
        } else if (keyEvent->key() == Qt::Key_A && keyEvent->modifiers() & Qt::ControlModifier) {
            emit selectAll();
        } else if (keyEvent->key() == Qt::Key_Backspace) {
            emit backspace();
        } else if (keyEvent->key() == Qt::Key_Delete) {
            emit deleteFile();
        } else if (keyEvent->key() == Qt::Key_F5) {
            emit refresh();
        } else if (keyEvent->key() == Qt::Key_H && keyEvent->modifiers() & Qt::ControlModifier) {
            emit showHidden();
        } else if (keyEvent->key() == Qt::Key_Z && keyEvent->modifiers() & Qt::ControlModifier) {
            emit undo();
        } else if (keyEvent->key() >= Qt::Key_A && keyEvent->key() <= Qt::Key_Z) {
            // Handle select
            // KeyboardSearchManager::self()->addKeys(keyEvent->text());
            emit keyPressed(keyEvent->text());
            keyEvent->ignore();
        }
    }

    return QObject::eventFilter(obj, e);
}
