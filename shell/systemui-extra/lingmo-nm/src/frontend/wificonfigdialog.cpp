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
#include "wificonfigdialog.h"
#include "ui_wificonfigdialog.h"
#include "utils.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <KWindowEffects>

#include <QtConcurrent>
#include <QPainterPath>

WiFiConfigDialog::WiFiConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WiFiConfigDialog)
{
    ui->setupUi(this);

    initTransparentState(); //初始化窗口透明度的GSetting方法

    this->setWindowFlags(Qt::FramelessWindowHint); //Qt::WindowStaysOnTopHint
    this->setWindowTitle(tr("WLAN Authentication"));//"Wifi 认证"
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowIcon(QIcon::fromTheme("lingmo-network", QIcon(":/res/x/setup.png")) );

    QPainterPath path;
    auto rect = this->rect();
    rect.adjust(1, 1, -1, -1);
    path.addRoundedRect(rect, 6, 6);
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
    KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));

    ui->lbTitle->setText(tr("Input WLAN Information Please")); //输入Wi-Fi名称和密码后点击确定
    ui->lbWifiId->setText(tr("WLAN ID：")); //Wi-Fi连接名称：
    ui->lbWifiName->setText(tr("WLAN Name:")); //Wi-Fi名称：
    ui->lbWifiPassord->setText(tr("Password：")); //Wi-Fi密码：
    ui->btnCancel->setText(tr("Cancl"));
    ui->btnOk->setText(tr("Ok"));
    ui->leWifiId->setContextMenuPolicy(Qt::NoContextMenu);
    ui->leWiFiName->setContextMenuPolicy(Qt::NoContextMenu);

    ui->leWifiPassword->setContextMenuPolicy(Qt::NoContextMenu);
    ui->leWifiPassword ->setEchoMode(QLineEdit::Password);

    ui->checkBoxPwd->setStyleSheet(checkBoxQss);
    ui->checkBoxPwd->setFocusPolicy(Qt::NoFocus);

    ui->leWifiId->setContextMenuPolicy(Qt::NoContextMenu); //禁止LineEdit的右键菜单
    ui->leWiFiName->setContextMenuPolicy(Qt::NoContextMenu);
    ui->leWifiPassword->setContextMenuPolicy(Qt::NoContextMenu);

    this->setEnableOfBtn();
    this->setFixedSize(474, 320);
}

WiFiConfigDialog::~WiFiConfigDialog()
{
    delete ui;
}

void WiFiConfigDialog::paintEvent(QPaintEvent *event)
{
    double trans = this->getTransparentData();

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QRect rect = this->rect();
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setOpacity(trans);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect, 6, 6);
    QWidget::paintEvent(event);
}

//创建获取窗口透明度信息的GSetting的对象
void WiFiConfigDialog::initTransparentState()
{
    if (QGSettings::isSchemaInstalled("org.lingmo.control-center.personalise")) {
        m_transparency_gsettings = new QGSettings("org.lingmo.control-center.personalise");
    }
}

//使用GSetting获取当前窗口应该使用的透明度
double WiFiConfigDialog::getTransparentData()
{
    if (!m_transparency_gsettings) {
        return 0.7;
    }

    QStringList keys = m_transparency_gsettings->keys();
    if (keys.contains("transparency")) {
        double tp = m_transparency_gsettings->get("transparency").toDouble();
        return tp;
    } else {
        return 0.7;
    }
}

void WiFiConfigDialog::on_btnCancel_clicked()
{
    ui->leWiFiName->setText("");
    ui->leWifiPassword->setText("");
    hide();
}

void WiFiConfigDialog::on_btnOk_clicked()
{
    QFuture < void > future1 =  QtConcurrent::run([=](){
        this->toConfigWifi();
    });
    hide();
}

void WiFiConfigDialog::toConfigWifi()
{
    QString connName = ui->leWiFiName->text();
    QString password = ui->leWifiPassword->text();

    QString strConntype = "nmcli connection modify '" + connName + "' wifi-sec.psk-flags 0";
    Utils::m_system(strConntype.toUtf8().data());

    QString tmpPath = "/tmp/lingmo-nm-btoutput-" + QDir::home().dirName();
    QString cmdStr = "export LANG='en_US.UTF-8';export LANGUAGE='en_US';nmcli device wifi connect '" + connName + "' password '" + password + "' > " + tmpPath;
    Utils::m_system(cmdStr.toUtf8().data());

    QFile file(tmpPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"Can't open the file /tmp/lingmo-nm-btoutput in function toConfigWifi !";
    }
    QString line = file.readLine();
    file.close();
    qDebug()<<"connect_wifi_result: "<< line;

    if (line.indexOf("successfully") != -1) {
        QString strConntype = "nmcli connection modify '" + connName + "' wifi-sec.psk-flags 2";
        system(strConntype.toUtf8().data());
    } else {
        QString cmd = "export LANG='en_US.UTF-8';export LANGUAGE='en_US';nmcli connection delete '" + ui->leWifiId->text() + "'";
        int status = system(cmd.toUtf8().data());
        qDebug()<<"executed cmd="<<cmd<<". res="<<status;
    }
}

//切换密码明文
void WiFiConfigDialog::on_checkBoxPwd_clicked()
{
    if (ui->leWifiPassword->echoMode() == QLineEdit::Password) {
        ui->checkBoxPwd->setChecked(true);
        ui->leWifiPassword->setEchoMode(QLineEdit::Normal);
    } else {
        ui->checkBoxPwd->setChecked(false);
        ui->leWifiPassword->setEchoMode(QLineEdit::Password);
    }
}

//void WiFiConfigDialog::on_checkBoxPwd_released()
//{
//    ui->checkBoxPwd->setChecked(false);
//    ui->leWifiPassword ->setEchoMode(QLineEdit::Password);
//}

void WiFiConfigDialog::on_leWifiId_textEdited(const QString &arg1)
{
    this->setEnableOfBtn();
}

void WiFiConfigDialog::on_leWiFiName_textEdited(const QString &arg1)
{
    this->setEnableOfBtn();
}

void WiFiConfigDialog::on_leWifiPassword_textEdited(const QString &arg1)
{
    this->setEnableOfBtn();
}

//设置界面按钮是否可点击
void WiFiConfigDialog::setEnableOfBtn()
{
    if (ui->leWifiId->text().isEmpty() ) {
        ui->btnOk->setEnabled(false);
        return;
    }

    if (ui->leWiFiName->text().isEmpty() ) {
        ui->btnOk->setEnabled(false);
        return;
    }

    if (ui->leWifiPassword->text().isEmpty() ) {
        ui->btnOk->setEnabled(false);
        return;
    }

    ui->btnOk->setEnabled(true);
}
