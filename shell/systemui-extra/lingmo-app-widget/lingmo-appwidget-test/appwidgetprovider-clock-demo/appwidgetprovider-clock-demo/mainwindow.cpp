/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QProcess>
#include <QtQuick/QQuickView>
#include <QDir>
#include <QMessageBox>
#include <QDBusReply>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//初始化界面
    for (int h = 0; h < 24; h++) {
        ui->comboBox->addItem(QString::number(h));
    }
    for (int m = 0; m < 60; m++) {
        ui->comboBox_2->addItem(QString::number(m));
    }

    for (int s = 0; s < 60; s++) {
        ui->comboBox_3->addItem(QString::number(s));
    }
    ui->dateEdit->setDate(QDate::currentDate());
    ui->comboBox->setCurrentText(QString::number(QTime::currentTime().hour()));
    ui->comboBox_2->setCurrentText(QString::number(QTime::currentTime().minute()));
    ui->comboBox_3->setCurrentText(QString::number(QTime::currentTime().second()));

//加载小插件demo的qml ui，但是加上后启动太慢，先去掉
//    QQuickView *p=new QQuickView();
//    QString path=QString("qrc:/clock.qml");
//    qDebug()<<"path"<<path;
//    QUrl url(path);
//    p->setSource(url);
//    QWidget*w= QWidget::createWindowContainer(p,this);
//    w->setFixedSize(370,340);
//    ui->verticalLayout_3->addWidget(w);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=](){
//        qDebug()<<"timer update";
        ui->dateEdit->setDate(QDate::currentDate());
        ui->comboBox->setCurrentText(QString::number(QTime::currentTime().hour()));
        ui->comboBox_2->setCurrentText(QString::number(QTime::currentTime().minute()));
        ui->comboBox_3->setCurrentText(QString::number(QTime::currentTime().second()));
    });
    timer->start(1000);


//闹钟记录的数据存放位置
    QString path1 = QDir::homePath() + "/.config/appwidgetclock/config.ini";
    setting = new QSettings(path1, QSettings::IniFormat);

//clcok小插件的后台服务dbus接口
    appwidgetinterface = new QDBusInterface("org.lingmo.appwidget.provider.democlock",
                                            "/org/lingmo/appwidget/provider",
                                            "org.lingmo.appwidget.provider.democlock",
                                            QDBusConnection::sessionBus());


//初始化闹钟列表
    ui->listView->setViewMode(QListView::ListMode);
    itemmode = new QStandardItemModel(this);
    ui->listView->setModel(itemmode);
    setting->beginGroup("AlarmClock");
    QStringList AlarmClocklist = setting->allKeys();
    for (int i = 0; i<AlarmClocklist.count(); i++) {
        QStandardItem *item = new QStandardItem(AlarmClocklist.at(i));
        itemmode->appendRow(item);
    }
    setting->endGroup();
    setting->sync();


//监控闹钟列表的点击事件，进行删除闹钟
    connect(ui->listView, &QListView::pressed, this, [=]() {
        QString key = itemmode->item(ui->listView->currentIndex().row())->text();
        setting->beginGroup("AlarmClock");
        setting->remove(key);
        setting->endGroup();
        setting->sync();
        appwidgetinterface->asyncCall("appWidgetUpdate");
        ui->listView->model()->removeRow(ui->listView->currentIndex().row());
    });

}

MainWindow::~MainWindow()
{
    delete ui;
    delete appwidgetinterface;
    delete setting;
    delete itemmode;

}

void MainWindow::on_pushButton_clicked()
{
//设置修改日期和时间，qml的图形钟表会自动根据系统时间进行更改，在qml中内部有逻辑处理，这里更改后通知小插件更改的主要是日期
    QDate date = ui->dateEdit->date();// 日期
    int year = date.year();          // 年
    int month = date.month();        // 月
    int day = date.day();            // 日
    int hour = ui->comboBox->currentText().toInt();
    int min = ui->comboBox_2->currentText().toInt();
    int sss = ui->comboBox_3->currentText().toInt();
    qDebug() << "日期and时间" << year << month << day << hour << min << sss;

    QString m_qstrDateTime = QString("%1%2%3%4%5.%6").
            arg(month, 2, 10, QChar('0')).                 /*月份，必须两位*/
            arg(QString("%1").arg(day, 2, 10, QChar('0')))./*日，必须两位*/
            arg(hour, 2, 10, QChar('0')).                  /*小时，必须两位*/
            arg(min, 2, 10, QChar('0')).                   /*分钟，必须两位*/
            arg(year).                                     /*年份，必须4位*/
            arg(sss, 2, 10, QChar('0'));                   /*秒，必须两位*/

//设置时间需要提权，对应电脑的密码需要改一下。
    qDebug() << m_qstrDateTime;
    QString qstrDateTime = QString("/bin/sh -c \"echo \"%1\" | sudo -S date %2\"").arg("123123.."/*电脑密码*/).arg(m_qstrDateTime);
    qDebug() << qstrDateTime;

    QProcess::execute(qstrDateTime);
    QString qstrSaveTime = QString("/bin/sh -c \"echo \"%1\" | sudo -S hwclock --systohc\"").arg("123123.."/*电脑密码*/);
    QProcess::execute(qstrSaveTime);


//通过后台服务通知管理服务更新
    appwidgetinterface->asyncCall("appWidgetUpdate");
}

void MainWindow::on_pushButton_2_clicked()
{
    setting->beginGroup("AlarmClock");
    QString value = ui->timeEdit->text();
    if(setting->contains(value)) {
        qDebug()<<" 已经存在该时间的闹钟";
        QMessageBox::critical(this, "提示","已经存在该时间的闹钟，请换一个时间点");
        setting->endGroup();
        setting->sync();
        return ;
    }
    setting->setValue(value, true);
    setting->endGroup();
    setting->sync();


    QStandardItem *item = new QStandardItem(value);
    itemmode->appendRow(item);
    appwidgetinterface->asyncCall("appWidgetUpdate");


}
