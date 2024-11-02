/*
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */
#include "action-label.h"
#define ACTION_NORMAL_COLOR QColor(55, 144, 250, 255)
#define ACTION_HOVER_COLOR QColor(64, 169, 251, 255)
#define ACTION_PRESS_COLOR QColor(41, 108, 217, 255)
using namespace LingmoUISearch;
ActionLabel::ActionLabel(const QString &action, const QString &key, QWidget *parent) : KBorderlessButton(parent)
{
    m_action = action;
    m_key = key;
    this->initUi();
    this->installEventFilter(this);
}

void ActionLabel::initUi()
{
    this->setText(m_action);
//    QPalette pal = palette();
//    pal.setColor(QPalette::WindowText, ACTION_NORMAL_COLOR);
//    pal.setColor(QPalette::Light, ACTION_HOVER_COLOR);
//    pal.setColor(QPalette::Dark, ACTION_PRESS_COLOR);
//    this->setPalette(pal);
//    this->setForegroundRole(QPalette::WindowText);
    this->setCursor(QCursor(Qt::PointingHandCursor));
}

bool ActionLabel::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        if (event->type() == QEvent::MouseButtonRelease) {
            Q_EMIT this->actionTriggered(m_action);
            return false;
        }
    }


//    if (watched == this) {
//        if(event->type() == QEvent::MouseButtonPress) {
//            this->setForegroundRole(QPalette::Dark);
//            return true;
//        } else if(event->type() == QEvent::MouseButtonRelease) {
//            Q_EMIT this->actionTriggered(m_action);
//            this->setForegroundRole(QPalette::Light);
//            return true;
//        } else if(event->type() == QEvent::Enter) {
//            this->setForegroundRole(QPalette::Light);
//            return true;
//        } else if(event->type() == QEvent::Leave) {
//            this->setForegroundRole(QPalette::WindowText);
//            return true;
//        }
//    }
    return KBorderlessButton::eventFilter(watched, event);
}
