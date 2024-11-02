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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListWidget>

#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBox->setEnabled(false);

    menu = new QMenu(ui->pushButton);
    menu->setProperty("fillIconSymbolicColor", true);
    menu->addAction(QIcon::fromTheme("edit-find-symbolic"), "edit-find-symbolic");
    menu->addAction(QIcon::fromTheme("edit-cut-symbolic"), "edit-cut-symbolic");
    menu->addAction(QIcon::fromTheme("user-trash"), "user-trash");
    menu->addAction(QIcon::fromTheme("open-menu-symbolic"), "open-menu-symbolic");
    menu->addAction(QIcon::fromTheme("pane-hide-symbolic"), "pane-hide-symbolic");
    ui->pushButton->setMenu(menu);

    view = new QListWidget(this);
    view->resize(300, 200);
    view->move(100, 200);

    auto item = new QListWidgetItem(QIcon::fromTheme("window-close"), "window-close", view);
    item = new QListWidgetItem(QIcon::fromTheme("window-close-symbolic"), "window-close-symoblic", view);
    item = new QListWidgetItem(QIcon::fromTheme("user-trash"), "user-trash", view);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_checkBox_toggled(bool checked)
{
    ui->pushButton->setProperty("useIconHighlightEffect", checked);
    ui->pushButton->update();
    ui->toolButton->setProperty("useIconHighlightEffect", checked);
    ui->toolButton->update();
    view->setProperty("useIconHighlightEffect", checked);
    view->viewport()->update();
    menu->setProperty("useIconHighlightEffect", checked);
    ui->comboBox->setEnabled(checked);

    int mode = ui->comboBox->currentIndex();
    ui->pushButton->setProperty("iconHighlightEffectMode", mode);
    ui->pushButton->update();
    ui->toolButton->setProperty("iconHighlightEffectMode", mode);
    ui->toolButton->update();
    view->setProperty("iconHighlightEffectMode", mode);
    view->viewport()->update();
    menu->setProperty("iconHighlightEffectMode", mode);
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    int mode = index;
    ui->pushButton->setProperty("iconHighlightEffectMode", mode);
    ui->pushButton->update();
    ui->toolButton->setProperty("iconHighlightEffectMode", mode);
    ui->toolButton->update();
    view->setProperty("iconHighlightEffectMode", mode);
    view->viewport()->update();
}
