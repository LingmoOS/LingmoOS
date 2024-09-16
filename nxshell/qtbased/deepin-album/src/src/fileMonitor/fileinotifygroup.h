// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINOTIFYGROUP_H
#define FILEINOTIFYGROUP_H

#include <QObject>

class FileInotify;

class FileInotifyGroup : public QObject
{
    Q_OBJECT
public:
    explicit FileInotifyGroup(QObject *parent = nullptr);
    ~FileInotifyGroup() override;

    //添加：path下的改动会实时同步到对应的album中
    void startWatch(const QStringList &paths, const QString &album, int UID);

signals:
    //监控到改变
    void sigMonitorChanged(QStringList fileAdd, QStringList fileDelete, QString album, int UID);
    //自动导入路径被删除
    void sigMonitorDestroyed(int UID);
private:
    QList<FileInotify *> watchers;
};

#endif // FILEINOTIFYGROUP_H
