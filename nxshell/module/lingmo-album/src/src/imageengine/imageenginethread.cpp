// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageenginethread.h"
#include "dbmanager/dbmanager.h"
#include "unionimage/unionimage.h"
#include "albumControl.h"

#include <QDirIterator>

ImageEngineThreadObject::ImageEngineThreadObject()
{
    setAutoDelete(false); //从根源上禁止auto delete
}

void ImageEngineThreadObject::needStop(void *imageobject)
{
    if (nullptr == imageobject || ifCanStopThread(imageobject)) {
        bneedstop = true;
        bbackstop = true;
    }
}

bool ImageEngineThreadObject::ifCanStopThread(void *imgobject)
{
    Q_UNUSED(imgobject);
    return true;
}

void ImageEngineThreadObject::run()
{
    runDetail(); //原本要run的内容
    emit runFinished(); //告诉m_obj我run完了，这里不再像之前那样直接判断不为nullptr然后解引用m_obj，因为m_obj销毁后不会自动置为nullptr
    this->deleteLater(); //在消息传达后销毁自己
}

ImportImagesThread::ImportImagesThread()
{
    connect(this, &ImportImagesThread::sigRepeatUrls, AlbumControl::instance(), &AlbumControl::sigRepeatUrls);
    connect(this, &ImportImagesThread::sigImportProgress, AlbumControl::instance(), &AlbumControl::sigImportProgress);
    connect(this, &ImportImagesThread::sigImportFinished, AlbumControl::instance(), &AlbumControl::sigImportFinished);
    //通知前端刷新相关界面
    connect(this, &ImportImagesThread::sigImportFinished, AlbumControl::instance(), &AlbumControl::sigRefreshAllCollection);
    connect(this, &ImportImagesThread::sigImportFinished, AlbumControl::instance(), &AlbumControl::sigRefreshImportAlbum);
    connect(this, &ImportImagesThread::sigImportFinished, AlbumControl::instance(), &AlbumControl::sigRefreshSearchView);
    connect(this, &ImportImagesThread::sigImportFinished, [=]() {
        emit AlbumControl::instance()->sigRefreshCustomAlbum(-1);
    });
}

ImportImagesThread::~ImportImagesThread()
{

}

void ImportImagesThread::setData(const QStringList &paths, const int UID)
{
    for (QUrl path : paths) {
        m_paths << LibUnionImage_NameSpace::localPath(path);
    }
    m_UID = UID;
    m_type = DataType_String;
}

void ImportImagesThread::setData(const QList<QUrl> &paths, const int UID, const bool checkRepeat)
{
    for (QUrl path : paths) {
        m_paths << LibUnionImage_NameSpace::localPath(path);
    }
    m_UID = UID;
    m_checkRepeat = checkRepeat;
    m_type = DataType_Url;
}

void ImportImagesThread::setNotifyUI(bool bValue)
{
    m_notifyUI = bValue;
}

bool ImportImagesThread::ifCanStopThread(void *imgobject)
{
    Q_UNUSED(imgobject);
    return true;
}

void ImportImagesThread::runDetail()
{
    //相册中本次导入之前已导入的所有路径
    DBImgInfoList oldInfos = AlbumControl::instance()->getAllInfosByUID(QString::number(m_UID));
    QStringList allOldImportedPaths;
    for (DBImgInfo info : oldInfos) {
        allOldImportedPaths.push_back(info.filePath);
    }

    QStringList tempPaths;
    //判断是否含有目录
    for (QString path : m_paths) {
        //是目录，向下遍历,得到所有文件
        if (QDir(path).exists()) {
            QFileInfoList infos = LibUnionImage_NameSpace::getImagesAndVideoInfo(path, true);

            std::transform(infos.begin(), infos.end(), std::back_inserter(tempPaths), [](const QFileInfo & info) {
                return info.absoluteFilePath();
            });
        } else {//非目录
            tempPaths << path;
        }
    }

    //条件过滤
    int noReadCount = 0; //记录已存在于相册中的数量，若全部存在，则不进行导入操作
    for (QString imagePath : tempPaths) {
        //去掉不支持的图片和视频
        bool bIsVideo = LibUnionImage_NameSpace::isVideo(imagePath);
        if (!bIsVideo && !LibUnionImage_NameSpace::imageSupportRead(imagePath)) {
            noReadCount++;
            continue;
        }

        //当前文件不存在
        QFileInfo info(imagePath);
        if (!info.exists()) {
            noReadCount++;
            continue;
        }

        //判断文件是否可读
        if (!info.isReadable()) {
            noReadCount++;
            continue;
        }

        //已导入
        if (allOldImportedPaths.contains(imagePath)) {
            noReadCount++;
            continue;
        }

        m_filePaths << imagePath;
    }

    //已全部存在，无需导入
    if (noReadCount == tempPaths.size() && tempPaths.size() > 0 && m_checkRepeat) {
        QStringList urlPaths;
        for (QString path : tempPaths) {
            urlPaths.push_back("file://" + path);
        }
        emit sigRepeatUrls(urlPaths);
        return;
    }

    //导入所有
    DBImgInfo dbInfo;
    DBImgInfoList dbInfos;
    int i = 1;
    int size = m_filePaths.size();
    for (QString path : m_filePaths) {
        bool bIsVideo = LibUnionImage_NameSpace::isVideo(path);
        dbInfo =  AlbumControl::instance()->getDBInfo(path, bIsVideo);
        dbInfo.albumUID = QString::number(m_UID);
        dbInfos << dbInfo;

        emit sigImportProgress(i++, size);
    }

    std::sort(dbInfos.begin(), dbInfos.end(), [](const DBImgInfo & lhs, const DBImgInfo & rhs) {
        return lhs.changeTime > rhs.changeTime;
    });

    //导入图片数据库ImageTable3
    DBManager::instance()->insertImgInfos(dbInfos);

    //导入图片数据库AlbumTable3
    if (m_UID >= 0) {
        AlbumDBType atype = AlbumDBType::AutoImport;
        if (m_UID == 0) {
            atype = AlbumDBType::Favourite;
        }

        DBManager::instance()->insertIntoAlbum(m_UID, m_filePaths, atype);
    }

    //原createNewCustomAutoImportAlbum逻辑
    if (m_UID > 0) {
        emit AlbumControl::instance()->sigRefreshSlider();
        emit AlbumControl::instance()->sigAddCustomAlbum(m_UID);
    }

    QThread::msleep(100);
    //发送导入完成信号
    if (m_notifyUI)
        emit sigImportFinished();
}

