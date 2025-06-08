/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "datetimedialog.h"
#include <KWindowSystem>

DateTimeDialog::DateTimeDialog(QQuickWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(Qt::Popup);
    setColor(Qt::transparent);
    installEventFilter(this);
}

void DateTimeDialog::open()
{
    setVisible(true);
    setMouseGrabEnabled(true);
    setKeyboardGrabEnabled(true);
}

bool DateTimeDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (QWindow *w = qobject_cast<QWindow*>(object)) {
            if (!w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos())) {
                DateTimeDialog::setVisible(false);
            }
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            DateTimeDialog::setVisible(false);
        }
    } else if (event->type() == QEvent::Show) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    } else if (event->type() == QEvent::Hide) {
        setMouseGrabEnabled(false);
        setKeyboardGrabEnabled(false);
    }

    return QObject::eventFilter(object, event);
}
