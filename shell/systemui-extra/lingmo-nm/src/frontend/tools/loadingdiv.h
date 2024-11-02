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

#ifndef LOADINGDIV_H
#define LOADINGDIV_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include <QList>
#include <QDebug>

class LoadingDiv : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingDiv(QWidget *parent = nullptr);

Q_SIGNALS:
    void toStopLoading();

public Q_SLOTS:
    void switchAnimStep();
    void startLoading();
    void stopLoading();

private:
    QLabel *loadingGif = nullptr;
    QTimer *switchTimer = nullptr;

    int currentPage;
    int countCurrentTime;
};

#endif // LOADINGDIV_H
