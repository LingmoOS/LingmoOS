// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base_commandlink_button.h"
#include <QKeyEvent>

BaseCommandLinkButton::BaseCommandLinkButton(const QString name, QWidget *parent)
    : Dtk::Widget::DCommandLinkButton(name, parent)
{
    // Add the event filter.
    this->installEventFilter(this);
}

BaseCommandLinkButton::~BaseCommandLinkButton()
{

}

bool BaseCommandLinkButton::eventFilter(QObject *obj, QEvent *event)
{
    // Catch the keyboard press enter signal.
    if (obj == this) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                emit clicked();
                return true;
            }
        }
    }
    return DCommandLinkButton::eventFilter(obj, event);
}
