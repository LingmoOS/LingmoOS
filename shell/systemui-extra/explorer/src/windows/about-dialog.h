/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QObject>
#include <QDialog>
#include <QPoint>
#include <QMouseEvent>
#include <QGSettings>

#ifdef LINGMO_SDK_QT_WIDGETS
#include <kaboutdialog.h>
#endif

namespace Ui {
class AboutDialog;
}

#ifdef LINGMO_SDK_QT_WIDGETS
class AboutDialog : public kdk::KAboutDialog
#else
class AboutDialog : public QDialog
#endif
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();
    void resetSize();

private:
   void  initUI();
    QString getCurrentVersion();

private Q_SLOTS:
    void on_closeBtn_clicked();

private:
    Ui::AboutDialog *ui;
    QGSettings *m_gSettings;
    void setSupportText();
    //颜色转换
    QString convertRGB16HexStr(const QColor &color);
    bool m_isFirstLoad = false;
};

#endif // ABOUTDIALOG_H
