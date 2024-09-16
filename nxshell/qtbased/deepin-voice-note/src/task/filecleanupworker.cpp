// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecleanupworker.h"
#include "common/vnoteitem.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

FileCleanupWorker::FileCleanupWorker(VNOTE_ALL_NOTES_MAP *qspAllNotesMap, QObject *parent)
    : VNTask(parent)
    , m_qspAllNotesMap(qspAllNotesMap)
{
}
void FileCleanupWorker::run()
{
    if (nullptr == m_qspAllNotesMap) {
        return;
    }

    //获取所有语音文件路径
    fillVoiceSet();
    //获取所有图片文件路径
    fillPictureSet();
    //遍历笔记
    if (scanAllNotes()) {
        //清空数据
        cleanVoice();
        cleanPicture();
    }
}

/**
 * @brief FileCleanupWorker::cleanVoice
 * 清理语音
 */
void FileCleanupWorker::cleanVoice()
{
    for (QString path : m_voiceSet) {
        if (!QFile::remove(path)) {
            qCritical() << "remove file " << path << " failed!";
        }
    }
}

/**
 * @brief FileCleanupWorker::cleanPicture
 * 清理图片
 */
void FileCleanupWorker::cleanPicture()
{
    for (QString path : m_pictureSet) {
        if (!QFile::remove(path)) {
            qCritical() << "remove file " << path << " failed!";
        }
    }
}

/**
 * @brief FileCleanupWorker::fillVoiceSet
 * 获取项目下用户所有的语音完整路径
 */
void FileCleanupWorker::fillVoiceSet()
{
    //文件夹路径
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/voicenote";
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }

    //存放文件路径
    for (auto fileName : dir.entryList(QStringList("*.mp3"), QDir::Files | QDir::NoSymLinks)) {
        m_voiceSet.insert(dirPath + "/" + fileName);
    }
}

/**
 * @brief FileCleanupWorker::fillPictureSet
 * 获取项目下用户所有的图片完整路径
 */
void FileCleanupWorker::fillPictureSet()
{
    //文件夹路径
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/images";
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }

    QStringList filters = {"*.png", "*.jpg", "*.bmp"};
    for (auto fileName : dir.entryList(filters, QDir::Files | QDir::NoSymLinks)) {
        m_pictureSet.insert(dirPath + "/" + fileName);
    }
}

/**
 * @brief FileCleanupWorker::scanAllNotes
 * 遍历所有的笔记
 * @return true: 遍历成功， false: 遍历失败
 */
bool FileCleanupWorker::scanAllNotes()
{
    //遍历笔记
    QList<VNOTE_ITEMS_MAP *> voiceItems = m_qspAllNotesMap->notes.values();
    for (VNOTE_ITEMS_MAP *voiceItem : voiceItems) {
        QList<VNoteItem *> notes = voiceItem->folderNotes.values();
        for (VNoteItem *note : notes) {
            if (note->htmlCode.isEmpty()) {
                //5.9及以前版本的数据
                scanVoiceByBlocks(note->datas);
            } else {
                //遍历笔记内所有语音
                scanVoiceByHtml(note->htmlCode);
                //遍历笔记内所有图片
                scanPictureByHtml(note->htmlCode);
            }
        }
    }
    return true;
}

/**
 * @brief FileCleanupWorker::removeVoicePathBySet
 * 移除语音集合中的指定路径
 * @param path 待移除的路径
 */
void FileCleanupWorker::removeVoicePathBySet(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }
    //移除笔记内存在的路径
    m_voiceSet.remove(path);
}

/**
 * @brief FileCleanupWorker::removePicturePathBySet
 * 移除图片集合中的指定路径
 * @param path 待移除的路径
 */
void FileCleanupWorker::removePicturePathBySet(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }
    //移除笔记内存在的路径
    m_pictureSet.remove(path);
}

/**
 * @brief FileCleanupWorker::scanVoiceByHtml
 * 遍历笔记中的语音
 * @param html 笔记html文本
 */
void FileCleanupWorker::scanVoiceByHtml(const QString &htmlCode)
{
    //匹配语音块标签的正则表达式
    QRegExp rx("<div.+jsonkey.+>");
    rx.setMinimal(true); //最小匹配
    //匹配语音路径的正则表达式
    QRegExp rxJson("(/\\S+)+/voicenote/[\\w\\-]+\\.mp3");
    rxJson.setMinimal(false); //最大匹配
    QStringList list;
    int pos = 0;
    //查找语音块
    while ((pos = rx.indexIn(htmlCode, pos)) != -1) {
        //获取语音路径
        if (rxJson.indexIn(rx.cap(0)) != -1) {
            removeVoicePathBySet(rxJson.cap(0));
        }
        pos += rx.matchedLength();
    }
}

/**
 * @brief FileCleanupWorker::scanPictureByHtml
 * 遍历笔记中的图片
 * @param html 笔记html文本
 */
void FileCleanupWorker::scanPictureByHtml(const QString &htmlCode)
{
    //匹配图片块标签的正则表达式
    QRegExp rx("<img.+src=.+>");
    rx.setMinimal(true); //最小匹配
    //匹配本地图片路径的正则表达式（图片位置限制在images文件夹，后缀限制为a-z长度为3到4位）
    QRegExp rxPath("(/\\S+)+/images/[\\w\\-]+\\.[a-z]{3,4}");
    rxPath.setMinimal(false); //最大匹配
    int pos = 0;
    //查找图片块
    while ((pos = rx.indexIn(htmlCode, pos)) != -1) {
        //获取图片路径
        if (rxPath.indexIn(rx.cap(0)) != -1) {
            removePicturePathBySet(rxPath.cap(0));
        }
        pos += rx.matchedLength();
    }
}

/**
 * @brief scanVoiceByBlocks
 * 遍历5.9及以前版本笔记中的语音路径
 * @param datas
 */
void FileCleanupWorker::scanVoiceByBlocks(const VNOTE_DATAS &datas)
{
    for (auto data : datas.voiceBlocks) {
        if (VNoteBlock::Voice == data->getType()) {
            removeVoicePathBySet(data->ptrVoice->voicePath);
        }
    }
}
