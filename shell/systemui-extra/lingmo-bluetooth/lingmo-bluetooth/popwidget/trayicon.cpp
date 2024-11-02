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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "trayicon.h"

TrayIcon::TrayIcon(bool _trayshow, bool _existAdapter, QWidget *parent)
    : QWidget(parent)
{
    if(QGSettings::isSchemaInstalled(GSETTING_SCHEMA_LINGMOBLUETOOH)){
        settings = new QGSettings(GSETTING_SCHEMA_LINGMOBLUETOOH);
        existSettings = true;
    }

    if(QGSettings::isSchemaInstalled(GSETTING_SCHEMA_UKCC)){
        ukccGsetting = new QGSettings(GSETTING_SCHEMA_UKCC,GSETTING_PACH_UKCC);
    }
    existAdapter = _existAdapter;

    bluetooth_tray_icon = new QSystemTrayIcon(this);
    tray_Menu = new QMenu(this);
    QAction *settins_action = new QAction(tr("Set Bluetooth Item"),tray_Menu);
    settins_action->setIcon(QIcon::fromTheme("document-page-setup-symbolic", QIcon(":/res/x/setup.png")) );
    tray_Menu->addAction(settins_action);
    canShow = true;
    connect(tray_Menu, &QMenu::aboutToShow, this, [=]{
        emit hideTrayWidget();
        canShow = false;
    });
    connect(tray_Menu, &QMenu::aboutToHide, this, [=]{
        emit hideTrayWidget();
        canShow = true;
    });
    connect(tray_Menu, &QMenu::triggered, this, [=]{
        emit openBluetoothSettings();
    });

    //Create taskbar tray icon and connect to signal slot
    //创建任务栏托盘图标，并连接信号槽
    bluetooth_tray_icon = new QSystemTrayIcon(this);
    bluetooth_tray_icon->setContextMenu(tray_Menu);
    bluetooth_tray_icon->setToolTip(tr("Bluetooth"));
    bluetooth_tray_icon->setProperty("useIconHighlightEffect", 0x2);

    if (existAdapter && existSettings && _trayshow != settings->get("tray-show").toBool()) {
        if(QGSettings::isSchemaInstalled(GSETTING_SCHEMA_LINGMOBLUETOOH)) {
            _trayshow = settings->get("tray-show").toBool();
            QMap<QString, QVariant> attr;
            attr.remove("trayShow");
            attr.insert("trayShow", QVariant(settings->get("tray-show").toBool()));
            Config::setDefaultAdapterAttr(attr);
        }
    }
    bluetooth_tray_icon->setVisible(_trayshow && existAdapter);

    connect(bluetooth_tray_icon,&QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason){
                switch (reason)
                {
                    case QSystemTrayIcon::MiddleClick:
                    case QSystemTrayIcon::DoubleClick: /* 来自于双击激活。 */
                    case QSystemTrayIcon::Trigger: /* 来自于单击激活。 */
                        if (clickEnable && canShow)
                            emit showTrayWidget();
                        break;
                    case QSystemTrayIcon::Unknown:
                    case QSystemTrayIcon::Context:
                    break;
                }
            });
    updateTrayiconMenuStyle();
#ifdef KYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())){
        secState = true;
        bluetooth_tray_icon->setVisible(false);
    }
#endif
}

TrayIcon::~TrayIcon()
{
    delete settings;
}

void TrayIcon::SetTrayIcon(bool open)
{
    KyDebug() << open;
    if(!open){
        if(QIcon::hasThemeIcon("bluetooth-error"))
            bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-error"));
        else
            bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-active-symbolic"));
    }else{
        bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-active-symbolic"));
    }
}

void TrayIcon::SetAdapterFlag(bool set) {
    KyDebug() << set;
    if (secState)
        return;
    if (!existAdapter)
        return;
    bluetooth_tray_icon->setVisible(set);

    if(QGSettings::isSchemaInstalled(GSETTING_SCHEMA_LINGMOBLUETOOH)){
        settings->set("tray-show", QVariant(set));
    }

    if (set && QGSettings::isSchemaInstalled(GSETTING_SCHEMA_UKCC))
        ukccGsetting->set("show",QVariant(true));
    KyDebug() ;
}

void TrayIcon::setClickEnable(bool enable) {
    clickEnable = !enable;
}

void TrayIcon::setAdapterExist(bool exist, bool show) {
    if (exist) {
        if (QGSettings::isSchemaInstalled(GSETTING_SCHEMA_UKCC)) {
            ukccGsetting->set("show",QVariant(true));
        }
    }
    existAdapter = exist;
    bluetooth_tray_icon->setVisible(exist && show);
    if (!bluetooth_tray_icon->isVisible())
        emit hideTrayWidget();
}

void TrayIcon::updateTrayiconMenuStyle() {
    QPalette pal = this->palette();
    //托盘watcher异常时的规避方案 bug#214664
    if (tray_Menu != nullptr) {
        pal.setColor(QPalette::Base, pal.color(QPalette::Base)); //处理Wayland环境setPalette(pal)不生效问题
        pal.setColor(QPalette::Text, pal.color(QPalette::Text));
        tray_Menu->setPalette(pal);
    }
}
