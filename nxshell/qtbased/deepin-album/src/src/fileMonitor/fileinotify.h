// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINOTIFY_H
#define FILEINOTIFY_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QFileSystemWatcher>

class FileInotify : public QObject
{
    Q_OBJECT
public:
    explicit FileInotify(QObject *parent = nullptr);
    ~FileInotify() override;

    //添加和删除监控
    void addWather(const QStringList &paths, const QString &album, int UID);
    //void removeWatcher(const QString &path); //预留，暂未使用

    void clear();
    //获取监控目录所有照片
    void getAllPicture(bool isFirst);
    //文件数量改变
//    void fileNumChange(); //预留，暂未使用

signals:
    void pathDestroyed(int UID);

    //监控到改变
    void sigMonitorChanged(QStringList fileAdd, QStringList fileDelete, QString album, int UID);

public slots:
    //发送插入
    void onNeedSendPictures();

private:
    //检查是否新建了子文件夹
    void checkNewPath();

    bool m_running = false;
    QStringList m_newFile;      //当前新添加的
    QStringList m_deleteFile;   //当前删除的
    QStringList m_currentDirs;  //给定的当前监控路径
    QString m_currentAlbum;     //给定当前的相册
    int m_currentUID;           //给定当前的相册的UID
    QStringList  m_Supported;   //支持的格式
    QTimer *m_timer;
    QFileSystemWatcher m_watcher; //实际执行监控的类
};

#endif // FILEINOTIFY_H
