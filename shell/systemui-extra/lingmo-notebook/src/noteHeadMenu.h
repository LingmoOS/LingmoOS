/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef NOTEHEADMENU_H
#define NOTEHEADMENU_H

#include <QWidget>
#include <QPainter>
#include <QAction>
#include <QMenu>

#include "lingmo_settings_monitor.h"

namespace Ui {
class noteHeadMenu;
}

class noteHeadMenu : public QWidget
{
    Q_OBJECT

public:
    explicit noteHeadMenu(QWidget *parent = nullptr);
    ~noteHeadMenu() override;

    Ui::noteHeadMenu *ui;

public:
    QColor colorWidget;
    QAction *m_menuActionDel = nullptr;
    QAction *m_topAction = nullptr;

signals:
    void requestTopMost();

protected:
    void paintEvent(QPaintEvent *event) override;
//    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void btnInit();
    void slotsInit();
    void initMenu();

    QMenu *m_menu;

signals:
    void requestNewNote();
    void requestShowNote();
    void requestUpdateMenuIcon();
//    void requestFullscreen();
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

private:
    void handleTabletMode(LingmoUISettingsMonitor::TabletStatus status);
};

#endif // NOTEHEADMENU_H
