/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef CALENDAR_H
#define CALENDAR_H
#include "lingmo/ilingmopanelplugin.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QFontMetrics>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QMenu>
#include <QPointF>
#include <QMainWindow>
#include "lingmocalendarwidget.h"

class StartBarLibrary;
class LINGMOCalendarPlugin;

class LINGMOCalendarPlugin: public QObject, public ILINGMOPanelPlugin
{
    Q_OBJECT
public:
    explicit LINGMOCalendarPlugin(const ILINGMOPanelPluginStartupInfo &startupInfo);
    ~LINGMOCalendarPlugin();

    virtual QWidget *widget();
    virtual QString themeId() const { return "Calendar"; }
    virtual Flags flags() const { return NeedsHandle; }
    void realign();
    bool isSeparate() const { return true; }
private:
    LINGMOCalendarWidget *m_widget;
};

class CalendarLibrary: public QObject, public ILINGMOPanelPluginLibrary
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "lingmo.org/Panel/PluginInterface/3.0")
    Q_INTERFACES(ILINGMOPanelPluginLibrary)
public:
    ILINGMOPanelPlugin *m_plugin;
    ILINGMOPanelPlugin *instance(const ILINGMOPanelPluginStartupInfo &startupInfo) const
    {
        return new LINGMOCalendarPlugin(startupInfo);
    }
};





#endif // CALENDAR_H
