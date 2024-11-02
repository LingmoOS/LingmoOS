/*
 * Copyright 2020. kylinos.cn.
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
#ifndef UKMEDIAMONITORWINDOWTHREAD_H
#define UKMEDIAMONITORWINDOWTHREAD_H

#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <X11/Xlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <X11/extensions/shape.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <X11/Xatom.h>
#include <QVector>

class UkmediaMonitorWindowThread : public QThread
{
public:
    UkmediaMonitorWindowThread();
    void get_window_nameAndid();
    //结束线程标志位
    bool bStopThread;
    bool bCreateWindow;
    bool bFirstEnterSystem;
    QVector<unsigned long> windowId;
    void run();
};

#endif // UKMEDIAMONITORWINDOWTHREAD_H
