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

#include <QListWidget>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_tool_bar = new QToolBar(this);
    m_tool_bar->addAction("test1");
    m_tool_bar->addAction("test2");
    m_tool_bar->addAction("test3");
    m_tool_bar->addAction("test4");
    addToolBar(m_tool_bar);

    m_list_view = new QListWidget(this);
    m_list_view->setAttribute(Qt::WA_TranslucentBackground);
    m_list_view->setAttribute(Qt::WA_Hover);
    m_list_view->setStyleSheet("background: transparent");
    m_list_view->addItem("test1");
    m_list_view->addItem("test2");
    m_list_view->addItem("test3");
    m_list_view->addItem("test4");
    setCentralWidget(m_list_view);
}

MainWindow::~MainWindow()
{

}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    m_blur_region = m_tool_bar->rect();
}
