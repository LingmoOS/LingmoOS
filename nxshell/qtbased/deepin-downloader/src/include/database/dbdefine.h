// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
* @file %{CurrentDocument:dbdefine.h}
* @brief
* @author bulongwei  <bulongwei@uniontech.com>
* @version 1.0.0
* @date %{CurrentDate:2020-05-26} %{CurrentTime:17:59}
* @copyright 2020-%{CurrentDate:2020} Uniontech Technology Co., Ltd.
*/

#ifndef DBDEFINE_H
#define DBDEFINE_H
#include <QString>
#include <QDateTime>

struct TaskStatus
{
    QString taskId; //任务id，主键唯一标识
    int downloadStatus; //任务状态
    QDateTime modifyTime; //修改时间
    QString compeletedLength ; //已下载大小
    QString  downloadSpeed ; //下载速度
    QString  totalLength; //下载总文件大小
    int percent;          //当前下载百分比
    int totalFromSource; //获取的下载文件大小
    QDateTime finishTime; //完成时间
    TaskStatus(){
        taskId = "";
        downloadStatus = -1;
        compeletedLength = "";
        downloadSpeed = "";
        totalLength = "";
        percent  = -1;
        totalFromSource = 0;
    };
    TaskStatus(QString task_id,int download_status,QDateTime modify_time,QString compLength,
                  QString downSpeed,QString totalLen,int per,int total, QDateTime finishTi)
    {
        taskId = task_id;
        downloadStatus = download_status;
        modifyTime.setDate(modify_time.date());
        modifyTime.setTime(modify_time.time());
        compeletedLength = compLength ;
        downloadSpeed = downSpeed;
        totalLength = totalLen;
        if (per < 0 || per > 100) {
            percent = 0;
        } else {
            percent = per;
        }
        totalFromSource = total;
        finishTime = finishTi;
    };
};


struct TaskInfo
{
    QString taskId; //任务id，主键唯一标识
    QString gid; //下载gid
    int gidIndex; //位置index
    QString url; //下载url地址
    QString fileLength; //文件大小
    QString downloadPath; //下载全路径包括文件名
    QString downloadFilename; //下载文件名
    QDateTime createTime; //任务创建时间
    TaskInfo(){
        taskId = "";
        gid = "";
        gidIndex = -1;
        url = "";
        downloadPath = "";
        downloadFilename = "";
    };
    TaskInfo(QString task_id,QString gId,int gid_index,QString Url,QString download_path,
                QString download_filename,QDateTime create_time)
    {
        taskId = task_id;
        gid = gId;
        gidIndex = gid_index;
        url = Url;
        downloadPath = download_path;
        downloadFilename = download_filename;
        createTime.setDate(create_time.date());
        createTime.setTime(create_time.time());
    };
};

struct TaskInfoHash
{
    QString taskId ;//任务id唯一标识
    QString url; //下载url
    QString downloadType; //下载类型
    QString filePath; //种子/metalink文件路径
    QString selectedNum; //选择的种子文件序号
    QString infoHash; //种子文件hash值
    TaskInfoHash(){
        taskId = "" ;
        url = "";
        downloadType = "";
        filePath = "";
        selectedNum = "";
        infoHash = "";
    };
    TaskInfoHash(QString task_id ,QString Url ,QString type,
               QString seed,QString selNum,QString hash)
    {
        taskId = task_id ;
        url = Url;
        downloadType = type;
        filePath = seed;
        selectedNum = selNum;
        infoHash = hash;
    }
};

#endif // DBDEFINE_H
