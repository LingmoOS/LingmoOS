/*
 * Qt5-LINGMO
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDir>
#include <QUrl>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QFileDialog d;
    d.setDirectory(ui->label->text());
    d.selectFile(ui->label_2->text());
    d.exec();

    qDebug()<<d.selectedUrls();
    qDebug()<<d.directory();
}

void MainWindow::on_pushButton_2_clicked()
{
    auto d = new QFileDialog;
    d->setDirectory(ui->label->text());
    d->selectFile(ui->label_2->text());
    d->show();
}

void MainWindow::on_pushButton_3_clicked()
{
    QFileDialog d;
    d.setAcceptMode(QFileDialog::AcceptOpen);
    d.setDirectory(ui->label->text());
    d.selectFile(ui->label_2->text());
    d.exec();

    qDebug()<<d.selectedUrls();
    qDebug()<<d.directory();
}

void MainWindow::on_pushButton_4_clicked()
{
    QFileDialog d;
    d.setAcceptMode(QFileDialog::AcceptSave);
    d.setDirectory(ui->label->text());
    d.selectFile(ui->label_2->text());
    d.exec();

    qDebug()<<d.selectedUrls();
    qDebug()<<d.directory();
}

void MainWindow::on_pushButton_5_clicked()
{
    qDebug()<<QFileDialog::getOpenFileUrl(0, "test", ui->label->text());
}
