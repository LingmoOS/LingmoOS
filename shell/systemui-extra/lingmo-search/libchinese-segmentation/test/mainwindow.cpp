/*
 *
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
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hanzi-to-pinyin.h"
#include "chinese-segmentation.h"
#include "Traditional-to-Simplified.h"
#include <QMenu>
#include <QDebug>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QMenu * menu = new QMenu(this);
    menu->addAction("Default");
    menu->addAction("Tone");
    menu->addAction("Tone2");
    menu->addAction("Tone3");
    menu->addAction("FirstLetter");
    ui->toolButton->setMenu(menu);
    initconnections();
    ui->lineEdit_2->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initconnections()
{
    connect(ui->toolButton->menu(), &QMenu::triggered, [&](QAction *action){
        qDebug() << "tool button:" << action->text();
        m_action = action->text();
        ui->toolButton->setText(action->text());
    });
    connect(ui->pushButton, &QPushButton::pressed, [&]() {
        PinyinDataStyle dataStyle;
        SegType segType;
        PolyphoneType polyType;
        ExDataProcessType exType;

        if (m_action == "Default") {
            dataStyle = PinyinDataStyle::Default;
        } else if (m_action == "Tone") {
            dataStyle = PinyinDataStyle::Tone;
        } else if (m_action == "Tone2") {
            dataStyle = PinyinDataStyle::Tone2;
        } else if (m_action == "Tone3") {
            dataStyle = PinyinDataStyle::Tone3;
        } else if (m_action == "FirstLetter") {
            dataStyle = PinyinDataStyle::FirstLetter;
        }

        if(!ui->checkSegBox->isChecked())
            segType = SegType::Segmentation;
        else
            segType = SegType::NoSegmentation;

        if(ui->checkPolyBox_2->isChecked())
            polyType = PolyphoneType::Enable;
        else
            polyType = PolyphoneType::Disable;

        if (ui->checkExBox_3->isChecked())
            exType = ExDataProcessType::Default;
        else
            exType = ExDataProcessType::Delete;

        HanZiToPinYin::getInstance()->setConfig(dataStyle, segType, polyType, exType);

        ui->lineEdit_4->clear();
        QString text = ui->lineEdit_2->text();
        qDebug() << "input:" << text;

        QStringList list;
        HanZiToPinYin::getInstance()->getResults(text.toStdString(), list);

        ui->lineEdit_4->setText(list.join(" "));
        qDebug() << "result:" << list.join(" ");

        vector<KeyWord> result = ChineseSegmentation::getInstance()->callSegment(text.toStdString());

        list.clear();
        for (auto &info:result) {
            list.append(QString().fromStdString(info.word));
        }
        ui->lineEdit_6->setText(list.join("/"));

        string simplified = Traditional2Simplified::getInstance()->getResults(text.toStdString());

        ui->lineEdit_7->setText(QString().fromStdString(simplified));
    });
}

