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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRegion>

class QListWidget;
class QToolBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QRegion blurRegion READ blurRegion WRITE setBlurRegion MEMBER m_region)
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const QRegion blurRegion() {return m_blur_region;}
    void setBlurRegion(const QRegion &region) {m_blur_region = region;}

protected:
    void resizeEvent(QResizeEvent *e);

private:
    QListWidget *m_list_view;
    QToolBar *m_tool_bar;

    QRegion m_blur_region = QRegion();
};

#endif // MAINWINDOW_H
