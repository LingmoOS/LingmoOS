// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcomworker.h"
#include "dfontinfomanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfmdbmanager.h"
#include "dcopyfilesmanager.h"

#include <QDir>

#include <unistd.h>

FontManager *FontManager::m_fontManager = nullptr;

FontManager *FontManager::instance()
{
    if (m_fontManager == nullptr) {
        m_fontManager = new FontManager;
    }

    return m_fontManager;
}

DComWorker::DComWorker(QObject *parent)
    : QObject(parent)
{
    if (!autoDelete())
        setAutoDelete(true);
}

void DComWorker::run()
{
}

GetFontListWorker::GetFontListWorker(GetFontListWorker::Type type, QObject *parent)
    : DComWorker(parent)
    , m_type(type)
{
}


void GetFontListWorker::run()
{
    qDebug() << __FUNCTION__ << m_type << "begin";
    DFontInfoManager *inst = DFontInfoManager::instance();
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();
    if (m_type == Startup) {
        thread->m_allFontPathList.clear();
        thread->m_allFontPathList = inst->getAllFontPath(true);
//        removeUserAddFonts();

        if(DFMDBManager::instance()->isDBDeleted()){
            thread->updateDb();//想把它放在第1个，getStartFontList之前
            DFMDBManager::instance()->getAllRecords();
            QList<DFontPreviewItemData> list = DFMDBManager::instance()->getFontInfo(50, &thread->m_delFontInfoList);//DFMDBManager::recordList
            thread->m_fontModelList.clear();
            thread->m_fontModelList.append(list);
        }
        else {
            DFMDBManager::instance()->getAllRecords();
            QList<DFontPreviewItemData> list = DFMDBManager::instance()->getFontInfo(50, &thread->m_delFontInfoList);
            thread->m_startModelList = list;
            thread->m_fontModelList.append(list);
        }
    }

    if (m_type == ALL || m_type == AllInSquence) {
        thread->m_allFontPathList.clear();
        thread->m_allFontPathList = inst->getAllFontPath(false);
    }

    if (m_type == CHINESE || m_type == AllInSquence) {
        thread->m_chineseFontPathList.clear();
        thread->m_chineseFontPathList = inst->getAllChineseFontPath();
    }

    if (m_type == MONOSPACE || m_type == AllInSquence) {
        thread->m_monoSpaceFontPathList.clear();
        thread->m_monoSpaceFontPathList = inst->getAllMonoSpaceFontPath();
    }
    qDebug() << __FUNCTION__ << m_type << "end";
}

void GetFontListWorker::removeUserAddFonts()
{
    if (geteuid() == 0) {
        return;
    }
    QStringList installFont = DFMDBManager::instance()->getInstalledFontsPath();

    QList<QString>::iterator iter = DFontPreviewListDataThread::instance()->m_allFontPathList.begin();
    while (iter != DFontPreviewListDataThread::instance()->m_allFontPathList.end()) {
        QString filePath = *iter;
        if (!DFMDBManager::instance()->isSystemFont(filePath) && !installFont.contains(filePath)) {
            QFileInfo openFile(filePath);
            QFile::remove(filePath);

            QDir fileDir(openFile.path());
            if (fileDir.isEmpty()) {
                fileDir.removeRecursively();
            }

            iter = DFontPreviewListDataThread::instance()->m_allFontPathList.erase(iter);
        } else {
            ++iter;
        }
    }
}

void FontManager::getFontListInSequence()
{
    GetFontListWorker getFontList(GetFontListWorker::AllInSquence);
    getFontList.run();
}


/*************************************************************************
 <Function>      getStartFontList
 <Description>   获取启动时的需要用到的字体列表
 <Author>        null
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManager::getStartFontList()
{
    QThreadPool *threadPool = DCopyFilesManager::instance()->getPool();

    GetFontListWorker *getAll = new GetFontListWorker(GetFontListWorker::Startup);
    threadPool->start(getAll);
    GetFontListWorker *getChinese = new GetFontListWorker(GetFontListWorker::CHINESE);
    threadPool->start(getChinese);
    GetFontListWorker *getMonospace = new GetFontListWorker(GetFontListWorker::MONOSPACE);
    threadPool->start(getMonospace);
    threadPool->waitForDone();
}

/*************************************************************************
 <Function>      getChineseAndMonoFont
 <Description>   获取当前系统中的中文字体和等宽字体
 <Author>        null
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManager::getChineseAndMonoFont()
{
    QThreadPool *threadPool = DCopyFilesManager::instance()->getPool();

    GetFontListWorker *getChinese = new GetFontListWorker(GetFontListWorker::CHINESE);
    threadPool->start(getChinese);
    GetFontListWorker *getMonospace = new GetFontListWorker(GetFontListWorker::MONOSPACE);
    threadPool->start(getMonospace);
    threadPool->waitForDone();
}
