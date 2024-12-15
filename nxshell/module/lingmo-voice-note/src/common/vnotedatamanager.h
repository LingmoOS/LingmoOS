// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEDATAMANAGER_H
#define VNOTEDATAMANAGER_H

#include "datatypedef.h"

#include <QObject>

class LoadFolderWorker;
class LoadNoteItemsWorker;
class VNoteFolderOper;
class VNoteItemOper;
class FolderQryDbVisitor;
struct VNoteFolder;

class VNoteDataManager : public QObject
{
    Q_OBJECT
public:
    explicit VNoteDataManager(QObject *parent = nullptr);

    static VNoteDataManager *instance();

    //获取所有记事本数据
    VNOTE_FOLDERS_MAP *getNoteFolders();
    //获取所有记事项数据
    VNOTE_ALL_NOTES_MAP *getAllNotesInFolder();
    //加载默认图标
    void reqNoteDefIcons();
    //加载记事本数据
    void reqNoteFolders();
    //加载记事项数据
    void reqNoteItems();
signals:
    //记事本数据加载完成
    void onNoteFoldersLoaded();
    //记事项数据加载完成
    void onNoteItemsLoaded();
    //所有数据加载完成
    void onAllDatasReady();

public slots:
    //加载记事本数据线程执行完成
    void onFoldersLoaded(VNOTE_FOLDERS_MAP *foldesMap);
    //加载笔记数据线程执行完成
    void onAllNotesLoaded(VNOTE_ALL_NOTES_MAP *notesMap);

protected:
    //添加一个记事本
    VNoteFolder *addFolder(VNoteFolder *folder);
    //获取记事本数据
    VNoteFolder *getFolder(qint64 folderId);
    //删除一个记事本
    VNoteFolder *delFolder(qint64 folderId);
    //获取记事本数量
    qint32 folderCount();
    //添加一个记事项
    VNoteItem *addNote(VNoteItem *note);
    //获取一个记事项
    VNoteItem *getNote(qint64 folderId, qint32 noteId);
    //删除记事项
    VNoteItem *delNote(qint64 folderId, qint32 noteId);
    //获取一个记事本的记事项个数
    qint32 folderNotesCount(qint64 folderId);
    //获取一个记事本的所有记事项数据
    VNOTE_ITEMS_MAP *getFolderNotes(qint64 folderId);
    //获取记事本图标
    QPixmap getDefaultIcon(qint32 index, IconsType type);

private:
    QScopedPointer<VNOTE_FOLDERS_MAP> m_qspNoteFoldersMap;
    QScopedPointer<VNOTE_ALL_NOTES_MAP> m_qspAllNotesMap;

    LoadFolderWorker *m_pForldesLoadThread {nullptr};
    LoadNoteItemsWorker *m_pNotesLoadThread {nullptr};

    //State used to check whether data have been loaded
    // or not.
    enum DataState {
        DataNotLoaded = 0x0,
        FolderDataReady = (0x1),
        NotesDataReady = (0x1 << 1),
    };

    int m_fDataState = {DataNotLoaded};

    bool isAllDatasReady() const;

    static VNoteDataManager *_instance;

    static QVector<QPixmap> m_defaultIcons[IconsType::MaxIconsType];
    static QReadWriteLock m_iconLock;

    friend class LoadIconsWorker;
    friend class VNoteFolderOper;
    friend class VNoteItemOper;
    friend class FolderQryDbVisitor;
    friend struct VNoteFolder;
};

#endif // VNOTEDATAMANAGER_H
