/*
* Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef CALENDARBUTTON_H
#define CALENDARBUTTON_H
#include <QToolButton>
#include <QTimer>
#include <QProcess>
#include <QMenu>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QtDBus/QtDBus>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QGSettings>
#include <QPushButton>
#include "lingmo/ilingmopanelplugin.h"
#include <lingmo/listengsettings.h>
#include <lingmo/common.h>
#include "calendarbuttontext.h"


class CalendarButton : public QPushButton
{
    Q_OBJECT
public:
    CalendarButton(ILINGMOPanelPlugin *plugin,QWidget* parent = 0);
    ~CalendarButton();
protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *event);
    bool event(QEvent *event);
private:
    void initTimeGsettings();
    void initFontGsettings();
Q_SIGNALS:
    void pressTimeText();
    void pressShowHideCalendar();
private Q_SLOTS:
    void setControlTime();
    void updateBtnText(QString timerStr);
    void setSystemStyle();

private:
    ILINGMOPanelPlugin * m_plugin;
    QWidget *m_parent;
    QGSettings *m_styleGsettings;
    ListenGsettings *m_listenGsettings;
    int m_panelSize;
    QGSettings *m_fontGsettings;
    QString m_systemFontSize;
    QString m_systemFont;
    QMenu *m_menuCalender;
    const qreal m_btnAlphaF = 0.13; //按钮背景透明度暂时使用LINGMO3.1版本的透明度
};

#endif // CALENDARBUTTON_H
