/*
* Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef CALENDARBUTTONTEXT_H
#define CALENDARBUTTONTEXT_H

#include <QObject>
#include <QDateTime>
#include <QGSettings>
#include <QPushButton>
#include "lingmo/ilingmopanelplugin.h"

#include "lingmosdk/lingmosdk-system/libkydate.h"

class CalendarButtonText : public QObject
{
    Q_OBJECT
public:
    CalendarButtonText(ILINGMOPanelPlugin *plugin,QPushButton *parent);
    ~CalendarButtonText();
    enum DateStyle{YEAR_MON_DAY=0, MON_DAY};

public:
    QString getBtnText();

private:
    void updateFontSize(QString btnText);
    void setOptimalFont(QStringList textList, int btnMaxLength);
    QString getFormatsLocale();

public:
    QPushButton *m_parent;
    ILINGMOPanelPlugin * m_plugin;
    QGSettings *m_fontGsettings = nullptr;
    QGSettings *m_panelGsettings = nullptr;
};

#endif // CALENDARBUTTONTEXT_H
