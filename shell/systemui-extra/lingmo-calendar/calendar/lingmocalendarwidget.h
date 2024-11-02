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

#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H
#include <QFrame>
#include <QGSettings/QGSettings>
#include <QTimer>
#include <QEvent>
#include <QTranslator>
#include "lingmo/ilingmopanelplugin.h"
#include "../lunarcalendarwidget/frmlunarcalendarwidget.h"
#include "calendarbutton.h"
#include "lingmowebviewdialog.h"
#include "../lunarcalendarwidget/lunarcalendarwidget.h"
#include "../lunarcalendarwidget/morelabel.h"
extern LingmoUIWebviewDialogStatus status;
class LINGMOCalendarWidget:public QWidget
{
    Q_OBJECT
public:
    LINGMOCalendarWidget(ILINGMOPanelPlugin *plugin, QWidget* parent = 0);
    ~LINGMOCalendarWidget();
//    static LINGMOCalendarWidget &getInstance()
//    {
//        static LINGMOCalendarWidgest m_pInstance(ILINGMOPanelPlugin *plugin, QWidget *parent);
//        return m_pInstance;
//    }
    void realign();
    void changeWidowpos(bool restore = false);
    void change(int n);
//    QStringList m_markIdList;
//    QList<MarkInfo> m_markInfoList;
    ShowMoreLabel * label = new ShowMoreLabel(this);
//    LunarCalendarWidget *mm_widget = nullptr;
private:
    void translator();
protected:
private:
    ILINGMOPanelPlugin *m_plugin;
    QWidget *m_parent;
    QHBoxLayout *m_layout;
    frmLunarCalendarWidget *m_frmLunarWidget;
    CalendarButton *m_calendarButton;
    LunarCalendarWidget *m_widget;
};

#endif // CALENDARWIDGET_H
