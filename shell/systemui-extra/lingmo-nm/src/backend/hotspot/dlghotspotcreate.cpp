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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "dlghotspotcreate.h"
#include "ui_dlghotspotcreate.h"
#include "utils.h"

DlgHotspotCreate::DlgHotspotCreate(QString wiFiCardName, QWidget *parent) :
    wirelessCardName(wiFiCardName),
    QDialog(parent),
    ui(new Ui::DlgHotspotCreate)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setStyleSheet("background-color:white;");

    ui->lbLeftup->setStyleSheet("QLabel{background-color:#266ab5;}");
    ui->lbLeftupIcon->setStyleSheet("QLabel{background-image:url(:/res/h/no-pwd-wifi.png);background-color:transparent;}");
    ui->lbLeftupTitle->setStyleSheet("QLabel{font-size:12px;color:#ffffff;background-color:transparent;}");
    ui->btnCancel->setStyleSheet("QPushButton{border:1px solid #aaaaaa;background-color:#f5f5f5;}"
                                 "QPushButton:Hover{border:2px solid #629ee8;background-color:#eeeeee;}"
                                 "QPushButton:Pressed{border:1px solid #aaaaaa;background-color:#d8d8d8;}");
    ui->btnOk->setStyleSheet("QPushButton{border:1px solid #aaaaaa;background-color:#f5f5f5;}"
                                 "QPushButton:Hover{border:2px solid #629ee8;background-color:#eeeeee;}"
                                 "QPushButton:Pressed{border:1px solid #aaaaaa;background-color:#d8d8d8;}");
    ui->checkBoxPwd->setStyleSheet("QCheckBox::indicator {width: 18px; height: 9px;}"
                                   "QCheckBox::indicator:checked {image: url(:/res/h/show-pwd.png);}"
                                   "QCheckBox::indicator:unchecked {image: url(:/res/h/hide-pwd.png);}");

    ui->lbLeftupTitle->setText(tr("Create Hotspot")); //创建个人热点
    ui->lbNetName->setText(tr("Network name")); //网络名称:
    ui->lbSecurity->setText(tr("WLAN security")); //WLAN 安全性:
    ui->lbPassword->setText(tr("Password")); //密码:
    ui->btnCancel->setText(tr("Cancel")); //取消
    ui->btnOk->setText(tr("Ok")); //确定
    ui->btnOk->setEnabled(false);

    ui->cbxSecurity->addItem(tr("None")); //无
    ui->cbxSecurity->addItem(tr("WPA and WPA2 Personal")); //WPA 及 WPA2 个人
    ui->cbxSecurity->setCurrentIndex(1);
    connect(ui->cbxSecurity,SIGNAL(currentIndexChanged(QString)),this,SLOT(changeDialog()));
}

DlgHotspotCreate::~DlgHotspotCreate()
{
    delete ui;
}

void DlgHotspotCreate::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        this->isPress = true;
        this->winPos = this->pos();
        this->dragPos = event->globalPos();
        event->accept();
    }
}
void DlgHotspotCreate::mouseReleaseEvent(QMouseEvent *event){
    this->isPress = false;
    this->setWindowOpacity(1);
}
void DlgHotspotCreate::mouseMoveEvent(QMouseEvent *event){
    if(this->isPress){
        this->move(this->winPos - (this->dragPos - event->globalPos()));
        this->setWindowOpacity(0.9);
        event->accept();
    }
}

void DlgHotspotCreate::on_btnCancel_clicked()
{
    this->close();
    Q_EMIT btnHotspotState();
}

void DlgHotspotCreate::on_btnOk_clicked()
{
    //nmcli device wifi hotspot [ifname ifname] [con-name name] [ssid SSID] [band {a | bg}] [channel channel] [password password]
    //example: nmcli device wifi hotspot ifname wlan0 con-name MyHostspot ssid MyHostspotSSID password 12345678
    QString str;
    if(ui->cbxSecurity->currentIndex() == 0 ){
        str = "nmcli device wifi hotspot ifname '" + wirelessCardName + "' con-name '" + ui->leNetName->text() + "' ssid '" + ui->leNetName->text() + "' SSID";
    }else{
        str = "nmcli device wifi hotspot ifname '" + wirelessCardName + "' con-name '" + ui->leNetName->text() + "' ssid '" + ui->leNetName->text() + "' password '" + ui->lePassword->text() + "'";
    }
    Utils::m_system(str.toUtf8().data());

    this->close();
    Q_EMIT updateHotspotList();
}

void DlgHotspotCreate::on_checkBoxPwd_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->lePassword ->setEchoMode(QLineEdit::Password);
    } else {
        ui->lePassword->setEchoMode(QLineEdit::Normal);
    }
}

void DlgHotspotCreate::on_leNetName_textEdited(const QString &arg1)
{
    if(ui->cbxSecurity->currentIndex() == 0 ){
        if (ui->leNetName->text() == ""){
            ui->btnOk->setEnabled(false);
        } else {
            ui->btnOk->setEnabled(true);
        }
    }else{
        if (ui->leNetName->text() == "" || ui->lePassword->text().size() < 5){
            ui->btnOk->setEnabled(false);
        } else {
            ui->btnOk->setEnabled(true);
        }
    }
}

void DlgHotspotCreate::on_lePassword_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == "" || ui->lePassword->text().size() < 5){
        ui->btnOk->setEnabled(false);
    } else {
        ui->btnOk->setEnabled(true);
    }
}

void DlgHotspotCreate::changeDialog()
{
    if(ui->cbxSecurity->currentIndex()==0){
        ui->lbPassword->setEnabled(false);
        ui->lePassword->setEnabled(false);
        ui->checkBoxPwd->setEnabled(false);
    } else {
        ui->lbPassword->setEnabled(true);
        ui->lePassword->setEnabled(true);
        ui->checkBoxPwd->setEnabled(true);
    }
}
