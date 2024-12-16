/*
 * Copyright (c) 2017 Ragnar Thomsen <rthomsen6@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "libzipplugin.h"
#include "common.h"
#include "queries.h"
#include "datamanager.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <qplatformdefs.h>
#include <QDirIterator>
#include <QTimer>
#include <QDataStream>
#include <QTextCodec>

#include <utime.h>

//#include <zlib.h>
#define READBYTES 10240         // 每次读取文件大小

LibzipPluginFactory::LibzipPluginFactory()
{
    registerPlugin<LibzipPlugin>();
}

LibzipPluginFactory::~LibzipPluginFactory()
{

}



LibzipPlugin::LibzipPlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
    qInfo() << "LibzipPlugin";
    m_ePlugintype = PT_Libzip;
    m_listCodecs.clear();
    m_listCodecs << "UTF-8" << "GB18030" << "GBK" << "Big5" << "us-ascii";  // 初始化中文编码格式
}

LibzipPlugin::~LibzipPlugin()
{

}

PluginFinishType LibzipPlugin::list()
{
    qInfo() << "LibzipPlugin插件加载压缩包数据";
    setPassword(QString());
    m_mapFileCode.clear();
    m_setHasHandlesDirs.clear();
    m_setHasRootDirs.clear();
    DataManager::get_instance().resetArchiveData();

    // 处理加载流程
    int errcode = 0;
    zip_error_t err;

    // 打开压缩包文件
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_RDONLY, &errcode);   // 打开压缩包文件
    zip_error_init_with_code(&err, errcode);

    // 若打开失败，返回错误
    if (nullptr == archive) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    // 获取文件压缩包文件数目
    const auto nofEntries = zip_get_num_entries(archive, 0);

    // 获取压缩包的注释内容
    QByteArray strCode;
    m_strComment = m_common->trans2uft8(zip_get_archive_comment(archive, nullptr, ZIP_FL_ENC_RAW), strCode);

    // 循环构建数据
    for (zip_int64_t i = 0; i < nofEntries; i++) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        handleArchiveData(archive, i);  // 构建数据
    }

    zip_close(archive);

    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::testArchive()
{
    m_workStatus = WT_Test;
    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    qInfo() << "解压缩数据";

    setPassword(QString());
    m_workStatus = WT_Extract;
    int errcode = 0;
    m_bOverwriteAll = false;        //是否全部覆盖
    m_bSkipAll = false;             // 是否全部跳过
    m_mapLongName.clear();
    m_setLongName.clear();
    m_mapLongDirName.clear();
    m_mapRealDirValue.clear();
//    m_bHandleCurEntry = false; //false:提取使用选中文件及子文件 true:提取使用选中文件
    zip_error_t err;

    // 打开压缩包
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_RDONLY, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (nullptr == archive) {
        // 特殊包操作
        // return minizip_extractFiles(files, options);
        m_eErrorType = ET_ArchiveDamaged ;
        return PFT_Error;
    }

    // 右键解压时按照按照压缩包大小计算
    if (!options.bExistList) {
        m_dScaleSize = 100.0 / options.qComressSize;
    } else {
        m_dScaleSize = 100.0 / options.qSize;
    }
    m_bDlnfs = m_common->isSubpathOfDlnfs(options.strTargetPath);

    // 执行解压操作
    bool bHandleLongName = false;
    if (options.bAllExtract) {  // 全部解压
        qlonglong qExtractSize = 0;
        zip_int64_t nofEntries = zip_get_num_entries(archive, 0);
        for (zip_int64_t i = 0; i < nofEntries; ++i) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                m_bCancel = false;      // 重置标志位
                break;
            }

            QString strFileName;

            // 解压单个文件
            m_eErrorType = extractEntry(archive, i, options, qExtractSize, strFileName, bHandleLongName);

            // 方便右键解压时提示是否有数据解压出来
            if (!options.bExistList && 0 == i) {
                FileEntry entry;
                entry.strFullPath = strFileName;
                DataManager::get_instance().archiveData().listRootEntry << entry;
            }

            if (ET_NoError == m_eErrorType || (bHandleLongName == true && ET_NoError == m_eErrorType)) {  // 无错误，继续解压下一个文件
                continue;
            } else if (ET_UserCancelOpertion == m_eErrorType) {    // 用户取消，结束解压，返回结束标志
                zip_close(archive);
                return PFT_Cancel;
            } else {    // 处理错误
                // 判断是否需要密码，若需要密码，弹出密码输入对话框，用户输入密码之后，重新解压当前文件
                if (ET_WrongPassword == m_eErrorType || ET_NeedPassword == m_eErrorType) {
                    PasswordNeededQuery query(strFileName);
                    emit signalQuery(&query);
                    query.waitForResponse();

                    if (query.responseCancelled()) {
                        setPassword(QString());
                        zip_close(archive);
                        return PFT_Cancel;
                    } else {
                        setPassword(query.password());
                        zip_set_default_password(archive, m_strPassword.toUtf8().constData());
                        i--;
                    }
                }  else {
                    zip_close(archive);
                    return PFT_Error;
                }

            }
        }
    } else { // 部分提取
        qlonglong qExtractSize = 0;
        m_listCurIndex.clear();
        getIndexBySelEntry(files);    // 获取索引值

        // 提取指定文件
        for (int i = 0; i < m_listCurIndex.count(); ++i) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            QString strFileName;

            // 解压单个文件
            m_eErrorType = extractEntry(archive, m_listCurIndex[i], options, qExtractSize, strFileName, bHandleLongName);

            if (ET_NoError == m_eErrorType || (bHandleLongName == true && ET_NoError == m_eErrorType)) {  // 无错误，继续解压下一个文件
                continue;
            } else if (ET_UserCancelOpertion == m_eErrorType) {    // 用户取消，结束解压，返回结束标志
                zip_close(archive);
                return PFT_Cancel;
            } else {    // 处理错误
                // 判断是否需要密码，若需要密码，弹出密码输入对话框，用户输入密码之后，重新解压当前文件
                if (ET_WrongPassword == m_eErrorType || ET_NeedPassword == m_eErrorType) {

                    PasswordNeededQuery query(strFileName);
                    emit signalQuery(&query);
                    query.waitForResponse();

                    if (query.responseCancelled()) {
                        setPassword(QString());
                        zip_close(archive);
                        return PFT_Cancel;
                    } else {
                        setPassword(query.password());
                        zip_set_default_password(archive, m_strPassword.toUtf8().constData());
                        i--;
                    }
                } else {
                    zip_close(archive);
                    return PFT_Error;
                }

            }
        }
    }

    if (bHandleLongName == true) {
        m_eErrorType = ET_LongNameError;
    }

    zip_close(archive);
    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    setPassword(QString());
    m_workStatus = WT_Add;
    qInfo() << "添加压缩包数据";
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名
    zip_error_init_with_code(&err, errcode);
    if (!archive) {
        emit error(("Failed to open the archive: %1")); //ReadOnlyArchiveInterface::error
        return PFT_Error;
    }

    m_curFileCount = 0;
    for (const FileEntry &e : files) {
        // 过滤上级目录（不对全路径进行压缩）
        QString strPath = QFileInfo(e.strFullPath).absolutePath() + QDir::separator();

        //取消按钮 结束
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        // If entry is a directory, traverse and add all its files and subfolders.
        QFileInfo info(e.strFullPath);
        if (QFileInfo(e.strFullPath).isDir() && !info.isSymLink()) {
            if (!writeEntry(archive, e.strFullPath, options, true, strPath)) {
                if (zip_close(archive)) {
                    emit error(("Failed to write archive."));
                    return PFT_Error;
                }
                return PFT_Error;
            }

            QDirIterator it(e.strFullPath,
                            QDir::AllEntries | QDir::Readable |
                            QDir::Hidden | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);

            while (!QThread::currentThread()->isInterruptionRequested() && it.hasNext()) {
                const QString path = it.next();

                if (QFileInfo(path).isDir()&&!QFileInfo(path).isSymLink()) {
                    if (!writeEntry(archive, path, options, true, strPath)) {
                        if (zip_close(archive)) {
                            emit error(("Failed to write archive."));
                            return PFT_Error;
                        }
                        return PFT_Error;
                    }
                } else {
                    if (!writeEntry(archive, path, options, false, strPath)) {
                        if (zip_close(archive)) {
                            emit error(("Failed to write archive."));
                            return PFT_Error;
                        }
                        return PFT_Error;
                    }
                }
                ++m_curFileCount;
            }
        } else {
            if (!writeEntry(archive, e.strFullPath, options, false, strPath)) {
                if (zip_close(archive)) {
                    emit error(("Failed to write archive."));
                    return PFT_Error;
                }
                return PFT_Error;
            }
        }
        ++m_curFileCount;
    }


    m_pCurArchive = archive;
    // TODO:Register the callback function to get progress feedback.
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);
    zip_register_cancel_callback_with_state(archive, cancelCallback, nullptr, this);

    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
        // ZIP_ER_WRITE    /* S Write error */
        // ENOSPC          /* No space left on device */
        if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_WRITE && zip_error_code_system(zip_get_error(archive)) == ENOSPC) {
            m_eErrorType = ET_InsufficientDiskSpace;
        }

        return PFT_Error;
    }

    // We list the entire archive after adding files to ensure entry
    // properties are up-to-date.


    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    m_workStatus = WT_Move;
    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    m_workStatus = WT_Copy;
    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::deleteFiles(const QList<FileEntry> &files)
{
    // 初始化变量
    setPassword(QString());
    m_workStatus = WT_Delete;
    int errcode = 0;
    zip_error_t err;

    // 打开压缩包
    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), 0, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (nullptr == archive) {
        // 打开压缩包失败
        emit error(("Failed to open the archive: %1"));
        m_eErrorType = ET_FileOpenError;
        return PFT_Error;
    }

    m_curFileCount = 0;
    m_pCurArchive = archive; // 置空，防止进度处理
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this); // 进度回调
    zip_register_cancel_callback_with_state(archive, cancelCallback, nullptr, this);        // 取消回调

    m_listCurIndex.clear();
    getIndexBySelEntry(files);    // 获取索引值

    // 循环调用删除操作
    for (int i = 0; i < m_listCurIndex.count(); i++) {
        deleteEntry(m_listCurIndex[i], archive/*, i, count*/);        //delete from archive
    }

    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::renameFiles(const QList<FileEntry> &files)
{
    // 初始化变量
    setPassword(QString());
    m_workStatus = WT_Rename;
    int errcode = 0;
    zip_error_t err;

    // 打开压缩包
    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), 0, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (nullptr == archive) {
        // 打开压缩包失败
        emit error(("Failed to open the archive: %1"));
        m_eErrorType = ET_FileOpenError;
        return PFT_Error;
    }

    m_curFileCount = 0;
    m_pCurArchive = archive; // 置空，防止进度处理
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this); // 进度回调
    zip_register_cancel_callback_with_state(archive, cancelCallback, nullptr, this);        // 取消回调

    m_listCurIndex.clear();
    getIndexBySelEntry(files, true);    // 获取索引值，只针对单个文件或文件夹

    // 循环调用重命名操作,只针对单个文件或文件夹
    for (int i = 0; i < m_listCurIndex.count(); i++) {
        QString strAlias;
        if (!files[0].strFullPath.endsWith(QDir::separator())) { //文件重命名
            QString strPath = QFileInfo(files[0].strFullPath).path();
            if(strPath == "." || strPath.isEmpty() || strPath.isNull()) {
                strAlias = files[0].strAlias;
            } else {
                strAlias = strPath + QDir::separator() + files[0].strAlias;
            }
        } else { //文件夹重命名
            QString strPath = QFileInfo(files[0].strFullPath.left(files[0].strFullPath.length() - 1)).path();
            if(strPath == "."){
                strAlias = files[0].strAlias + QDir::separator();
            } else {
                strAlias = strPath + QDir::separator() + files[0].strAlias + QDir::separator();
            }
            strAlias = strAlias + m_listCurName[i].right(m_listCurName[i].length()-files[0].strFullPath.length());
        }
        renameEntry(m_listCurIndex[i], archive,  strAlias);        //rename from archive
    }
    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }
    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::addComment(const QString &comment)
{
    setPassword(QString());
    m_workStatus = WT_Comment;

    // 初始化变量
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_CREATE, &errcode); //filename()压缩包名
    zip_error_init_with_code(&err, errcode);
    if (!archive) {
        return PFT_Error;
    }

    // 注释字符串转换
    QByteArray tmp = comment.toUtf8();
    const char *commentstr = tmp.constData();
//    const char *commentstr13 = comment.toUtf8().constData(); // 该写法不安全，会返回空字符串
    zip_uint16_t commentlength = static_cast<zip_uint16_t>(strlen(commentstr));     // 获取注释长度

    /**
      * 设置压缩包注释
      * 如果注释为空，原注释会被移除掉
      * 注释编码必须为ASCII或者UTF-8
      */
    errcode = zip_set_archive_comment(archive, commentstr, commentlength);

    // 结果判断
    if (ZIP_ER_OK != errcode) {
        return PFT_Error;
    }

    // 注册进度回调
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);

    // 关闭保存
    if (zip_close(archive)) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    return PFT_Nomral;
}

PluginFinishType LibzipPlugin::updateArchiveData(const UpdateOptions &/*options*/)
{
    m_mapFileCode.clear();
    m_setHasHandlesDirs.clear();
    m_setHasRootDirs.clear();
    DataManager::get_instance().resetArchiveData();

    // 处理加载流程
    int errcode = 0;
    zip_error_t err;

    zip_t *archive = zip_open(QFile::encodeName(m_strArchiveName).constData(), ZIP_RDONLY, &errcode);   // 打开压缩包文件
    zip_error_init_with_code(&err, errcode);

    //某些特殊文件，如.crx用zip打不开，需要替换minizip
    if (!archive) {
//        m_bAllEntry = true;
//        return minizip_list();
    }

    // 获取文件压缩包文件数目
    const auto nofEntries = zip_get_num_entries(archive, 0);

    // 循环构建数据
    for (zip_int64_t i = 0; i < nofEntries; i++) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        handleArchiveData(archive, i);  // 构建数据
    }

    zip_close(archive);

    return PFT_Nomral;
}

void LibzipPlugin::pauseOperation()
{
    m_bPause = true;
}

void LibzipPlugin::continueOperation()
{
    m_bPause = false;
}

bool LibzipPlugin::doKill()
{
    m_bPause = false;
    m_bCancel = true;
    return false;
}

bool LibzipPlugin::writeEntry(zip_t *archive, const QString &entry, const CompressOptions &options, bool isDir, const QString &strRoot)
{
    Q_ASSERT(archive);
    QFileInfo info(entry);
    QString str;
    if (!options.strDestination.isEmpty()) {
        str = QString(options.strDestination + entry.mid(strRoot.length()));
    } else {
        //移除前缀路径
        str = entry.mid(strRoot.length());
    }
    zip_int64_t index;
    if (isDir && !info.isSymLink()) {
        index = zip_dir_add(archive, str.toUtf8().constData(), ZIP_FL_ENC_GUESS);
        if (-1 == index) {
            // If directory already exists in archive, we get an error.
            return false;
        }
    } else if(info.isSymLink()) { // symlink
        QByteArray strLinkArr = info.symLinkTarget().toUtf8();
        struct zip_source *source = zip_source_buffer(archive , strLinkArr, strLinkArr.length(), 0);
        if (source)
        {
            index = zip_add(archive, str.toUtf8().constData(), source);
        }
        else
        {
            zip_source_free(source);
            emit error(("Failed to add entry: %1"));
            return false;
        }
        zip_source_commit_write(source);
        zip_source_close(source);
     } else {
        // 获取源文件
        zip_source_t *src = zip_source_file(archive, QFile::encodeName(entry).constData(), 0, -1);
        if (!src) {
            emit error(("Failed to add entry: %1"));
            return false;
        }

        // 向压缩包中添加文件
        index = zip_file_add(archive, str.toUtf8().constData(), src, ZIP_FL_ENC_GUESS | ZIP_FL_OVERWRITE);
        if (-1 == index) {
            zip_source_free(src);
            emit error(("Failed to add entry: %1"));
            return false;
        }
        zip_source_commit_write(src);
        zip_source_close(src);
    }
    zip_uint64_t uindex = static_cast<zip_uint64_t>(index);
#ifndef Q_OS_WIN
    // 设置文件权限
    QT_STATBUF result;
    if (QT_LSTAT(QFile::encodeName(entry).constData(), &result) != 0) {
    } else {
        zip_uint32_t attributes = result.st_mode << 16;
        if (zip_file_set_external_attributes(archive, uindex, 0, ZIP_OPSYS_UNIX, attributes) != 0) {
        }
    }
#endif

    // 设置压缩的加密算法
    if (options.bEncryption && !options.strEncryptionMethod.isEmpty()) { //ReadOnlyArchiveInterface::password()
        int ret = 0;
        if (QLatin1String("AES128") == options.strEncryptionMethod) {
            ret = zip_file_set_encryption(archive, uindex, ZIP_EM_AES_128, options.strPassword.toUtf8().constData());
        } else if (QLatin1String("AES192") == options.strEncryptionMethod) {
            ret = zip_file_set_encryption(archive, uindex, ZIP_EM_AES_192, options.strPassword.toUtf8().constData());
        } else if (QLatin1String("AES256") == options.strEncryptionMethod) {
            ret = zip_file_set_encryption(archive, uindex, ZIP_EM_AES_256, options.strPassword.toUtf8().constData());
        }
        if (ret != 0) {
            emit error(("Failed to set compression options for entry: %1"));
            return false;
        }
    }

    // 设置压缩算法
    zip_int32_t compMethod = ZIP_CM_DEFAULT;
    if (!options.strCompressionMethod.isEmpty()) {
        if (QLatin1String("Deflate") == options.strCompressionMethod) {
            compMethod = ZIP_CM_DEFLATE;
        } else if (QLatin1String("BZip2") == options.strCompressionMethod) {
            compMethod = ZIP_CM_BZIP2;
        } else if (QLatin1String("Store") == options.strCompressionMethod) {
            compMethod = ZIP_CM_STORE;
        }
    }

    // 设置压缩等级
    const int compLevel = (options.iCompressionLevel != -1) ? options.iCompressionLevel : 6;
    if (zip_set_file_compression(archive, uindex, compMethod, zip_uint32_t(compLevel)) != 0) {
        emit error(("Failed to set compression options for entry: %1"));
        return false;
    }
    return true;
}


void LibzipPlugin::progressCallback(zip_t *, double progress, void *that)
{
    static_cast<LibzipPlugin *>(that)->emitProgress(progress);      // 进度回调
}


int LibzipPlugin::cancelCallback(zip_t *, void *that)
{
    return static_cast<LibzipPlugin *>(that)->cancelResult();       // 取消回调
}

bool LibzipPlugin::handleArchiveData(zip_t *archive, zip_int64_t index)
{
    if (nullptr == archive) {
        return false;
    }

    zip_stat_t statBuffer;
    if (zip_stat_index(archive, zip_uint64_t(index), ZIP_FL_ENC_RAW, &statBuffer)) {
        return false;
    }

    QByteArray strCode;
    // 对文件名进行编码探测并转码
    QString name = m_common->trans2uft8(statBuffer.name, strCode);
    m_mapFileCode[index] = strCode;
    if(name.contains(QLatin1Char('\\')))
        name = name.replace(QLatin1Char('\\'), QDir::separator());
    //对于名称以"/"开头的去除开始的"/"
    if(name.startsWith(QDir::separator())) {
        name.remove(0,1);
    }
    FileEntry entry;
    entry.iIndex = int(index);
    entry.strFullPath = name;
    statBuffer2FileEntry(statBuffer, entry);

    handleEntry(entry);

    // 存储总数据
    DataManager::get_instance().archiveData().mapFileEntry[name] = entry;

    return true;
}

void LibzipPlugin::statBuffer2FileEntry(const zip_stat_t &statBuffer, FileEntry &entry)
{
    // FileEntry stFileEntry;

    // 文件名
    if (statBuffer.valid & ZIP_STAT_NAME) {
        const QStringList pieces = entry.strFullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
        entry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
    }

    // 是否为文件夹
    if (entry.strFullPath.endsWith(QDir::separator())) {
        entry.isDirectory = true;
    }

    // 文件真实大小（文件夹显示项）
    if (statBuffer.valid & ZIP_STAT_SIZE) {
        if (!entry.isDirectory) {
            entry.qSize = qlonglong(statBuffer.size);
            DataManager::get_instance().archiveData().qSize += statBuffer.size;
            DataManager::get_instance().archiveData().qComressSize += statBuffer.comp_size;
        } else {
            entry.qSize = 0;
        }
    }

    // 文件最后修改时间
    if (statBuffer.valid & ZIP_STAT_MTIME) {
        entry.uLastModifiedTime = uint(statBuffer.mtime);
    }

    DataManager::get_instance().archiveData().strComment = m_strComment;
}

ErrorType LibzipPlugin::extractEntry(zip_t *archive, zip_int64_t index, const ExtractionOptions &options, qlonglong &qExtractSize, QString &strFileName, bool &bHandleLongName)
{
    zip_stat_t statBuffer;
    if (zip_stat_index(archive, zip_uint64_t(index), ZIP_FL_ENC_RAW, &statBuffer) != 0) {
        return ET_FileReadError;
    }

    strFileName = m_common->trans2uft8(statBuffer.name, m_mapFileCode[index]);    // 解压文件名（压缩包中）
    //fix 232873
    if(strFileName.indexOf("../") != -1) {
        qInfo() << "skipped ../ path component(s) in " << strFileName;
        strFileName = strFileName.replace("../", "");
    }
    if(strFileName.contains(QLatin1Char('\\')))
        strFileName = strFileName.replace(QLatin1Char('\\'), QDir::separator());
    QString strOriginName = strFileName;

    // 针对文件夹名称过长的情况，直接提示解压失败，文件夹名称过长
//    QStringList listPath = strFileName.split(QDir::separator());
//    listPath.removeLast();
//    for (int i = 0; i < listPath.count(); ++i) {
//        if (NAME_MAX < QString(listPath[i]).toLocal8Bit().length()) {
//            emit signalCurFileName(strFileName); // 发送当前正在解压的文件名
//            return ET_LongNameError;
//        }
//    }

    QString strFilePath;
    QString strTempFileName = strFileName;
    int iIndex = strFileName.lastIndexOf(QDir::separator());

    if (iIndex >= 0) {
        strFilePath = strFileName.left(iIndex);   // bug114527 left函数参数为截取的字符串长度
        strTempFileName = strFileName.right(strFileName.length() - iIndex - 1);
    }

    QString tempFilePathName;
    if(!m_bDlnfs) {
        QString sDir = m_common->handleLongNameforPath(strFilePath, strFileName, m_mapLongDirName, m_mapRealDirValue);
        if(sDir.length() > 0) {
           strFilePath = sDir.endsWith(QDir::separator())?sDir.left(sDir.length() -1):sDir;
           if(strFileName.endsWith(QDir::separator())) {
               strFileName = sDir;
           } else if (NAME_MAX >= QString(strTempFileName).toLocal8Bit().length()) {
               strFileName = sDir + strTempFileName;
           }
           if(!m_mapLongDirName.isEmpty()) {
               bHandleLongName = true;
           }
        }
        if (NAME_MAX < QString(strTempFileName).toLocal8Bit().length() && !strTempFileName.endsWith(QDir::separator())) {
            QString strTemp = strTempFileName.left(TRUNCATION_FILE_LONG);

            // 保存文件路径，不同目录下的同名文件分开计数,文件解压结束后才添加计数，
            tempFilePathName = strFilePath + QDir::separator() + strTemp;   // 路径加截取后的文件名
            if (m_mapLongName[tempFilePathName] >= LONGFILE_SAME_FILES ) {
                return ET_LongNameError;
            }
            bHandleLongName = true;
            QString sSuffix = QFileInfo(strTempFileName).completeSuffix();
            if(10 < sSuffix.length()){
                sSuffix = QFileInfo(strTempFileName).suffix();
                if(10 < sSuffix.length()) {
                    sSuffix = sSuffix.right(10);
                }
            }
            strTempFileName = strTemp + QString("(%1)").arg(m_mapLongName[tempFilePathName] + 1, LONGFILE_SUFFIX_FieldWidth, BINARY_NUM, QChar('0')) + "." + sSuffix;

            strFileName = strTempFileName;
            if (iIndex >= 0) {
                strFileName = strFilePath + QDir::separator() + strTempFileName;
            }

        }
    }

    // 提取
    if (!options.strDestination.isEmpty()) {
        if(bHandleLongName) {
            int nCnt = options.strDestination.count(QDir::separator());
            int nIndex = 0;
            for(int i = 0; i < nCnt; i++){
                nIndex = strFileName.indexOf(QDir::separator(), nIndex);
                nIndex++;
            }
            strFileName = strFileName.remove(0, nIndex);
        } else {
            strFileName = strFileName.remove(0, options.strDestination.size());
        }
    }


    if (bHandleLongName) {
        emit signalCurFileName(strOriginName); // 发送当前正在解压的文件名
    } else {
        emit signalCurFileName(strFileName); // 发送当前正在解压的文件名
    }

    bool bIsDirectory = strFileName.endsWith(QDir::separator());    // 是否为文件夹

    // 判断解压路径是否存在，不存在则创建文件夹
    if (QDir().exists(options.strTargetPath) == false)
        QDir().mkpath(options.strTargetPath);

    // 解压完整文件名（含路径）
    QString strDestFileName = options.strTargetPath + QDir::separator() + strFileName;

    QFile file(strDestFileName);

    // Store parent mtime.
    QString parentDir;
    if (strFileName.endsWith(QDir::separator())) {
        QDir pDir = QFileInfo(strDestFileName).dir();
        pDir.cdUp();
        parentDir = pDir.path();
    } else {
        parentDir = QFileInfo(strDestFileName).path();
    }
    // For top-level items, don't restore parent dir mtime.
    const bool restoreParentMtime = (parentDir != options.strTargetPath);

    time_t parent_mtime;
    if (restoreParentMtime) {
        parent_mtime = QFileInfo(parentDir).lastModified().toMSecsSinceEpoch() / 1000;
    }

    // 获取外部信息（权限）
    zip_uint8_t opsys;
    zip_uint32_t attributes;
    if (zip_file_get_external_attributes(archive, zip_uint64_t(index), ZIP_FL_UNCHANGED, &opsys, &attributes) == -1) {
        emit error(("Failed to read metadata for entry: %1"));
    }

    // 从压缩包中获取文件权限
    mode_t value = mode_t();
    switch (opsys) {
    case ZIP_OPSYS_UNIX:
        value = attributes >> 16;
        break;
    default:    // TODO: non-UNIX.
        break;
    }
    bool isLink = false;
    if(S_ISLNK(value)) {
        isLink = true;
    }
    QFileDevice::Permissions per = getPermissions(value);

    if (bIsDirectory && !isLink) {     // 文件夹
        if (PATH_MAX < QString(strFileName).toLocal8Bit().length())
            return ET_LongNameError;

        QDir dir;
        dir.mkpath(strDestFileName);

        // 文件夹加可执行权限
        per = per | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser ;
    } else if(isLink) { //软连接解压
        zip_file_t *zipFile = zip_fopen_index(archive, zip_uint64_t(index), 0);
        char buf[READBYTES] = {0};
        const auto readBytes = zip_fread(zipFile, buf, zip_uint64_t(READBYTES));
        if (readBytes > 0) {
            QString strBuf = QString(buf).toLocal8Bit();
            if (QFile::link(strBuf, strDestFileName)) {
                qInfo() << "Symlink's created:" << buf << strFileName;
            } else {
                qInfo() << "Can't create symlink" << buf << strFileName;
            }
        }
        file.close();
        zip_fclose(zipFile);
    } else {        // 普通文件

        // 判断是否有同名文件
        if (file.exists()) {
            if (m_bSkipAll) {       // 全部跳过
                m_mapLongName[tempFilePathName]++;   // 保存文件路径，不同目录下的同名文件分开计数，解压成功，添加计数
                return ET_NoError;
            } else {
                if (!m_bOverwriteAll) {     // 若不是全部覆盖，单条处理

                    OverwriteQuery query(strDestFileName);

                    emit signalQuery(&query);
                    query.waitForResponse();

                    if (query.responseCancelled()) {
                        emit signalCancel();
                        return ET_UserCancelOpertion;
                    } else if (query.responseSkip()) {
                        m_mapLongName[tempFilePathName]++;   // 保存文件路径，不同目录下的同名文件分开计数，解压成功，添加计数
                        return ET_NoError;
                    } else if (query.responseSkipAll()) {
                        m_bSkipAll = true;
                        m_mapLongName[tempFilePathName]++;   // 保存文件路径，不同目录下的同名文件分开计数，解压成功，添加计数
                        return ET_NoError;
                    }  else if (query.responseOverwriteAll()) {
                        m_bOverwriteAll = true;
                    }
                }
            }
        }

        if (bHandleLongName) {
            m_setLongName << strFileName;
        }

        // 若文件存在且不是可写权限，重新创建一个文件
        if (file.exists() && !file.isWritable()) {
            file.remove();
            file.setFileName(strDestFileName);
            file.setPermissions(QFileDevice::WriteUser);
        }

        // 对文件路径做判断，防止特殊包未先解压出文件夹，导致解压失败
        if (QDir().exists(QFileInfo(strDestFileName).path()) == false)
            QDir().mkpath(QFileInfo(strDestFileName).path());

        zip_file_t *zipFile = zip_fopen_index(archive, zip_uint64_t(index), 0);
        // 错误处理
        if (nullptr == zipFile) {
            int iErr = zip_error_code_zip(zip_get_error(archive));
            if (ZIP_ER_WRONGPASSWD == iErr) {//密码错误

                // 对密码编码的探测
                bool bCheckFinished = false;
                int iCodecIndex = 0;
                while (nullptr == zipFile && false == bCheckFinished) {
                    if (m_listCodecs.length() == iCodecIndex) {
                        bCheckFinished = true;
                        if (file.exists()) {
                            file.remove();
                        }

                        return ET_WrongPassword;
                    } else {
                        // 115645 【专业版】【1060】【归档管理器】【5.12.0.2】无法解压中文密码的zip压缩包（含有长名称）
                        zip_set_default_password(archive, passwordUnicode(m_strPassword, iCodecIndex).data());
                        iCodecIndex++;
                        zip_error_clear(archive);
                        zipFile = zip_fopen_index(archive, zip_uint64_t(index), 0);
                        iErr = zip_error_code_zip(zip_get_error(archive));
                        if (iErr != ZIP_ER_WRONGPASSWD && zipFile != nullptr) {//密码正确
                            bCheckFinished = true;
                        }
                    }
                }
            } else if (ZIP_ER_NOPASSWD == iErr) {   // 无密码输入
                return ET_NeedPassword;
            } else {
                return ET_FileOpenError;
            }
        }

        // 以只写的方式打开待解压的文件
        if (file.open(QIODevice::WriteOnly) == false) {
            zip_fclose(zipFile);

            QList<QString> entryNameList = strDestFileName.split('/');
            foreach (auto &tmp, entryNameList) {
                // 判断文件名是否过长
                if (NAME_MAX < tmp.toLocal8Bit().length()) {
                    return ET_LongNameError;
                }
            }

            emit signalFileWriteErrorName(QFileInfo(file.fileName()).fileName());
            return ET_FileWriteError;
        }

        // 写文件
        QDataStream out(&file);
        zip_int64_t sum = 0;
        char buf[READBYTES];
        int writeSize = 0;
        double dScale = 1;
        // 右键解压时按照文件比例计算大小
        if (!options.bExistList) {
            dScale = double(statBuffer.comp_size) / statBuffer.size;
        }
        while (sum != zip_int64_t(statBuffer.size)) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            if (m_bPause) { //解压暂停
                sleep(1);
                continue;
            }

            const auto readBytes = zip_fread(zipFile, buf, zip_uint64_t(READBYTES));

            if (readBytes < 0) {
                file.close();
                zip_fclose(zipFile);
                emit signalFileWriteErrorName(QFileInfo(file.fileName()).fileName());
                return ET_FileWriteError;
            }

            if (out.writeRawData(buf, int(readBytes)) != readBytes) {
                file.close();
                zip_fclose(zipFile);

                if (isInsufficientDiskSpace(options.strTargetPath, static_cast<qint64>(readBytes))) {  // 小于readBytes作为磁盘空间不足的判断标准
                    return ET_InsufficientDiskSpace;
                } else {
                    emit signalFileWriteErrorName(QFileInfo(file.fileName()).fileName());
                    return ET_FileWriteError;
                }
            }

            sum += readBytes;
            writeSize += readBytes;

            // 计算进度并显示（右键快捷解压使用压缩包大小，计算比例）
            qExtractSize += readBytes * dScale;
            emit signalprogress((double(qExtractSize)) * m_dScaleSize);
        }

        file.close();
        zip_fclose(zipFile);
    }
    if(!isLink) {
        // 设置文件/文件夹权限
        file.setPermissions(per);

        // Set mtime for entry.
        utimbuf times;
        times.modtime = statBuffer.mtime;
        utime(strDestFileName.toUtf8().constData(), &times);

        if (restoreParentMtime) {
            // Restore mtime for parent dir.
            times.modtime = parent_mtime;
            utime(parentDir.toUtf8().constData(), &times);
        }
    }
    m_mapLongName[tempFilePathName]++;   // 保存文件路径，不同目录下的同名文件分开计数，解压成功，添加计数
    return ET_NoError;
}

void LibzipPlugin::emitProgress(double dPercentage)
{
    bool flag = true;
    while (flag) {
        if (QThread::currentThread()->isInterruptionRequested()) { //线程结束
            break;
        }

        // 暂停
        if (m_bPause) {
            sleep(1);
            continue;
        }

        // 处理当前文件名
        if (m_pCurArchive) {
            if (WT_Add == m_workStatus) {
                // 压缩操作显示当前正在压缩的文件名
                zip_uint64_t index = zip_uint64_t(m_curFileCount * dPercentage);
                // 发送当前文件名信号
                emit signalCurFileName(m_common->trans2uft8(zip_get_name(m_pCurArchive, index, ZIP_FL_ENC_RAW), m_mapFileCode[zip_int64_t(index)]));
            } else if (WT_Delete == m_workStatus) {
                // 删除操作显示当前正在删除的文件名
                int iSpan = qRound(m_listCurName.count() * dPercentage);    // 获取占比
                QString strCurFileName;
                // 按照进度占比处理当前文件名
                if (iSpan < 0) {
                    strCurFileName = m_listCurName[0];
                } else if (iSpan >= m_listCurIndex.count()) {
                    strCurFileName = m_listCurName[m_listCurName.count() - 1];
                } else {
                    strCurFileName = m_listCurName[iSpan];
                }

                // 发送当前文件名信号
                emit signalCurFileName(strCurFileName);
            }

        }

        // 发送进度信号
        emit signalprogress(dPercentage * 100);

        flag = false;
    }

    m_bPause = false;
}

int LibzipPlugin::cancelResult()
{
    if (m_bCancel) {
        m_bCancel = false;
        return 1;
    } else {
        return 0;
    }
}

QByteArray LibzipPlugin::passwordUnicode(const QString &strPassword, int iIndex)
{
    if (m_strArchiveName.endsWith(".zip")) {
        // QStringList listCodecName = QStringList() << "UTF-8" << "GB18030" << "GBK" <<"Big5"<< "us-ascii";
        int nCount = strPassword.count();
        bool b = false;

        // 检测密码是否含有中文
        for (int i = 0 ; i < nCount ; i++) {
            QChar cha = strPassword.at(i);
            ushort uni = cha.unicode();
            if (uni >= 0x4E00 && uni <= 0x9FA5) {   // 判断是否是中文
                b = true;
                break;
            }
        }

        // chinese
        if (b) {
            QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
            QTextCodec *gbk = QTextCodec::codecForName(m_listCodecs[iIndex].toUtf8().data());
            // QTextCodec *gbk = QTextCodec::codecForName("UTF-8");

            //utf8 -> 所需编码
            //1. utf8 -> unicode
            QString strUnicode = utf8->toUnicode(strPassword.toUtf8().data());
            //2. unicode -> 所需编码, 得到QByteArray
            QByteArray gb_bytes = gbk->fromUnicode(strUnicode);
            return gb_bytes; //获取其char *115645 【专业版】【1060】【归档管理器】【5.12.0.2】无法解压中文密码的zip压缩包（含有长名称）
        } else {
            return strPassword.toUtf8();
        }
    } else {
        return strPassword.toUtf8();
    }

}

bool LibzipPlugin::deleteEntry(int index, zip_t *archive)
{
    // 事件循环
    if (QThread::currentThread()->isInterruptionRequested()) {
        if (zip_close(archive)) {
            // 发送保存失败
            emit error(("Failed to write archive."));
            m_eErrorType = ET_FileWriteError;
            return false;
        }
        return false;
    }

    int statusDel = zip_delete(archive, zip_uint64_t(index));   // 获取删除状态
    if (-1 == statusDel) {
        // 删除失败
        emit error(("Failed to delete entry: %1"));
        m_eErrorType = ET_DeleteError;
        return false;
    }

    return true;
}

bool LibzipPlugin::renameEntry(int index, zip_t *archive, const QString &strAlisa)
{
    // 事件循环
    if (QThread::currentThread()->isInterruptionRequested()) {
        if (zip_close(archive)) {
            // 发送保存失败
            emit error(("Failed to write archive."));
            m_eErrorType = ET_FileWriteError;
            return false;
        }
        return false;
    }

    int statusDel = zip_rename(archive, zip_uint64_t(index), strAlisa.toUtf8().data());   // 获取重命名状态
    if (-1 == statusDel) {
        // 重命名失败
        emit error(("Failed to rename entry: %1"));
        m_eErrorType = ET_RenameError;
        return false;
    }

    return true;
}

void LibzipPlugin::getIndexBySelEntry(const QList<FileEntry> &listEntry, bool isRename)
{
    m_listCurIndex.clear();
    m_listCurName.clear();
    ArchiveData stArchiveData = DataManager::get_instance().archiveData();

    // 筛选待提取文件/文件夹索引
    for (FileEntry entry : listEntry) {
        auto iter = stArchiveData.mapFileEntry.find(entry.strFullPath);
        for (; iter != stArchiveData.mapFileEntry.end();) {
            if (!iter.key().startsWith(entry.strFullPath)) {
                break;
            } else {
                // 获取有效索引
                if (iter.value().iIndex >= 0) {
                    m_listCurIndex << iter.value().iIndex;      // 保存文件索引
                    m_listCurName << iter.value().strFullPath;  // 保存文件名
                }

                ++iter;

                // 如果文件夹，直接跳过
                if (!entry.strFullPath.endsWith(QDir::separator())) {
                    break;
                }
            }
        }
    }
    if(isRename) return;//重命名不做排序处理
    // 升序排序
    std::stable_sort(m_listCurIndex.begin(), m_listCurIndex.end());
}
