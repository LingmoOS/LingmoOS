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
#include "ukmedia_monitor_window_thread.h"
//#include <QDebug>
UkmediaMonitorWindowThread::UkmediaMonitorWindowThread()
{
    bStopThread = false;
    bCreateWindow = false;
    bFirstEnterSystem = true;
}

void UkmediaMonitorWindowThread::get_window_nameAndid()
{
    Display *display;
    Window rootwin;
    display = XOpenDisplay( NULL );
    rootwin = DefaultRootWindow( display );
    Atom atom = XInternAtom(display, "WM_DELETE_WINDOW", false);
    XSelectInput( display, rootwin, SubstructureNotifyMask | ExposureMask);/*事件可以参考x.h*/
    XEvent event;
//    while ( 1 )
//    {
//    printf("get event\n");
    fflush(stdout);
    while (true) {
        XNextEvent(display, &event );
//        printf("event type is %d\n",event.type);
        fflush(stdout);
        if (event.type == Expose /*ReparentNotify*/ /*&& bCreateWindow == false*/) {

            printf("ReparentNotify\n\n");
            fflush(stdout);
            if (bFirstEnterSystem == false) {
                Window x11window;
                Display *x11display;
                char **srname = (char **)malloc(sizeof(char *));
                XReparentEvent *reparentevent = (XReparentEvent *)&event;

                //printf( "new window: %ld \n", (unsigned long)(reparentevent->window));
                //printf( "new  parent: %ld \n", (unsigned long)(reparentevent->parent));
                fflush(stdout);
                windowId.append((unsigned long)reparentevent->window);
                /*获取到新建窗口的window ID*/
                x11window = (unsigned long)(reparentevent->window);
                x11display =(reparentevent->display);
                XFetchName(x11display, x11window, srname);
                x11display = XOpenDisplay( NULL );/*！！！注意这里以后如果你想对获取到的window ID进行操作必须重新链接*/

                fflush(stdout);
                free(srname);
                bCreateWindow = true;
                XCloseDisplay(x11display);
            }
            bFirstEnterSystem = false;

        }
        else if (event.type == /*DestroyNotify*//*ClientMessage*/ UnmapNotify /*&&*/ /*bCreateWindow*/) {
            bCreateWindow = false;
            // 执行用户定义的关闭窗口操作
            fflush(stdout);
            Window x11window;
            Display *x11display;
            char **srname = (char **)malloc(sizeof(char *));
            XUnmapEvent *reparentevent = (XUnmapEvent *)&event;
            //printf( "close window: %ld \n", (unsigned long)(reparentevent->window));
            fflush(stdout);
            if (windowId.contains((unsigned long)reparentevent->window)) {
                int index = windowId.indexOf((unsigned long)reparentevent->window);
                //printf( "remove ---------------------------- index : %d \n\n",index);
                fflush(stdout);
            windowId.removeAt(index);
            }
            /*获取到新建窗口的window ID*/
            x11window = (unsigned long)(reparentevent->window);
            x11display =(reparentevent->display);
            XFetchName(x11display, x11window, srname);
            x11display = XOpenDisplay( NULL );/*！！！注意这里以后如果你想对获取到的window ID进行操作必须重新链接*/

            fflush(stdout);
            free(srname);
            XCloseDisplay(x11display);
        }
    }
     XCloseDisplay(display);
}

//线程执行函数
void UkmediaMonitorWindowThread::run()
{
//    qDebug()<<"thread run!";
    bStopThread = false;
    while(!bStopThread)
    {
//        printf("thread run! \n\n");
        fflush(stdout);
        //to do what you want
        get_window_nameAndid();
        //延时等待一会
        QEventLoop eventloop;
        QTimer::singleShot(1000, &eventloop, SLOT(quit()));
        eventloop.exec();
    }
}
