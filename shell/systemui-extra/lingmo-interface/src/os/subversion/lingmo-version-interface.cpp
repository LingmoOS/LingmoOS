/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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


#include <QCoreApplication>
#include <QSettings>
#include <QDebug>
#include <QObject>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "lingmo-version-interface.h"

#define SUBVERSION "/var/lib/system-info/subversion"

//查找以File开头的key
int cmpKey(const char *keystr)
{
    int result=strncmp(keystr,"File",4);
//    qDebug()<<result;
    return result;
}

//查找文件/文件夹是否存在
int searchFile(const char *dirstr)
{
    int exist=access(dirstr,F_OK);
    return exist;
}

//获取当前子版本列表
subVersionCredibility *lingmo_os_sysinfo_getchildrroup()
{
    QSettings inifile(SUBVERSION,QSettings::IniFormat);
    //获取section列表
    QStringList groupList=inifile.childGroups();

    int secNum=groupList.size();
    subVersionCredibility *subVC=(subVersionCredibility *)malloc(sizeof(subVersionCredibility)*(secNum+1));
//    qDebug()<<"section个数："<<secNum;

    for(int i=0;i<secNum;i++)
    {
        int sum=0;
        int filenum=0;
        //分别查找每个section对应的键值对
        QString section=groupList.at(i);
        inifile.beginGroup(section);
//        qDebug()<<"["<<section<<"]";
        QStringList keyList=inifile.childKeys();
        for(int j=0;j<keyList.size();j++)
        {
            QString key=keyList.at(j);
            QByteArray kstr=key.toLatin1();
            char *keystr=kstr.data();
            int result=cmpKey(keystr);
            if(result==0)
            {
                //计算以File开头的文件总数量
                sum+=1;

                //Qt解码base64的字符串
                QString val=inifile.value(key).toString();
                QByteArray dir=QByteArray::fromBase64(val.toLatin1());
                char *dirstr=dir.data();
                //去除解码后的换行符
                *(dirstr+strlen(dirstr)-1)=0;

                //计算存在的文件数量
                int judge=searchFile(dirstr);
                if(judge==0){
                    filenum+=1;
                }
//                qDebug()<<keystr<<"="<<dirstr;
            }
        }

           //计算可信度
           //结构体数据填充
           int credibility;
           if(sum==0)credibility=0;
           else{
               credibility=filenum*100/sum;
           }
           subVC[i].credibility=credibility;
//           qDebug()<<"credibility="<<credibility;

           subVC[i].subVersionName=(char *)malloc(sizeof(char)*20);
           QByteArray secStr=section.toLatin1();
           char *sectionstr=secStr.data();
           strcpy(subVC[i].subVersionName,sectionstr);
//           qDebug()<<subVC[i].subVersionName<<subVC[i].credibility;

           inifile.endGroup();
    }

    subVC[secNum].subVersionName=NULL;
    subVC[secNum].credibility=0;
    return subVC;
}

//验证是否是给定的子版本
int lingmo_os_sysinfo_verifysubversion(const char *version)
{
    //判断配置文件是否存在
    int fexist=searchFile(SUBVERSION);

    if(fexist==0){
        //遍历section列表
        QSettings inifile(SUBVERSION,QSettings::IniFormat);
        QStringList groupList=inifile.childGroups();
        int secNum=groupList.size();
        for(int i=0;i<secNum;i++)
        {
            QString section=groupList.at(i);
            QByteArray secStr=section.toLatin1();
            char *sectionstr=secStr.data();

            //对比验证子版本号是否存在
            int cmp=strcmp(sectionstr,version);
            if(cmp==0){
                //从结构体中查找子版本号的可信度
                subVersionCredibility *subVC=(subVersionCredibility *)malloc(sizeof(subVersionCredibility)*(secNum+1));
                subVC=lingmo_os_sysinfo_getchildrroup();
                for(int i=0;i<secNum;i++)
                {
                    int cmpVer=strcmp(version,subVC[i].subVersionName);
                    if(cmpVer==0){
                        int credibility=subVC[i].credibility;
                        return credibility;
                    }
                }
            }
        }
        qDebug()<<"warning:The subversion does not exist";
        return 0;
    }
    else{
        qDebug()<<"warning:The configuration file does not exist";
    }

    return 0;

}

