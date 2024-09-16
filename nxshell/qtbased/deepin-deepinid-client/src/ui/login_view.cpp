// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login_view.h"

#include <QContextMenuEvent>

LoginView::LoginView(QWidget *parent) : QWebEngineView(parent)
{

}

void LoginView::contextMenuEvent(QContextMenuEvent *event)
{
    event->ignore();
}
