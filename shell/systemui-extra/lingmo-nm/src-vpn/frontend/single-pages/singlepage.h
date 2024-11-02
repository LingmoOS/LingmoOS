/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SINGLEPAGE_H
#define SINGLEPAGE_H

#include "divider.h"
#include "kylable.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QListWidget>
#include <QDir>
#include <QComboBox>
#include <QEvent>
#include <QProcess>
#include <QDebug>
#include "lingmonetworkdeviceresource.h"
#include "kwidget.h"
#include "kswitchbutton.h"

using namespace kdk;

#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_SPACING 0
#define TITLE_FRAME_HEIGHT 50
#define TITLE_LAYOUT_MARGINS 24,0,24,0
#define NET_LAYOUT_MARGINS 8,4,0,4
#define TEXT_HEIGHT 20
#define SETTINGS_LAYOUT_MARGINS 23,0,24,0

#define MAX_ITEMS 4
#define MAX_WIDTH 412
#define MIN_WIDTH 404

#define SCROLL_STEP 4

#define TRANSPARENCY "transparency"
#define TRANSPARENCY_GSETTINGS "org.lingmo.control-center.personalise"

class SinglePage : public QWidget
{
    Q_OBJECT
public:
    explicit SinglePage(QWidget *parent = nullptr);
    ~SinglePage();

    static void showDesktopNotify(const QString &message, QString soundName);

Q_SIGNALS:
    void activateFailed(QString errorMessage);
    void deactivateFailed(QString errorMessage);

private Q_SLOTS:
    void onTransChanged();
    void onThemeChanged(const QString &key);
    void setThemePalette();

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);

protected:
    void initUI();
    void initTransparency();
    void paintWithTrans();


private:
    void initWindowProperties();
    void initWindowTheme();

protected:
    QVBoxLayout * m_mainLayout = nullptr;

    QLabel * m_titleLabel = nullptr;
    QFrame * m_titleFrame = nullptr;
    QHBoxLayout * m_titleLayout = nullptr;

    QFrame * m_listFrame = nullptr;
    QListWidget * m_listWidget = nullptr;
    QVBoxLayout * m_listLayout = nullptr;

    QFrame * m_settingsFrame = nullptr;
    QHBoxLayout * m_settingsLayout = nullptr;
    KyLable * m_settingsLabel = nullptr;

    Divider * m_netDivider = nullptr;
    Divider * m_setDivider = nullptr;


    QGSettings * m_transGsettings = nullptr;
    double m_transparency = 1.0;  //透明度
    //监听主题的Gsettings
    QGSettings * m_styleGsettings = nullptr;

};

#endif // TABPAGE_H
