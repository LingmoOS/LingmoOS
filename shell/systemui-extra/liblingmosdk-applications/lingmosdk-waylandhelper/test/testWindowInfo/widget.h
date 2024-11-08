/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include "windowmanager/windowinfo.h"
#include "windowmanager/windowmanager.h"

using namespace kdk;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void initUi();
    void initConnections();

private:
    QPushButton* m_btnMinimize;
    QPushButton* m_btnMaximum;
    QPushButton* m_btnActivate;
    QPushButton* m_btnStayOnTop;
    QPushButton* m_btnClose;

    kdk::WindowManager *m_manager;
    WindowId m_lastestWindowId;
};
#endif // WIDGET_H
