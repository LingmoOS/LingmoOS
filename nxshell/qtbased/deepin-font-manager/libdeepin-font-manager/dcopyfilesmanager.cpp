// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcopyfilesmanager.h"
#include "fontmanagercore.h"
#include "dfontinfomanager.h"
#include "dfmdbmanager.h"

#include <QDateTime>
#include <QThread>
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QDebug>

const QString sysDir = QDir::homePath() + "/.local/share/fonts";

CopyFontThread::CopyFontThread(OPType type, short index)
    : m_opType(type)
    , m_index(index)
{
    if (!autoDelete())
        setAutoDelete(true);
    if (!m_srcFiles.isEmpty())
        qDebug() << __FUNCTION__ << index << m_srcFiles.size();
}

void CopyFontThread::run()
{
    if (m_srcFiles.isEmpty())
        return;
    qint64 startTm = QDateTime::currentMSecsSinceEpoch();

//    qDebug() << __FUNCTION__ << m_index << m_srcFiles.size() << m_srcFiles;
    if (m_opType != EXPORT && m_opType != INSTALL)
        return;

    for (const QString &fontFile : m_srcFiles) {
        if (m_opType == EXPORT) {
            QString target = FONTS_DESKTOP_DIR + QFileInfo(fontFile).fileName();
            if (QFileInfo(target).exists() && (QFileInfo(fontFile).size() == QFileInfo(target).size())) {
                continue;
            } else {
                QFile(target).remove();
            }

            if (!QFile::copy(fontFile, target))
                qDebug() << __FUNCTION__ << " copy file error " << fontFile << m_index;
        } else if (m_opType == INSTALL) {
            if (DCopyFilesManager::isInstallCanceled()) {
                DCopyFilesManager::deleteFiles(m_targetFiles, true);
                return;
            }

            QString filePathOrig;
            QString target;
            QString familyName = DCopyFilesManager::getTargetPath(fontFile, filePathOrig, target);
            QFile::copy(filePathOrig, target);
            m_targetFiles << target;
            Q_EMIT fileInstalled(familyName, target);
        }
    }
    qDebug() << __FUNCTION__ << m_index << m_opType << m_srcFiles.size() << " take time (ms) " << QDateTime::currentMSecsSinceEpoch() - startTm;
}

void CopyFontThread::appendFile(const QString &filePath)
{
    if (!m_srcFiles.contains(filePath))
        m_srcFiles.append(filePath);
}

//文件拷贝类型：导出 安装
DCopyFilesManager *DCopyFilesManager::inst = new DCopyFilesManager();
qint8 DCopyFilesManager::m_type = CopyFontThread::INVALID;
//安装是否被取消
volatile bool DCopyFilesManager::m_installCanceled = false;

DCopyFilesManager::DCopyFilesManager(QObject *parent)
    : QObject(parent)
    , m_localPool(nullptr)
    , m_maxThreadCnt(static_cast<qint8>(QThread::idealThreadCount()))
    , m_exportMaxThreadCnt(static_cast<qint8>(QThread::idealThreadCount()))
    , m_installMaxThreadCnt(static_cast<qint8>(QThread::idealThreadCount()))
    , m_sortOrder(1)
{
    // libdeepin-font-manger.so在安装时需要安装com.deepin.font-manager.gschema.xml
    // 到目录/usr/share/glib-2.0/schemas中。
    // 现在取消libdeepin-font-manger.so，且此配置文件并没有实际作用，故不再使用此文件，
    // 所有配置与原配置文件中数据一致，不改变原业务逻辑。
    m_useGlobalPool = false;
    m_expiryTimeout = -1;

    if (!m_useGlobalPool) {
        m_localPool = new QThreadPool(this);
        m_localPool->setMaxThreadCount(QThread::idealThreadCount());
        if (m_expiryTimeout > 0) {
            m_localPool->setExpiryTimeout(m_expiryTimeout);
        }
    }

    int maxThreadCnt = m_maxThreadCnt > 0 ? m_maxThreadCnt : 1;
    getPool()->setMaxThreadCount(maxThreadCnt);
    if (m_expiryTimeout > 0) {
        getPool()->setExpiryTimeout(m_expiryTimeout);
    }

    qDebug() << __FUNCTION__ << "export max thread count = " << m_exportMaxThreadCnt << ", install max thread count = " << m_installMaxThreadCnt;
}

DCopyFilesManager *DCopyFilesManager::instance()
{
    return inst;
}

/**
* @brief DCopyFilesManager::copyFiles 拷贝文件列表，打印出拷贝文件耗用时间
* @param type 拷贝文件的类型：导出 安装
* @param fontList 要拷贝的源文件列表
* @return void
*/
void DCopyFilesManager::copyFiles(CopyFontThread::OPType type, QStringList &fontList)
{
    if (fontList.isEmpty())
        return;

    sortFontList(fontList);
    m_type = type;
    qint64 start = QDateTime::currentMSecsSinceEpoch();
    int tcount = 0;
    if (type == CopyFontThread::EXPORT) {
        tcount = m_exportMaxThreadCnt;
    } else {
        tcount = m_installMaxThreadCnt;
    }

    QList<CopyFontThread *> threads = QList<CopyFontThread *>();
    for (int i = 0; i < tcount; ++i) {
        CopyFontThread *thread = new CopyFontThread(type, static_cast<short>(i));
        threads.append(thread);
    }

    //debug log
    qDebug() << __FUNCTION__ << tcount  << type << fontList;

    int index = 0;
    int maxMod = (2 * tcount - 1) % (2 * tcount);

    for (const QString &file : fontList) {
        int modVal = (index % (2 * tcount));
        if (modVal < tcount) {
            threads.at(modVal)->appendFile(file);
        } else {
            threads.at(maxMod - modVal)->appendFile(file);
        }
        index++;
    }

    for (CopyFontThread *thread : threads) {
        if (type == CopyFontThread::INSTALL) {
            connect(thread, &CopyFontThread::fileInstalled, FontManagerCore::instance(), &FontManagerCore::onInstallResult);
        }
        getPool()->start(thread);
    }
    getPool()->waitForDone();

    if (m_installCanceled) {
        m_installCanceled = false;
        deleteFiles(fontList, false);
    }
    m_type = CopyFontThread::INVALID;
    qDebug() << __FUNCTION__ << " take (ms) :" << QDateTime::currentMSecsSinceEpoch() - start;
}

/**
* @brief DCopyFilesManager::getTargetPath 获取安装前字体的安装后目标路径
* @param inPath 传入的带familyName的字体路径
* @param srcPath 安装前字体的路径
* @param targetPath 安装后字体的路径
* @return 字体的familyName
*/
QString DCopyFilesManager::getTargetPath(const QString &inPath, QString &srcPath, QString &targetPath)
{
    QString targetDir;
    QStringList fileParamList = inPath.split("|");

    srcPath = fileParamList.at(0);
    QString familyName = fileParamList.at(1);

    //这里有过familyname中带有 /  的话，创建的目录会多一层，导致与其他不统一，也会造成删除时删除不完全的问题
    if (familyName.contains("/")) {
        familyName.replace("/", "-");
    }

    DFontInfo fontInfo = DFontInfoManager::instance()->getFontInfo(srcPath);
    QString src = DFMDBManager::instance()->isFontInfoExist(fontInfo);

    if (!src.isEmpty()) {
        targetPath = src;
        return familyName;
    }
    if(DFontInfoManager::instance()->isFontInInstalledDirs(fontInfo.filePath)){
        targetPath = QDir::homePath() + "/.local/share/fonts" + fontInfo.filePath.mid(fontInfo.filePath.lastIndexOf("/"));
        return familyName;
    }

    const QFileInfo info(srcPath);
    QString dirName = familyName;

    if (dirName.isEmpty()) {
        dirName = info.baseName();
    }

    targetPath = QString("%1/%2/%3").arg(sysDir).arg(dirName).arg(info.fileName());
    targetDir = QString("%1/%2").arg(sysDir).arg(dirName);

    QDir dir(targetDir);
    dir.mkpath(".");
    return familyName;
}

/**
* @brief DCopyFilesManager::deleteFiles 删除取消安装时已经安装的字体文件列表
* @param fileList 传入的待安装字体列表
* @param isTarget 是否是目标文件，还是带familyName的源文件路径
* @return void
*/
void DCopyFilesManager::deleteFiles(const QStringList &fileList, bool isTarget)
{
    for (const QString &font : fileList) {
        QString target = font;
        QString src;
        if (!isTarget)
            getTargetPath(font, src, target);

        QFile(target).remove();
        QDir fileDir(QFileInfo(target).path());
        if (fileDir.isEmpty()) {
            fileDir.removeRecursively();
        }
    }
}
