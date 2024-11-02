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

#include "utils.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>


///////////////////////////////////////////////////////////////////////////////
// The Utils class

Utils::Utils(){}

int Utils::m_system(char *cmd)
{
    int status = 0;
    pid_t pid;
    clock_t start,finish;
    if ((pid = vfork()) <0) {
        qDebug()<<"failed to create a subprocess by using vfork";
        status = -1;
    } else if (pid==0) {
        const char *new_argv[4];
        struct sigaction sa_cld;
        sa_cld.sa_handler = SIG_DFL;
        sa_cld.sa_flags = 0;

        // 在子进程中放开SIGINT信号
        sigemptyset(&sa_cld.sa_mask);
        sigaction (SIGINT, &sa_cld, NULL);
        sigaction (SIGQUIT, &sa_cld, NULL);

        new_argv[0] = "sh";
        new_argv[1] = "-c";
        new_argv[2] = cmd;
        new_argv[3] = NULL;

        // execl("/bin/sh","sh","-c" ,cmd,(char *)0);
        start = clock();
        if (execve("/bin/sh",(char *const *) new_argv, NULL) <0) {
            qDebug()<<"failed to execve a shell command in function m_system";
            exit(1);
        } else {
            exit(0);
        }
    } else {
        waitpid(pid,&status,0);
        finish = clock();
    }
    //double duration = (double)(finish-start)/CLOCKS_PER_SEC;
    //qDebug()<<"It takes "<<QString::number(duration)<<" seconds to execute command:"<<cmd;
    return status;
}

void Utils::onRequestSendDesktopNotify(QString message)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<(tr("Lingmo NM"))
       <<((unsigned int) 0)
      <<QString("gnome-dev-ethernet")
     <<tr("lingmo network applet desktop message") //显示的是什么类型的信息
    <<message //显示的具体信息
    <<QStringList()
    <<QVariantMap()
    <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}


///////////////////////////////////////////////////////////////////////////////
// The NetworkSpeed class, used to get network speed

NetworkSpeed::NetworkSpeed(QObject *parent) :QObject(parent){}

int NetworkSpeed::getCurrentDownloadRates(char *netname, long *save_rate, long *tx_rate)
{
    FILE * net_dev_file; //文件指针
    char buffer[1024]; //文件中的内容暂存在字符缓冲区里
    //size_t bytes_read; //实际读取的内容大小
    char * match; //用以保存所匹配字符串及之后的内容
    int counter = 0;
    //int i = 0;
    char tmp_value[128];

    if((NULL == netname)||(NULL == save_rate)||(NULL == tx_rate)) {
        qDebug()<<"parameter pass error";
        return -1;
    }

    if ( (net_dev_file=fopen("/proc/net/dev", "r")) == NULL ) { //打开文件/pro/net/dev/，从中读取流量数据
        qDebug()<<"error occurred when try to open file /proc/net/dev/";
        return -1;
    }
    memset(buffer,0,sizeof(buffer));

    while (fgets(buffer,sizeof(buffer),net_dev_file) != NULL) {
        match = strstr(buffer,netname);

        if (NULL == match) {
            // qDebug()<<"No eth0 keyword to find!";
            continue;
        } else {
            match = match + strlen(netname) + strlen(":"); //地址偏移到冒号
            sscanf(match,"%ld ",save_rate);
            memset(tmp_value,0,sizeof(tmp_value));
            sscanf(match,"%s ",tmp_value);
            match = match + strlen(tmp_value);
            for (size_t i=0;i<strlen(buffer);i++) {
                if (0x20 == *match) {
                    match ++;
                } else {
                    if (8 == counter) {
                        sscanf(match,"%ld ",tx_rate);
                    }
                    memset(tmp_value,0,sizeof(tmp_value));
                    sscanf(match,"%s ",tmp_value);
                    match = match + strlen(tmp_value);
                    counter ++;
                }
            }
        }
    }

    return 0; //返回成功
}
