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

#include <QStyleFactory>
#include <QDir>
#include <QStringListModel>
#include <QGSettings>
#include <QFontDatabase>

bool init_style = false;
bool init_icon_theme = false;
bool init_system_font = false;
bool init_system_font_size = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QGSettings settings("org.lingmo.style", "/org/lingmo/style/");
    auto currentStyle = settings.get("styleName").toString();
    auto currentIconTheme = settings.get("iconThemeName").toString();

    auto styles = QStyleFactory::keys();
    styles.removeOne("lingmo");
    QStringListModel *style_model = new QStringListModel(styles, this);
    ui->comboBox->setModel(style_model);
    ui->comboBox->setCurrentText(currentStyle);

    QDir iconThemePaths = QDir("/usr/share/icons");
    auto iconThemes = iconThemePaths.entryList(QDir::Dirs);
    QStringListModel *icon_theme_model = new QStringListModel(iconThemes, this);
    ui->comboBox_2->setModel(icon_theme_model);
    ui->comboBox_2->setCurrentText(currentIconTheme);

    QFontDatabase db;
    QStringListModel *fonts_model = new QStringListModel(db.families(), this);
    ui->comboBox_3->setModel(fonts_model);
    ui->comboBox_3->setCurrentText(settings.get("systemFont").toString());

    ui->spinBox->setValue(settings.get("systemFontSize").toString().toDouble());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if (!init_style) {
        init_style = true;
        return;
    }
    //change style
    QGSettings settings("org.lingmo.style", "/org/lingmo/style/");
    settings.set("styleName", arg1);
}

void MainWindow::on_comboBox_2_currentIndexChanged(const QString &arg1)
{
    if (!init_icon_theme) {
        init_icon_theme = true;
        return;
    }
    //change icon theme
    QGSettings settings("org.lingmo.style", "/org/lingmo/style/");
    settings.set("iconThemeName", arg1);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    QGSettings settings("org.lingmo.style", "/org/lingmo/style/");
    settings.set("menuTransparency", value);
}

void MainWindow::on_comboBox_3_currentTextChanged(const QString &arg1)
{
    if (!init_system_font) {
        init_system_font = true;
        return;
    }

    bool enableMenuSettings = true;
    if (arg1 != "lingmo-white" || arg1 != "lingmo-black") {
        enableMenuSettings = false;
    }

    ui->horizontalSlider->setEnabled(enableMenuSettings);

    QGSettings settings("org.lingmo.style", "/org/lingmo/style/");
    settings.set("systemFont", arg1);
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if (!init_system_font_size) {
        init_system_font_size = true;
        return;
    }
    QGSettings settings("org.lingmo.style", "/org/lingmo/style/");
    settings.set("systemFontSize", arg1);
}
