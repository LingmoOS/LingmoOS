// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEOLDDATAMANAGER_H
#define VNOTEOLDDATAMANAGER_H

#include "common/datatypedef.h"
#include "db/vnotedbmanager.h"

#include <QObject>

class OldDataLoadTask;

class VNoteOldDataManager : public QObject
{
    Q_OBJECT
public:
    explicit VNoteOldDataManager(QObject *parent = nullptr);

    static VNoteOldDataManager *instance();
    static void releaseInstance();

    VNOTE_FOLDERS_MAP *folders();
    VNOTE_ALL_NOTES_MAP *allNotes();

    //Should be called before reqDatas
    //初始化老数据库
    void initOldDb();
    //启动老数据库数据加载线程
    void reqDatas();
    //启动升级线程
    void doUpgrade();
signals:
    //数据加载完成信号
    void dataReady();
    //升级结束信号
    void upgradeFinish();
    //进度改变信号
    void progressValue(int value);
public slots:
    //加载完成
    void onFinishLoad();
    //升级结束
    void onFinishUpgrade();
    //进度改变
    void onProgress(int value);

protected:
    static VNoteOldDataManager *_instance;
    static VNoteDbManager *m_oldDbManger;

    QScopedPointer<VNOTE_FOLDERS_MAP> m_qspNoteFoldersMap;
    QScopedPointer<VNOTE_ALL_NOTES_MAP> m_qspAllNotes;

    friend class OldDataLoadTask;
};

#endif // VNOTEOLDDATAMANAGER_H
