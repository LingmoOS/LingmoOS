/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file-copy-operation.h"
#include "file-node-reporter.h"
#include "file-node.h"
#include "file-enumerator.h"
#include "file-info.h"
#include "file-utils.h"
#include "file-label-model.h"
#include "file-operation-manager.h"
#include "sound-effect.h"
#include "clipboard-utils.h"
#include <QProcess>
#include <QStorageInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include "file-copy.h"
#include <gio/gdesktopappinfo.h>

using namespace Peony;

static void handleDuplicate(FileNode *node)
{
    node->setDestFileName(FileUtils::handleDuplicateName(node->destBaseName()));
}

FileCopyOperation::FileCopyOperation(QStringList sourceUris, QString destDirUri, QObject *parent) : FileOperation (parent)
{
    for (auto u : sourceUris) {
        if (u.split ("://").length () != 2) {
            sourceUris.removeOne (u);
        }
    }

    QUrl destDirUrl = Peony::FileUtils::urlEncode(destDirUri);
    QUrl firstSrcUrl = Peony::FileUtils::urlEncode(sourceUris.first());
    if("label" == firstSrcUrl.scheme())
    {
        QString scheme = firstSrcUrl.path().section("?schema=",-1,-1);
        QString path = firstSrcUrl.path().section("?schema=", 0, 0).section("/",2,-1);
        firstSrcUrl = QString(scheme).append(":///").append(path);
    }

    if (destDirUrl.isParentOf(firstSrcUrl)) {
        if (1 == firstSrcUrl.path().split("/").count() - destDirUrl.path().split("/").count()) {
            m_is_duplicated_copy = true;
        }
    }/* else {
        // fix #83068
        // windows里的重复复制操作没有备份选项，但是会一直弹框提示，这里和windows的行为靠拢
        auto lastPasteDirectoryUri = ClipboardUtils::getInstance()->getLastTargetDirectoryUri();
        QUrl lastPasteDirectoryUrl = Peony::FileUtils::urlEncode(lastPasteDirectoryUri);
        if (destDirUrl == lastPasteDirectoryUrl) {
            m_is_duplicated_copy = true;
        }
    }*/

    QStringList srcUris;
    for(auto &uri : sourceUris){
        if(uri.startsWith("label://"))
        {
            QUrl url(uri);
            QString scheme = url.path().section("?schema=",-1,-1);
            QString path = url.path().section("?schema=", 0, 0).section("/",2,-1);
            uri = QString(scheme).append(":///").append(path);
        }
        srcUris.append(uri);
    }

    m_conflict_files.clear();
    m_source_uris = srcUris;
    m_dest_dir_uri = FileUtils::urlDecode(destDirUri);
    m_reporter = new FileNodeReporter;
    connect(m_reporter, &FileNodeReporter::nodeFound, this, &FileOperation::operationPreparedOne);
    m_info = std::make_shared<FileOperationInfo>(srcUris, destDirUri, FileOperationInfo::Copy);
}

FileCopyOperation::~FileCopyOperation()
{
    delete m_reporter;
    m_conflict_files.clear();
}

ExceptionResponse FileCopyOperation::prehandle(GError *err)
{
//    SoundEffect::getInstance()->copyOrMoveFailedMusic();

    switch (err->code) {
        case G_IO_ERROR_BUSY:
        case G_IO_ERROR_PENDING:
        case G_IO_ERROR_NO_SPACE:
        case G_IO_ERROR_CANCELLED:
        case G_IO_ERROR_INVALID_DATA:
        case G_IO_ERROR_NOT_SUPPORTED:
        case G_IO_ERROR_PERMISSION_DENIED:
        case G_IO_ERROR_CANT_CREATE_BACKUP:
        case G_IO_ERROR_TOO_MANY_OPEN_FILES:
            return Other;
    }

    /**
     * @note FIX BUG 79363
     * 文件传输中拔掉U盘，第一次报错:G_IO_ERROR_FAILED，后续报错:G_IO_ERROR_NOT_FOUND
     */
    if (G_IO_ERROR_NOT_FOUND == err->code && m_prehandle_hash.contains(G_IO_ERROR_FAILED)) {
        return IgnoreAll;
    }

    if (G_IO_ERROR_EXISTS == err->code && m_is_duplicated_copy) {
        return BackupAll;
    }

    if (m_prehandle_hash.contains(err->code))
        return m_prehandle_hash.value(err->code);

    return Other;
}

void FileCopyOperation::progress_callback(goffset current_num_bytes,
        goffset total_num_bytes,
        FileCopyOperation *p_this)
{
    if (total_num_bytes < current_num_bytes)
        return;

    QUrl url(Peony::FileUtils::urlEncode(p_this->m_current_src_uri));
    auto currnet = p_this->m_current_offset + current_num_bytes;
    auto total = p_this->m_total_size;
    auto fileIconName = FileUtilsPrivate::getFileIconName(p_this->m_current_src_uri);
    auto destFileName = FileUtils::isFileDirectory(p_this->m_current_dest_dir_uri) ?
                p_this->m_current_dest_dir_uri + "/" + url.fileName() : p_this->m_current_dest_dir_uri;
    Q_EMIT p_this->FileProgressCallback(p_this->m_current_src_uri, destFileName, fileIconName, currnet, total);
}

void FileCopyOperation::copyRecursively(FileNode *node)
{
    if (isCancelled())
        return;

    node->setState(FileNode::Handling);
    QString destName = "";
    QString destDir = m_dest_dir_uri;

fallback_retry:
    QString destFileUri = node->resolveDestFileUri(m_dest_dir_uri);
    //QUrl destFileUrl = Peony::FileUtils::urlEncode(destFileUri);
    destFileUri = FileUtils::urlEncode(destFileUri);
    node->setDestUri(destFileUri);
    QString srcUri = node->uri();

    GFileWrapperPtr destFile = wrapGFile(g_file_new_for_uri(destFileUri.toUtf8().constData()));
    GFileWrapperPtr srcFile = wrapGFile(g_file_new_for_uri(srcUri.toUtf8().constData()));

    m_current_src_uri = node->uri();
    m_current_dest_dir_uri = destFileUri;
    g_autoptr(GFileInfo) srcFileInfo = nullptr;
    srcFileInfo = g_file_query_info(srcFile.get()->get()
                                    , G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET
                                    , G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
    if (G_FILE_TYPE_SYMBOLIC_LINK == g_file_info_get_file_type(srcFileInfo)) {
        if (!copyLinkedFile(node, srcFileInfo, destFile)) {
            goto fallback_retry;
        } else {
            return;
        }
    }

    //fix bug#163573, can not copy readonly folder issue
    QString srcParent;
    srcParent = FileUtils::getParentUri(node->uri());
    if (m_last_src_parent != srcParent || m_parent_flags == GFileCopyFlags(G_FILE_COPY_NONE)) {
        GFile *source_dir;
        gboolean readonly_source_fs = FALSE;
        source_dir = g_file_new_for_uri(FileUtils::urlEncode(srcParent).toUtf8());
        /* Query the source dir, not the file because if its a symlink we'll follow it */
        qDebug() << "node->uri():"<<node->uri()<<"srcParent:"<<QUrl(srcParent).url();
        if (source_dir) {
            GFileInfo *inf;
            inf = g_file_query_filesystem_info (source_dir, "filesystem::readonly", NULL, NULL);
            if (inf != NULL) {
                readonly_source_fs = g_file_info_get_attribute_boolean (inf, "filesystem::readonly");
                g_object_unref (inf);
            }
            g_object_unref (source_dir);
        }
        auto flags = (readonly_source_fs) ? G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_TARGET_DEFAULT_PERMS
                         : G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA;
        m_parent_flags = GFileCopyFlags(flags);
        m_last_src_parent = srcParent;
    }

    if (node->isFolder()) {
        GError *err = nullptr;
        GError *error = nullptr;

        //NOTE: mkdir doesn't have a progress callback.
        g_file_make_directory(destFile.get()->get(),
                              getCancellable().get()->get(),
                              &err);
        if (err) {
            FileOperationError except;
            if (err->code == G_IO_ERROR_CANCELLED) {
                return;
            }
            auto errWrapperPtr = GErrorWrapper::wrapFrom(err);
            int handle_type = prehandle(err);
            except.errorType = ET_GIO;
            except.errorStr = tr("Create folder %1 failed: %2").arg(node->destBaseName()).arg(err->message);
            except.srcUri = m_current_src_uri;
            except.destDirUri = m_current_dest_dir_uri;
            except.op = FileOpCopy;
            except.title = tr("File copy error");
            except.errorCode = err->code;
            if (handle_type == Other) {
                switch (err->code) {
                case G_IO_ERROR_EXISTS: {
                    except.dlgType = ED_CONFLICT;
                    Q_EMIT errored(except);
                    auto typeData = except.respCode;
                    handle_type = typeData;
                    break;
                }
                case G_IO_ERROR_FILENAME_TOO_LONG: {
                    if (node->destBaseName().length() > 255 &&
                        m_dest_dir_uri.startsWith(QString("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展"))) {
                        QString msg = tr("The file name exceeds the limit");
                        Q_EMIT operationInfoMsgBox(msg);
                        if (m_is_long_name_file_operation) {
                            m_is_long_name_file_operation = false;
                        }
                        return;
                    }
                    except.dlgType = ED_RENAME;
                    Q_EMIT errored(except);
                    auto typeData = except.respCode;
                    handle_type = typeData;
                    break;
                }
                default: {
                    except.dlgType = ED_WARNING;
                    Q_EMIT errored(except);
                    auto typeData = except.respCode;
                    handle_type = typeData;
                    if (handle_type != Cancel) {
                        return;
                    }
                    break;
                }
                }
            } else if (handle_type == SaveOne || handle_type == SaveAll) {
                if (node->destBaseName().length() > 255 && !m_is_long_name_error_msg_show &&
                    m_dest_dir_uri.startsWith(QString("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展"))) {
                    QString msg = tr("The file name exceeds the limit");
                    Q_EMIT operationInfoMsgBox(msg);
                    if (m_is_long_name_file_operation) {
                        m_is_long_name_file_operation = false;
                    }
                    m_is_long_name_error_msg_show = true;
                    return;
                }
            }
            //handle.
            node->setState(FileNode::Handling);
            switch (handle_type) {
            case IgnoreOne: {
                setHasError(true);
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    return;
                }
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case IgnoreAll: {
                setHasError(true);
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                m_prehandle_hash.insert(err->code, IgnoreOne);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    return;
                }
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case OverWriteOne: {
                setHasError(true);
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    auto result = udfCopyWarningDialog();
                    if (Cancel == result) {
                        cancel();
                    } else if (IgnoreOne == result || IgnoreAll == result) {
                        return;
                    }
                }
//                g_file_copy_attributes(srcFile.get()->get(),
//                                       destFile.get()->get(),
//                                       GFileCopyFlags(flags),
//                                       nullptr,
//                                       &error);
//                if (error) {
//                    qDebug() << __func__ << error->code << error->message;
//                }
//                g_error_free(error);
                //make dir has no overwrite
                break;
            }
            case OverWriteAll: {
                setHasError(true);
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
                m_prehandle_hash.insert(err->code, OverWriteOne);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    m_is_udf_warning = true;
                    auto result = udfCopyWarningDialog();
                    if (Cancel == result) {
                        cancel();
                    } else if (IgnoreOne == result || IgnoreAll == result) {
                        return;
                    }
                }
//                g_file_copy_attributes(srcFile.get()->get(),
//                                       destFile.get()->get(),
//                                       GFileCopyFlags(flags),
//                                       nullptr,
//                                       &error);
//                if (error) {
//                    qDebug() << __func__ << error->code << error->message;
//                }
//                g_error_free(error);
                break;
            }
            case BackupOne: {
                node->setErrorResponse(BackupOne);
                // use custom name
                QString name = "";
                QStringList extendStr = node->destBaseName().split(".");
                if (extendStr.length() > 0) {
                    extendStr.removeAt(0);
                }
                QString endStr = extendStr.join(".");
                if (except.respValue.contains("name")) {
                    name = except.respValue["name"].toString();
                    if (endStr != "" && name.endsWith(endStr)) {
                        node->setDestFileName(name);
                    } else if ("" != endStr && "" != name) {
                        node->setDestFileName(name + "." + endStr);
                    }
                }
                while (FileUtils::isFileExsit(node->resolveDestFileUri(m_dest_dir_uri))) {
                    handleDuplicate(node);
                }
//                GFileWrapperPtr destDir = wrapGFile(g_file_new_for_uri(node->destUri().toUtf8().constData()));
//                g_file_copy_attributes(srcFile.get()->get(),
//                                       destDir.get()->get(),
//                                       GFileCopyFlags(flags),
//                                       nullptr,
//                                       &error);
//                if (error) {
//                    qDebug() << __func__ << error->code << error->message;
//                }
//                g_error_free(error);
                goto fallback_retry;
            }
            case BackupAll: {
                node->setErrorResponse(BackupOne);
                while (FileUtils::isFileExsit(node->resolveDestFileUri(m_dest_dir_uri))) {
                    handleDuplicate(node);
                }
                //make dir has no backup
                m_prehandle_hash.insert(err->code, BackupOne);
                GFileWrapperPtr destDir = wrapGFile(g_file_new_for_uri(node->destUri().toUtf8().constData()));
//                g_file_copy_attributes(srcFile.get()->get(),
//                                       destDir.get()->get(),
//                                       GFileCopyFlags(flags),
//                                       nullptr,
//                                       &error);
//                if (error) {
//                    qDebug() << __func__ << error->code << error->message;
//                }
//                g_error_free(error);
                goto fallback_retry;
            }
            case Retry: {
                goto fallback_retry;
            }
            case RenameOne: {
                node->setErrorResponse(RenameOne);
                node->setDestFileName(except.respValue.value("newName").toString());
                goto fallback_retry;
            }
            case TruncateOne: {
                node->setErrorResponse(TruncateOne);
                auto respValut = except.respValue.value("cateType").toInt();
                if (m_cate_type == AllPost) {
                    respValut = Post;
                } else if (m_cate_type == ALLFront) {
                    respValut = Front;
                }
                node->truncateDestFileName(respValut);
                goto fallback_retry;
            }
            case TruncateAll: {
                node->setErrorResponse(TruncateOne);
                m_cate_type = except.respValue.value("cateType").toInt();
                node->truncateDestFileName(m_cate_type);
                if (Post == m_cate_type) {
                    m_cate_type = AllPost;
                } else if (Front == m_cate_type) {
                    m_cate_type = ALLFront;
                }
                m_prehandle_hash.insert(err->code, TruncateOne);
                goto fallback_retry;
            }
            case SaveOne: {
                node->setErrorResponse(SaveOne);
                if (!saveAsOtherPath()) {
                    break;
                }
                goto fallback_retry;
            }
            case SaveAll: {
                node->setErrorResponse(SaveOne);
                m_prehandle_hash.insert(err->code, SaveOne);
                if (!saveAsOtherPath()) {
                    break;
                }
                goto fallback_retry;
            }
            case Cancel: {
                node->setState(FileNode::Unhandled);
                cancel();
                break;
            }
            default:
                break;
            }
        } else {
            node->setState(FileNode::Handled);
            // if copy sucessed, flush all data
            g_autoptr(GFile) destFile = g_file_new_for_uri(destFileUri.toUtf8().constData());
            if (g_file_query_exists(destFile, nullptr)) {
                // copy file attribute
                // It is possible that some file systems do not support file attributes
                g_autoptr(GFile) srcFile = g_file_new_for_uri(srcUri.toUtf8().constData());
                g_file_copy_attributes(srcFile, destFile, G_FILE_COPY_ALL_METADATA, nullptr, &err);
                if (nullptr != err) {
                    qWarning() <<destFileUri<<"copy attribute error:" << err->code << "  ---  " << err->message;
                    g_error_free(err);
                    err = nullptr;
                }
                QList<int> labelIds = FileLabelModel::getGlobalModel()->getFileLabelIds(srcUri);
                for(auto &labelId: labelIds){
                    if(labelId <= 0)
                        continue;
                    FileLabelModel::getGlobalModel()->addLabelToFile(destFileUri, labelId);
                }
            }
        }

        if (SaveOne == node->responseType() || SaveAll == node->responseType()) {
            m_dest_dir_uri = destDir;
        }

        //assume that make dir finished anyway
        m_current_offset += node->size();
        Q_EMIT operationProgressedOne(node->uri(), node->destUri(), node->size());
        for (auto child : *(node->children())) {
            copyRecursively(child);
        }

        //copy folder attributes after copy child, support copy readonly files
        //related bug#163573
        if (node->state() == FileNode::Handled || node->responseType() == OverWriteOne || node->responseType() == OverWriteAll) {
            g_file_copy_attributes(srcFile.get()->get(),
                                   destFile.get()->get(),
                                   m_parent_flags,
                                   nullptr,
                                   &error);
        }

        if (error) {
            qDebug() << __func__ << error->code << error->message;
            g_error_free(error);
        }
    } else {
        GError *err = nullptr;
        QUrl url = node->uri();

        bool is_desktop_file = false;
        g_autoptr(GFile)        src = g_file_new_for_uri(node->uri().toUtf8().constData());
        g_autoptr(GFileInfo) srcInfo = nullptr;
        if (src) {
            srcInfo = g_file_query_info(src, "unix::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
        }
        if (SaveOne == node->responseType() || SaveAll == node->responseType()) {
           GFileWrapperPtr parentPtr = FileUtils::getFileParent(destFile);
           auto path = g_file_peek_path(parentPtr.get()->get());
           QDir destFileParentPath(path);
           if (!destFileParentPath.exists()) {
               destFileParentPath.mkpath(path);
           }
        }
        if (url.isLocalFile() && node->uri().endsWith(".desktop")) {
            GDesktopAppInfo* desktop_info = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
            if (G_IS_DESKTOP_APP_INFO(desktop_info)) {
                is_desktop_file = true;

                GKeyFile* key_file = g_key_file_new();
                QRegExp regExp (QString("\\ -\\ %1\\(\\d+\\)(\\.[0-9a-zA-Z\\.]+|)$").arg(QObject::tr("duplicate")));
                g_key_file_load_from_file(key_file, url.path().toUtf8().constData(), G_KEY_FILE_KEEP_COMMENTS, nullptr);
                QString locale_name = QLocale::system().name();
                QString local_generic_name_key = QString("Name[%1]").arg(locale_name);
                GError* error = NULL;
                if (g_key_file_has_key(key_file, G_KEY_FILE_DESKTOP_GROUP, local_generic_name_key.toUtf8().constData(), nullptr)) {
                    g_autofree char* val = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, local_generic_name_key.toUtf8().constData(), &error);
                    if (error) {
                        qWarning() << "get desktop file:" << node->uri() << " name error:" << error->code << " -- " << error->message;
                        g_error_free(error);
                        error = nullptr;
                    } else {
                        if (node->destBaseName().contains(regExp)) {
                            QString name1 = regExp.cap(0).replace(".desktop", "");
                            QString name = QString("%1%2.desktop").arg(val).arg(name1);
                            node->setDestFileName(name);
                        } else {
                            QString name = QString("%1 - %2(1).desktop").arg(val).arg(QObject::tr("duplicate"));
                            node->setDestFileName(name);
                        }
                    }
                } else if (g_key_file_has_key(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, nullptr)) {
                    g_autofree char* val = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, &error);
                    if (error) {
                        qWarning() << "get desktop file:" << node->uri() << " name error:" << error->code << " -- " << error->message;
                        g_error_free(error);
                        error = nullptr;
                    } else {
                        if (node->destBaseName().contains(regExp)) {
                            QString name1 = regExp.cap(0).replace(".desktop", "");
                            QString name = QString("%1%2.desktop").arg(val).arg(name1);
                            node->setDestFileName(name);
                        } else {
                            QString name = QString("%1 - %2(1).desktop").arg(val).arg(QObject::tr("duplicate"));
                            node->setDestFileName(name);
                        }
                    }
                }

                g_key_file_free(key_file);
                g_object_unref(desktop_info);
            }
        }


        FileCopy fileCopy (node->uri(), destFileUri, m_default_copy_flag,
                           getCancellable().get()->get(),
                           GFileProgressCallback(progress_callback),
                           this,
                           &err);
        fileCopy.setParentFlags(m_parent_flags);
        if (m_is_pause) fileCopy.pause();
        fileCopy.connect(this, &FileOperation::operationPause, &fileCopy, &FileCopy::pause, Qt::DirectConnection);
        fileCopy.connect(this, &FileOperation::operationResume, &fileCopy, &FileCopy::resume, Qt::DirectConnection);
        fileCopy.connect(this, &FileOperation::operationCancel, &fileCopy, &FileCopy::cancel, Qt::DirectConnection);
        if (m_is_pause) fileCopy.pause();
        fileCopy.run();
        if (err) {
            switch (err->code) {
            case G_IO_ERROR_INVALID_FILENAME: {
                QString newDestUri;
                if (makeFileNameValidForDestFS(m_current_src_uri, m_dest_dir_uri, &newDestUri)) {
                    if (newDestUri != destName) {
                        destName = newDestUri;
                        node->setDestFileName(newDestUri);
                        goto fallback_retry;
                    }
                }
                break;
            }
            case G_IO_ERROR_CANCELLED:
                return;
            case G_IO_ERROR_EXISTS:
                char* destFileName = g_file_get_uri(destFile.get()->get());
                if (NULL != destFileName) {
                    m_conflict_files << destFileName;
                    g_free(destFileName);
                }
                break;
            }

            FileOperationError except;
            auto errWrapperPtr = GErrorWrapper::wrapFrom(err);
            int handle_type = prehandle(err);
            except.errorType = ET_GIO;
            except.op = FileOpCopy;
            except.title = tr("File copy error");
            except.srcUri = m_current_src_uri;
            except.errorCode = err->code;
            except.errorStr = err->message;
            except.destDirUri = m_current_dest_dir_uri;

            //fix bug#121093, paste deleted file issue
            if (err->code == G_IO_ERROR_PERMISSION_DENIED) {
                except.errorStr = tr("Cannot opening file, permission denied!");
            }else if (err->code == G_IO_ERROR_NOT_FOUND) {
                except.errorStr = tr("File:%1 was not found.").arg(except.srcUri);
            }

            if (handle_type == Other) {
                switch (err->code) {
                case G_IO_ERROR_EXISTS: {
                    except.dlgType = ED_CONFLICT;
                    Q_EMIT errored(except);
                    auto typeData = except.respCode;
                    handle_type = typeData;
                    break;
                }
                case G_IO_ERROR_FILENAME_TOO_LONG: {
                    if (node->destBaseName().length() > 255 &&
                        m_dest_dir_uri.startsWith(QString("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展"))) {
                        QString msg = tr("The file name exceeds the limit");
                        Q_EMIT operationInfoMsgBox(msg);
                        if (m_is_long_name_file_operation) {
                            m_is_long_name_file_operation = false;
                        }
                        return;
                    }
                    except.dlgType = ED_RENAME;
                    Q_EMIT errored(except);
                    auto typeData = except.respCode;
                    handle_type = typeData;
                    break;
                }
                default: {
                    except.dlgType = ED_WARNING;
                    Q_EMIT errored(except);
                    auto typeData = except.respCode;
                    handle_type = typeData;
                    break;
                }
                }
            } else if (handle_type == SaveOne || handle_type == SaveAll) {
                if (node->destBaseName().length() > 255 && !m_is_long_name_error_msg_show &&
                    m_dest_dir_uri.startsWith(QString("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展"))) {
                    QString msg = tr("The file name exceeds the limit");
                    Q_EMIT operationInfoMsgBox(msg);
                    if (m_is_long_name_file_operation) {
                        m_is_long_name_file_operation = false;
                    }
                    m_is_long_name_error_msg_show = true;
                    return;
                }
            }
            //handle.
            node->setState(FileNode::Handling);
            switch (handle_type) {
            case IgnoreOne: {
                setHasError(true);
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case IgnoreAll: {
                setHasError(true);
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                m_prehandle_hash.insert(err->code, IgnoreOne);
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case OverWriteOne: {
                setHasError(true);
                node->setState(FileNode::Invalid);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    node->setErrorResponse(OverWriteOne);
                    auto result = udfCopyWarningDialog();
                    if (Cancel == result) {
                        cancel();
                    } else if (IgnoreOne == result || IgnoreAll == result) {
                        return;
                    }
                    break;
                }
                FileCopy fileOverWriteOneCopy (node->uri(), destFileUri,
                                   (GFileCopyFlags)(m_default_copy_flag | G_FILE_COPY_OVERWRITE),
                                   getCancellable().get()->get(),
                                   GFileProgressCallback(progress_callback),
                                   this,
                                   nullptr);
                fileCopy.connect(this, &FileOperation::operationPause, &fileOverWriteOneCopy, &FileCopy::pause, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationResume, &fileOverWriteOneCopy, &FileCopy::resume, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationCancel, &fileOverWriteOneCopy, &FileCopy::cancel, Qt::DirectConnection);
                if (m_is_pause) fileOverWriteOneCopy.pause();
                fileOverWriteOneCopy.run();
                if (fileCopy.getStatus() == FileCopy::FINISHED) {
                    node->setState(FileNode::Invalid);
                    node->setErrorResponse(OverWriteOne);
                } else {
                    node->setState(FileNode::Invalid);
                    node->setErrorResponse(OverWriteOne);
                }
                m_is_duplicated_copy = false;
                break;
            }
            case OverWriteAll: {
                setHasError(true);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    node->setErrorResponse(OverWriteOne);
                    m_prehandle_hash.insert(err->code, OverWriteOne);
                    m_is_udf_warning = true;
                    auto result = udfCopyWarningDialog();
                    if (Cancel == result) {
                        cancel();
                    } else if (IgnoreOne == result || IgnoreAll == result) {
                        return;
                    }
                    break;
                }
                FileCopy fileOverWriteOneCopy (node->uri(), destFileUri,
                                   (GFileCopyFlags)(m_default_copy_flag | G_FILE_COPY_OVERWRITE),
                                   getCancellable().get()->get(),
                                   GFileProgressCallback(progress_callback),
                                   this,
                                   nullptr);
                fileCopy.connect(this, &FileOperation::operationPause, &fileOverWriteOneCopy, &FileCopy::pause, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationResume, &fileOverWriteOneCopy, &FileCopy::resume, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationCancel, &fileOverWriteOneCopy, &FileCopy::cancel, Qt::DirectConnection);
                if (m_is_pause) fileOverWriteOneCopy.pause();
                fileOverWriteOneCopy.run();
                if (fileCopy.getStatus() == FileCopy::FINISHED) {
                    node->setState(FileNode::Invalid);
                    node->setErrorResponse(OverWriteOne);
                } else {
                    node->setState(FileNode::Invalid);
                    node->setErrorResponse(OverWriteOne);
                }
                m_prehandle_hash.insert(err->code, OverWriteOne);
                m_is_duplicated_copy = false;
                break;
            }
            case BackupOne: {
                node->setErrorResponse(BackupOne);
                // use custom name
                QString name = "";
                QStringList extendStr = node->destBaseName().split(".");
                if (extendStr.length() > 0) {
                    extendStr.removeAt(0);
                }
                QString endStr = extendStr.join(".");
                if (except.respValue.contains("name")) {
                    name = except.respValue["name"].toString();
                    if (endStr != "" && name.endsWith(endStr)) {
                        node->setDestFileName(name);
                    } else if ("" != endStr && "" != name) {
                        node->setDestFileName(name + "." + endStr);
                    }
                }

                while (FileUtils::isFileExsit(node->resolveDestFileUri(m_dest_dir_uri))) {
                    handleDuplicate(node);
                }
                goto fallback_retry;
            }
            case BackupAll: {
                node->setErrorResponse(BackupOne);
                while (FileUtils::isFileExsit(node->resolveDestFileUri(m_dest_dir_uri))) {
                    handleDuplicate(node);
                }
                m_prehandle_hash.insert(err->code, BackupOne);
                goto fallback_retry;
            }
            case Retry: {
                goto fallback_retry;
            }
            case RenameOne: {
                node->setErrorResponse(RenameOne);
                node->setDestFileName(except.respValue.value("newName").toString());
                goto fallback_retry;
            }
            case TruncateOne: {
                node->setErrorResponse(TruncateOne);
                auto respValut = except.respValue.value("cateType").toInt();
                if (m_cate_type == AllPost) {
                    respValut = Post;
                } else if (m_cate_type == ALLFront) {
                    respValut = Front;
                }
                node->truncateDestFileName(respValut);
                goto fallback_retry;
            }
            case TruncateAll: {
                node->setErrorResponse(TruncateOne);
                m_cate_type = except.respValue.value("cateType").toInt();
                node->truncateDestFileName(m_cate_type);
                if (Post == m_cate_type) {
                    m_cate_type = AllPost;
                } else if (Front == m_cate_type) {
                    m_cate_type = ALLFront;
                }
                m_prehandle_hash.insert(err->code, TruncateOne);
                goto fallback_retry;
            }
            case SaveOne: {
                node->setErrorResponse(SaveOne);
                if (!saveAsOtherPath()) {
                    break;
                }
                goto fallback_retry;
            }
            case SaveAll: {
                node->setErrorResponse(SaveOne);
                m_prehandle_hash.insert(err->code, SaveOne);
                if (!saveAsOtherPath()) {
                    break;
                }
                goto fallback_retry;
            }
            case Cancel: {
                node->setState(FileNode::Unhandled);
                cancel();
                break;
            }
            default:
                break;
            }
        } else {
            node->setState(FileNode::Handled);
        }

        if (is_desktop_file) {
            QUrl url(node->destUri());
            QRegExp regExp (QString("\\ -\\ %1\\(\\d+\\)(\\.[0-9a-zA-Z\\.]+|)$").arg(QObject::tr("duplicate")));
            GDesktopAppInfo *desktop_info = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
            if (G_IS_DESKTOP_APP_INFO(desktop_info)) {
                GKeyFile *key_file = g_key_file_new();
                g_key_file_load_from_file(key_file, url.path().toUtf8().constData(), G_KEY_FILE_KEEP_COMMENTS, nullptr);
                QString locale_name = QLocale::system().name();
                QString ext;
                QString local_generic_name_key = QString("Name[%1]").arg(locale_name);

                g_autofree char* nameStr = nullptr;

                if (node->destBaseName().contains(regExp)) {
                    ext = regExp.cap(0).replace(".desktop", "");
                }

                if (g_key_file_has_key(key_file, G_KEY_FILE_DESKTOP_GROUP, local_generic_name_key.toUtf8().constData(), nullptr)) {
                    nameStr = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, local_generic_name_key.toUtf8().constData(), nullptr);
                    g_key_file_set_value(key_file, G_KEY_FILE_DESKTOP_GROUP, local_generic_name_key.toUtf8().constData(), QString(nameStr + ext).toUtf8().constData());
                } else {
                    nameStr = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, nullptr);
                    g_key_file_set_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, QString(nameStr + ext).toUtf8().constData());
                }

                qDebug() << "set desktop name:" << nameStr + ext << "  -- " << url.path();

                GError*         error = NULL;
                g_key_file_save_to_file(key_file, url.path().toUtf8().constData(), &error);
                if (error) {
                    qWarning() << "save file error:" << error->code << "  --  " << error->message;
                    g_error_free(error);
                    error = nullptr;
                }
                g_key_file_free(key_file);
                g_object_unref(desktop_info);

                if (nullptr != srcInfo) {
                    g_autoptr(GFile) destFile = g_file_new_for_uri(node->destUri().toUtf8().constData());
                    if (destFile) {
                        g_file_set_attribute_uint32(destFile, G_FILE_ATTRIBUTE_UNIX_MODE, g_file_info_get_attribute_uint32(srcInfo, G_FILE_ATTRIBUTE_UNIX_MODE), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
                    }
                }
            } else {
                qDebug() << "desktop file:" << node->destUri() << " is wrong";
            }
        }

        if (SaveOne == node->responseType() || SaveAll == node->responseType()) {
            m_dest_dir_uri = destDir;
        }
        m_current_offset += node->size();
//        fileSync(srcUri, destFileUri);
        Q_EMIT operationProgressedOne(node->uri(), node->destUri(), node->size());

        if(srcUri.endsWith(".dsps") && destFileUri.endsWith(".dsps")){
            m_srcUrisOfCopyDspsFiles.append(FileUtils::urlDecode(srcUri));
            m_destUrisOfCopyDspsFiles.append(FileUtils::urlDecode(destFileUri));
        }
    }

    destFile.reset();
}

void FileCopyOperation::rollbackNodeRecursively(FileNode *node)
{
    switch (node->state()) {
    case FileNode::Handling:
    case FileNode::Handled: {
        if (node->isFolder()) {
            auto children = node->children();
            for (auto child : *children) {
                rollbackNodeRecursively(child);
            }
            GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
            //FIXME: there's a certain probability of failure to delete the folder without
            //any problem happended. because somehow an empty file will created in the folder.
            //i don't know why, but it is obvious that i have to delete them at first.
            bool is_folder_deleted = g_file_delete(dest_file, nullptr, nullptr);
            if (!is_folder_deleted) {
                FileEnumerator e;
                e.setEnumerateDirectory(node->destUri());
                e.enumerateSync();
                for (auto folder_child : *node->children()) {
                    if (!folder_child->destUri().isEmpty()) {
                        GFile *tmp_file = g_file_new_for_uri(folder_child->destUri().toUtf8().constData());
                        g_file_delete(tmp_file, nullptr, nullptr);
                        g_object_unref(tmp_file);
                    }
                    g_file_delete(dest_file, nullptr, nullptr);
                }
            }
            g_object_unref(dest_file);
        } else {
            if (!m_conflict_files.contains(node->destUri())) {
                GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
                g_file_delete(dest_file, nullptr, nullptr);
                g_object_unref(dest_file);
            }
        }
        operationRollbackedOne(node->destUri(), node->uri());
        break;
    }
    default: {
        //make sure all nodes were rollbacked.
        if (node->isFolder()) {
            auto children = node->children();
            for (auto child : *children) {
                rollbackNodeRecursively(child);
            }
        }
        break;
    }
    }
}

void FileCopyOperation::run()
{
    if (isCancelled())
        return;

    if (hook_check_operation_valid) {
        if (!hook_check_operation_valid(m_source_uris, m_dest_dir_uri, FILE_OPERATION_COPY)) {
            setHasError(true);
            cancel();
            Q_EMIT operationFinished();
            return;
        }
    }

    Q_EMIT operationStarted();

#ifdef LINGMO_UDF_BURN
    std::shared_ptr<FileOperationHelper> mHelper = std::make_shared<FileOperationHelper>(m_dest_dir_uri);
    if (mHelper->isUnixCDDevice()) {
        m_is_udf_burn_work = true;
        bool isMountpoint = false;
        mHelper->judgeSpecialDiscOperation();
        auto discType = mHelper->getDiscType();
        if (discType.contains("DVD-RW") || discType.contains("CD-RW")) {
            Q_EMIT operationWithoutRecording();
        }
        g_autoptr(GFile) file = g_file_new_for_uri (m_dest_dir_uri.toUtf8().constData());
        if (file) {
            g_autoptr(GFileInfo) fileInfo = g_file_query_info(file, G_FILE_ATTRIBUTE_UNIX_IS_MOUNTPOINT, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
            if (fileInfo) {
                isMountpoint = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_UNIX_IS_MOUNTPOINT);
            }
        }
        if (!mHelper->dealDVDReduce().isEmpty() && isMountpoint) {
            m_dest_dir_uri = mHelper->dealDVDReduce();
        }
    }
#endif

    Q_EMIT operationRequestShowWizard();

    goffset *total_size = new goffset(0);

    QList<FileNode*> nodes;
    for (auto uri : m_source_uris) {
        QString szTempUri = uri;
        if(szTempUri.startsWith("filesafe:///") && szTempUri.remove("filesafe:///").indexOf("/") == -1) {
            continue;
        }
        FileNode *node = new FileNode(uri, nullptr, m_reporter);
        if (szTempUri.startsWith("trash:///")) {
            auto file = g_file_new_for_uri(uri.toUtf8().constData());
            auto info = g_file_query_info (file,
                                      G_FILE_ATTRIBUTE_TRASH_ORIG_PATH,
                                      G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
            auto basename = g_path_get_basename (g_file_info_get_attribute_byte_string (info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH));
            if (basename) {
                node->setDestFileName(basename);
            }
        }
        node->findChildrenRecursively();
        node->computeTotalSize(total_size);
        nodes << node;
    }

    Q_EMIT operationPrepared();

    m_total_size = *total_size;
    delete total_size;
    Q_EMIT operationTotalFileSize(m_total_size);
    if (isCancelled()) {
        Q_EMIT operationFinished();
        return;
    }

    //判断剩余空间是否满足拷贝所需空间
    g_autoptr (GFile) destGfile = g_file_new_for_uri(m_dest_dir_uri.toUtf8().constData());
    g_autofree char* destPath = g_file_get_path(destGfile);
    QStorageInfo storage(destPath);
    if (!storage.isValid()) {
        qWarning() << "The file path is not mounted correctly";
    } else {
        // If the storage is valid, get the available disk space
        quint64 diskFreeSpace = storage.bytesAvailable();
        if(m_total_size > diskFreeSpace && diskFreeSpace > 0) {
            // If there is not enough space, create a new FileOperationError object
            FileOperationError except;
            QString name;
            if (storage.rootPath() == "/") {
                name = tr("File System");
            } else if (storage.rootPath() == "/data") {
                name = tr("Data");
            } else {
                name = storage.name();
            }
            double total_file_size_gb = (double)m_total_size / (1024 * 1024 * 1024);
            double need_total_size_gb = (double)(m_total_size - diskFreeSpace) / (1024 * 1024 * 1024);
            except.errorStr = tr("%1 no space left on device. "
                                 "Copy file size: %2 GB, "
                                 "Space needed: %3 GB.").arg(name).arg(QString::number(total_file_size_gb, 'f', 3))
                    .arg(QString::number(need_total_size_gb, 'f', 3));
            except.op = FileOpCopy;
            except.title = tr("File copy error");
            except.dlgType = ED_WARNING;
            Q_EMIT errored(except);
            Q_EMIT operationFinished();
            return;
        }
    }

    m_srcUrisOfCopyDspsFiles.clear();
    m_destUrisOfCopyDspsFiles.clear();

    for (auto node : nodes) {
        copyRecursively(node);
    }
    Q_EMIT operationProgressed();

    if (isCancelled()) {
        Q_EMIT operationStartRollbacked();
        for (auto file : nodes) {
            if (isCancelled()) {
                rollbackNodeRecursively(file);
            }
        }
    }

    //comment to fix bug#177163, copy and paste file has error and play success sound issue
    //copy operation has finished, no need reset flag, keep the same with move-operation
    //setHasError(false);
    QStringList burnUris = m_source_uris;
    for (auto node : nodes) {
        if (!isCancelled()) {
            if (node->state() != FileNode::Handled) {
                // 如果出现了不可处理的文件操作异常，则不允许用户进行此操作的undo/redo
                // FIXME: 对于多级自文件的场景，这么处理是有问题的，也许判断是否有不可回滚的异常处理机制需要再优化
                setHasError(true);
            }
            m_info->m_node_map.insert(node->uri(), node->destUri());
            if (m_is_udf_burn_work) {
                switch (node->responseType()) {
                case IgnoreOne:
                case OverWriteOne:
                    burnUris.removeOne(node->uri());
                    break;
                case BackupOne:
                    burnUris.replaceInStrings(node->uri(), node->destUri());
                    break;
                default:
                    break;
                }
            }
        }
        delete node;
    }
    m_info->m_dest_uris = m_info->m_node_map.values();
    nodes.clear();
#ifdef LINGMO_UDF_BURN
    if (mHelper->isUnixCDDevice() && !isCancelled()) {
        if(!mHelper->discWriteOperation(burnUris, m_dest_dir_uri)) {
            FileOperationError except;
            except.errorType = ET_CUSTOM;
            except.op = FileOpCopy;
            except.title = tr("File copy error");
            except.srcUri = m_source_uris.first();
            except.errorStr = mHelper->getDiscError();
            except.destDirUri = m_dest_dir_uri;
            except.dlgType = ED_WARNING;
            Q_EMIT errored(except);
        }
        m_is_udf_burn_work = false;
    } else {
        if (m_is_udf_burn_work) {
            m_is_udf_burn_work = false;
        }
    }
#endif
    Q_EMIT operationFinished();
    sendSrcAndDestUrisOfCopyDspsFiles();
    if (m_is_long_name_file_operation) {
        Q_EMIT operationSaveAsLongNameFile(m_save_as_other_uri);
    }
}

bool FileCopyOperation::copyLinkedFile(FileNode *node, GFileInfo *info, GFileWrapperPtr file)
{
    g_autoptr(GError) err = nullptr;
    QString target = g_file_info_get_symlink_target(info);
    if (!target.startsWith("/"))
    {
        QString parentUri = FileUtils::getParentUri(m_current_src_uri);
        target = QUrl(parentUri).path() + "/" + target;
    }
    target.replace("%", "%25");
    target.replace("#", "%23");
    target.prepend("file://");
    QUrl url(target);
    auto utf8 = url.path().toUtf8();
    auto symlinkValue = utf8.constData();
    g_file_make_symbolic_link(file.get()->get(), symlinkValue, nullptr, &err);
    if (err) {
        qDebug() << "linkrun:" << err->message;
        FileOperationError except;
        except.srcUri = m_current_src_uri;
        except.errorType = ET_GIO;
        except.isCritical = true;
        except.errorStr = err->message;
        except.errorCode = err->code;
        except.op = FileOpCopy;
        except.title = tr("Link file error");
        except.destDirUri = m_current_dest_dir_uri;
        auto handle_type = prehandle(err);

        if (handle_type == Other) {
            switch (err->code) {
            case G_IO_ERROR_EXISTS: {
                except.dlgType = ED_CONFLICT;
                Q_EMIT errored(except);
                auto typeData = except.respCode;
                handle_type = typeData;
                break;
            }
            case G_IO_ERROR_FILENAME_TOO_LONG: {
                except.dlgType = ED_RENAME;
                Q_EMIT errored(except);
                auto typeData = except.respCode;
                handle_type = typeData;
                break;
            }
            default: {
                except.dlgType = ED_WARNING;
                Q_EMIT errored(except);
                auto typeData = except.respCode;
                handle_type = typeData;
                break;
            }
            }
        }
        //handle.
        switch (handle_type) {
        node->setState(FileNode::Handling);
        case IgnoreOne: {
            setHasError(true);
            node->setState(FileNode::Unhandled);
            node->setErrorResponse(IgnoreOne);
            return true;
        }
        case IgnoreAll: {
            setHasError(true);
            node->setState(FileNode::Unhandled);
            node->setErrorResponse(IgnoreOne);
            m_prehandle_hash.insert(err->code, IgnoreOne);
            return true;
        }
        case OverWriteOne: {
            setHasError(true);
            bool success = g_file_delete(file.get()->get(),  nullptr, nullptr);
            if (!success) {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(Invalid);
                qWarning()<<"failed to remove Original dest file";
                return true;
            } else {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
            }
            return false;
        }
        case OverWriteAll: {
            setHasError(true);
            m_prehandle_hash.insert(err->code, OverWriteOne);
            bool success = g_file_delete(file.get()->get(),  nullptr, nullptr);
            if (!success) {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(Invalid);
                qWarning()<<"failed to remove Original dest file";
                return true;
            } else {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
            }
            return false;
        }
        case BackupOne: {
            node->setErrorResponse(BackupOne);
            QString name = "";
            QStringList extendStr = node->destBaseName().split(".");
            if (extendStr.length() > 0) {
                extendStr.removeAt(0);
            }
            QString endStr = extendStr.join(".");
            if (except.respValue.contains("name")) {
                name = except.respValue["name"].toString();
                if (endStr != "" && name.endsWith(endStr)) {
                    node->setDestFileName(name);
                } else if ("" != endStr && "" != name) {
                    node->setDestFileName(name + "." + endStr);
                }
            }

            while (FileUtils::isFileExsit(node->resolveDestFileUri(m_dest_dir_uri))) {
                handleDuplicate(node);
            }
            return false;
        }
        case BackupAll: {
            node->setErrorResponse(BackupOne);
            while (FileUtils::isFileExsit(node->resolveDestFileUri(m_dest_dir_uri))) {
                handleDuplicate(node);
            }
            m_prehandle_hash.insert(err->code, BackupOne);
            return false;
        }
        case Retry: {
            return false;
        }
        case RenameOne: {
            node->setErrorResponse(RenameOne);
            node->setDestFileName(except.respValue.value("newName").toString());
            return false;
        }
        case Cancel: {
            node->setState(FileNode::Unhandled);
            cancel();
            break;
        }
        default:
            break;
        }
    } else {
        node->setState(FileNode::Handled);
    }
    return true;
}

ExceptionResponse FileCopyOperation::udfCopyWarningDialog()
{
    FileOperationError except;
    ExceptionResponse typeData = Invalid;
    except.errorType = ET_CUSTOM;
    except.op = FileOpCopy;
    except.title = tr("File copy error");
    except.srcUri = m_source_uris.first();
    except.errorStr = tr("Burning does not support replacement");
    except.destDirUri = m_dest_dir_uri;
    except.dlgType = ED_WARNING;
    Q_EMIT errored(except);
    typeData = except.respCode;
    return typeData;
}

bool FileCopyOperation::saveAsOtherPath()
{
    QString destUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展";
    QDBusInterface iface ("com.lingmo.file.system.fuse","/com/lingmo/file/system/fuse","com.lingmo.file.system.fuse",QDBusConnection::systemBus());
    if (!iface.isValid()) {
        qWarning() << "fuse dbus has error : " << iface.lastError();
        return false;
    }
    if (!FileUtils::isFileExsit(destUri)) {
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        dir.mkdir("扩展");
        auto path = dir.path().append("/扩展");
        QDBusReply<bool> reply = iface.call("SetSetings", "true", path);
        if (reply.isValid()) {
            if (!reply.value()) {
                qInfo() << "fuse setings false";
                return false;
            }
        } else {
            qWarning() << "fuse dbus has error : " << iface.lastError();
            return false;
        }
    }
    if (m_current_src_uri.startsWith(destUri)) {
        return false;
    }
    m_save_as_other_uri = destUri;
    if (m_dest_dir_uri == m_save_as_other_uri) {
        if (m_is_long_name_file_operation) {
            m_is_long_name_file_operation = false;
        }
        return false;
    }
    m_dest_dir_uri = m_save_as_other_uri;
    if (!m_is_long_name_file_operation) {
        m_is_long_name_file_operation = true;
    }
    return true;
}

void FileCopyOperation::cancel()
{
    setHasError(true);
    if (m_reporter) {
        m_reporter->cancel();
    }

    ClipboardUtils::popLastTargetDirectoryUri(m_dest_dir_uri);

    FileOperation::cancel();
}
