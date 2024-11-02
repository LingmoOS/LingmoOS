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

#include "file-move-operation.h"
#include "file-node-reporter.h"
#include "file-node.h"
#include "file-enumerator.h"
#include "file-info.h"

#include "file-operation-manager.h"
#include "file-label-model.h"

#include <QDir>
#include <QProcess>
#include <QStorageInfo>
#include <QStandardPaths>
#include <QDBusInterface>
#include <QDBusReply>
#include <file-copy.h>

using namespace Peony;

static void handleDuplicate(FileNode *node)
{
    node->setDestFileName(FileUtils::handleDuplicateName(node->destBaseName()));
}

FileMoveOperation::FileMoveOperation(QStringList sourceUris, QString destDirUri, QObject *parent) : FileOperation (parent)
{
    for (auto u : sourceUris) {
        if (u.split("://").length() != 2) {
            sourceUris.removeOne (u);
        }
    }

    m_src_uris = sourceUris;

    /* favorite://xxx特殊处理,例如本机共享，bug#83353 */
    if (destDirUri.startsWith("favorite://")) {
        destDirUri = FileUtils::getTargetUri(destDirUri);
        if (destDirUri.isEmpty ()) {
            destDirUri = "favorite:///";
        }
    }

    m_dest_dir_uri = FileUtils::urlEncode(destDirUri);
    m_info = std::make_shared<FileOperationInfo>(sourceUris, destDirUri, FileOperationInfo::Move);

    QString srcId = FileUtils::getFileSystemId(m_src_uris.first());
    QString destId = FileUtils::getFileSystemId(m_dest_dir_uri);
    if (srcId.length() > 0 && srcId == destId)
        m_is_same_fs = true;
    else
        m_is_same_fs = false;
}

FileMoveOperation::~FileMoveOperation()
{
    if (m_reporter)
        delete m_reporter;
}

void FileMoveOperation::setCopyMove(bool copyMove)
{
    // use setAction() instead.
    setAction(copyMove? Qt::MoveAction: Qt::TargetMoveAction);

    return;
    m_copy_move = copyMove;
    m_info.get()->m_type = copyMove? FileOperationInfo::Copy: FileOperationInfo::Move;
    m_info.get()->m_opposite_type = copyMove? FileOperationInfo::Delete: FileOperationInfo::Move;
}

void FileMoveOperation::setAction(Qt::DropAction action)
{
    m_move_action = action;
    m_info.get()->m_drop_action = action;
    switch (action) {
    case Qt::CopyAction: {
        m_info.get()->m_type = FileOperationInfo::Copy;
        m_info.get()->m_opposite_type = FileOperationInfo::Delete;
        break;
    }
    default: {
        m_info.get()->m_type = FileOperationInfo::Move;
        m_info.get()->m_opposite_type = FileOperationInfo::Move;
        break;
    }
    }
}

void FileMoveOperation::setUriSort(bool isSort, int type)
{
    if (isSort != m_is_sort) {
        m_is_sort = isSort;
    }
    if (type != m_sort_type) {
        m_sort_type = type;
    }
}

void FileMoveOperation::setSearchOperation(const bool &isSearch)
{
    if (isSearch != m_is_search) {
        m_is_search = isSearch;
    }
}

void FileMoveOperation::progress_callback(goffset current_num_bytes,
        goffset total_num_bytes,
        FileMoveOperation *p_this)
{
    if (total_num_bytes < current_num_bytes)
        return;

    QUrl url(p_this->m_current_src_uri);
    auto currnet = p_this->m_current_offset + current_num_bytes;
    auto total = p_this->m_total_size;
    auto fileIconName = FileUtilsPrivate::getFileIconName(p_this->m_current_src_uri);
    auto destFileName = FileUtils::isFileDirectory(p_this->m_current_dest_dir_uri) ?
                p_this->m_current_dest_dir_uri + "/" + url.fileName() : p_this->m_current_dest_dir_uri;

//    qDebug() << "move: " << p_this->m_current_src_uri << "  ---  "  << destFileName << currnet << "/" << total << (float(currnet) / total);
    Q_EMIT p_this->FileProgressCallback(p_this->m_current_src_uri, destFileName, fileIconName, currnet, total);
    //format: move srcUri to destDirUri: curent_bytes(count) of total_bytes(count).
}

ExceptionResponse FileMoveOperation::prehandle(GError *err)
{
    //setHasError(true);

    /**
     * @note FIX BUG 79363
     * 文件传输中拔掉U盘，第一次报错:G_IO_ERROR_FAILED，后续报错:G_IO_ERROR_NOT_FOUND
     */
    if (G_IO_ERROR_NOT_FOUND == err->code && m_prehandle_hash.contains(G_IO_ERROR_FAILED)) {
        return IgnoreAll;
    }

    if (m_prehandle_hash.contains(err->code))
        return m_prehandle_hash.value(err->code);

    return Other;
}

void FileMoveOperation::move()
{
    if (isCancelled())
        return;

    // 特殊处理 favorite:///
    g_autoptr (GFile) destFile = g_file_new_for_uri (m_dest_dir_uri.toUtf8 ().constData ());
    g_autofree gchar* destSchema = g_file_get_uri_scheme (destFile);

    if (G_IS_FILE (destFile) && destSchema && !g_ascii_strcasecmp (destSchema, "favorite")) {
        for (auto src : m_src_uris) {
            g_autoptr (GFile) srcFile = g_file_new_for_uri (src.toUtf8 ().constData ());
            g_autoptr (GError) error = NULL;
            // 注意：不可能报冲突错误
            g_file_move (srcFile, destFile, m_default_copy_flag, getCancellable().get()->get(), GFileProgressCallback (progress_callback), this, &error);
            if (error) {
//                setHasError ();
                FileOperationError except;
                int handle_type = prehandle(error);
                except.errorType = ET_GIO;
                except.op = FileOpMove;
                except.title = tr("Move file error");
                except.errorCode = error->code;
                except.errorStr = error->message;
                except.srcUri = m_current_src_uri;
                except.destDirUri = m_current_dest_dir_uri;
                except.isCritical = false;
                if (handle_type == Other) {
                    if (G_IO_ERROR_EXISTS != error->code) {
                        except.dlgType = ED_WARNING;
                        Q_EMIT errored(except);
                    }
                    // ignore multiple bounces
                    if (except.errorCode == G_IO_ERROR_NOT_SUPPORTED) {
                        m_prehandle_hash.insert(error->code, IgnoreOne);
                    }
                }
            }
        }

        Q_EMIT operationFinished();

        return;
    }

    QList<FileNode*> nodes;
    QList<FileNode*> errNode;

    GError *err = nullptr;
    m_total_count = m_src_uris.count();
    auto destDir = wrapGFile(g_file_new_for_uri(m_dest_dir_uri.toUtf8().constData()));

    // file move
    for (auto srcUri : m_src_uris) {
        if (isCancelled())
            return;

        auto node = new FileNode(srcUri, nullptr, nullptr);
        node->setState(FileNode::Handling);

        auto srcFile = wrapGFile(g_file_new_for_uri(srcUri.toUtf8().constData()));
        char *base_name = g_file_get_basename(srcFile.get()->get());
        auto destFile = wrapGFile(g_file_resolve_relative_path(destDir.get()->get(), base_name));
        g_autofree char* destUri = g_file_get_uri(destFile.get()->get());
        node->setDestUri(destUri);
        if (srcUri.startsWith("trash:///")) {
            auto file = g_file_new_for_uri(srcUri.toUtf8().constData());
            auto info = g_file_query_info (file,
                                      G_FILE_ATTRIBUTE_TRASH_ORIG_PATH,
                                      G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
            auto basename = g_path_get_basename (g_file_info_get_attribute_byte_string (info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH));
            if (basename) {
                node->setDestFileName(basename);
            }
        }

        g_file_move(srcFile.get()->get(),
                    destFile.get()->get(),
                    m_default_copy_flag,
                    getCancellable().get()->get(),
                    GFileProgressCallback(progress_callback), this, &err);
        if (err) {
            errNode << node;
            //need free err info, fix bug#164662, drag file conflicts cause file lost issue
            g_error_free(err);
            err = nullptr;
        } else {
            node->setState(FileNode::Handled);
            Q_EMIT remoteFileEvent(103, srcUri, destUri);

            /* 文件（夹）剪切/鼠标拖动move后，标识模式更新 */
            g_autofree char* sourceUri = g_file_get_uri(srcFile.get()->get());
            QList<int> labelIds = FileLabelModel::getGlobalModel()->getFileLabelIds(sourceUri);
            qDebug()<< "move(), update label, labelIds:"<< labelIds.size() ;

            FileLabelModel::getGlobalModel()->removeFileLabel(sourceUri);
            for(auto &id: labelIds){
                if(id <= 0)
                    continue;
                qDebug()<< "move(), update label, id:"<< id <<" uri:"<<destUri<<" urlDecode:"<<FileUtils::urlDecode(destUri);
                FileLabelModel::getGlobalModel()->addLabelToFile(destUri, id);
            }
        }

        nodes << node;
    }

    // file copy-delete
    bool hasFolder = false;
    goffset *total_size = new goffset(0);
    for (auto eNode : errNode) {
        if (isCancelled())
            return;

        eNode->findChildrenRecursively();
        eNode->computeTotalSize(total_size);
        if(eNode->isFolder())
            hasFolder = true;
    }
    m_total_size = *total_size;
    operationPreparedOne("", m_total_size);
    delete total_size;

    //判断剩余空间是否满足拷贝所需空间
    g_autoptr (GFile) destGfile = g_file_new_for_uri(m_dest_dir_uri.toUtf8().constData());
    g_autofree gchar* destPath = g_file_get_path(destGfile);
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
            except.op = FileOpMove;
            except.title = tr("File move error");
            except.dlgType = ED_WARNING;
            Q_EMIT errored(except);
            Q_EMIT operationFinished();
            return;
        }
        // If there is enough space, emit the operationTotalFileSize signal with the total size of the files to be copied
        Q_EMIT operationTotalFileSize(m_total_size);
    }

    operationPrepared();

    if (!errNode.isEmpty()) {
        if (m_move_action == Qt::TargetMoveAction) {
            m_info.get()->m_type = FileOperationInfo::Move;
            m_info.get()->m_opposite_type = FileOperationInfo::Move;
        } else if(m_move_action == Qt::MoveAction){
            //only when action is copy can delete when ctrl+z
            //当前文件夹替换其实做的是合并操作，撤销操作不能删除文件夹，有丢失文件的风险
            if (m_is_same_fs && ! hasFolder){
                m_info.get()->m_type = FileOperationInfo::Copy;
                m_info.get()->m_opposite_type = FileOperationInfo::Delete;
            }
        } else{
            m_info.get()->m_type = FileOperationInfo::Copy;
            m_info.get()->m_opposite_type = FileOperationInfo::Delete;
        }
    }
    for (auto node : errNode) {
        if (isCancelled())
            return;
        moveForceUseFallback(node);
        fileSync(node->uri(), node->destUri());
    }

    operationStartSnyc();

#if 0
    for (auto file : nodes) {
        if (isCancelled())
            return;

        QString srcUri = file->uri();
        m_current_count = nodes.indexOf(file) + 1;
        m_current_src_uri = srcUri;
        m_current_dest_dir_uri = m_dest_dir_uri;

        auto srcFile = wrapGFile(g_file_new_for_uri(srcUri.toUtf8().constData()));
        char *base_name = g_file_get_basename(srcFile.get()->get());
        auto destFile = wrapGFile(g_file_resolve_relative_path(destDir.get()->get(),
                                  base_name));

        char *dest_uri = g_file_get_uri(destFile.get()->get());
        file->setDestUri(dest_uri);

        g_free(dest_uri);
        g_free(base_name);

//retry:
        GError *err = nullptr;
        g_file_move(srcFile.get()->get(),
                    destFile.get()->get(),
                    m_default_copy_flag,
                    getCancellable().get()->get(),
                    GFileProgressCallback(progress_callback),
                    this,
                    &err);
        if (err) {
            errNode << file;
        } else {
            file->setState(FileNode::Handled);
        }

        if (err) {
//            setHasError(true);
            auto errWrapper = GErrorWrapper::wrapFrom(err);
            switch (errWrapper.get()->code()) {
            case G_IO_ERROR_CANCELLED: {
                for (auto node : nodes) {
                    delete node;
                }
                nodes.clear();
                return;
            }
            case G_IO_ERROR_NOT_SUPPORTED:
            case G_IO_ERROR_WOULD_RECURSE:
            case G_IO_ERROR_EXISTS: {
                moveForceUseFallback(file);
                operationStartSnyc();
                continue;
            }
            default:
                break;
            }
            int handle_type = prehandle(err);
            FileOperationError except;
            except.srcUri = srcUri;
            except.destDirUri = m_dest_dir_uri;
            except.isCritical = false;
            except.op = FileOpMove;
            except.title = tr("Move file error");
            except.errorCode = err->code;
            except.errorStr = err->message;
            except.errorType = ET_GIO;
            if (handle_type == Other) {
                auto responseTypeWrapper = Invalid;
                if (G_IO_ERROR_EXISTS == err->code) {
                    except.dlgType = ED_CONFLICT;
                    Q_EMIT errored(except);
                    responseTypeWrapper = except.respCode;
                } else {
                    except.dlgType = ED_WARNING;
                    Q_EMIT errored(except);
                    responseTypeWrapper = except.respCode;
                }
                handle_type = responseTypeWrapper;
                //block until error has been handled.
            }

            GError *handled_err = nullptr;
            switch (handle_type) {
            case IgnoreOne: {
                file->setState(FileNode::Unhandled);
                file->setErrorResponse(IgnoreOne);
                //skip to next loop.
                break;
            }
            case IgnoreAll: {
                file->setState(FileNode::Unhandled);
                file->setErrorResponse(IgnoreOne);
                m_prehandle_hash.insert(err->code, IgnoreOne);
                break;
            }
            case OverWriteOne: {
                file->setState(FileNode::Handled);
                file->setErrorResponse(OverWriteOne);
                g_file_move(srcFile.get()->get(),
                            destFile.get()->get(),
                            GFileCopyFlags(G_FILE_COPY_NOFOLLOW_SYMLINKS|
                                           G_FILE_COPY_OVERWRITE),
                            getCancellable().get()->get(),
                            GFileProgressCallback(progress_callback),
                            this,
                            &handled_err);
                break;
            }
            case OverWriteAll: {
                file->setState(FileNode::Handled);
                file->setErrorResponse(OverWriteOne);
                g_file_move(srcFile.get()->get(),
                            destFile.get()->get(),
                            GFileCopyFlags(G_FILE_COPY_NOFOLLOW_SYMLINKS|
                                           G_FILE_COPY_OVERWRITE),
                            getCancellable().get()->get(),
                            GFileProgressCallback(progress_callback),
                            this,
                            &handled_err);
                m_prehandle_hash.insert(err->code, OverWriteOne);
                break;
            }
            case BackupOne: {
                file->setState(FileNode::Handled);
                file->setErrorResponse(BackupOne);
                // use custom name
                QString name = "";
                QStringList extendStr = file->destBaseName().split(".");
                if (extendStr.length() > 0) {
                    extendStr.removeAt(0);
                }
                QString endStr = extendStr.join(".");
                if (except.respValue.contains("name")) {
                    name = except.respValue["name"].toString();
                    if (endStr != "" && name.endsWith(endStr)) {
                        file->setDestFileName(name);
                    } else if ("" != endStr && "" != name) {
                        file->setDestFileName(name + "." + endStr);
                    }
                }
                while (FileUtils::isFileExsit(file->destUri())) {
                    handleDuplicate(file);
                    file->resolveDestFileUri(m_dest_dir_uri);
                }
                auto handledDestFileUri = file->resolveDestFileUri(m_dest_dir_uri);
                while (FileUtils::isFileExsit(handledDestFileUri))
                {
                    handledDestFileUri = file->resolveDestFileUri(m_dest_dir_uri);
                }
                auto handledDestFile = wrapGFile(g_file_new_for_uri(handledDestFileUri.toUtf8()));
                g_file_copy(srcFile.get()->get(),
                            handledDestFile.get()->get(),
                            GFileCopyFlags(m_default_copy_flag|G_FILE_COPY_BACKUP),
                            getCancellable().get()->get(),
                            GFileProgressCallback(progress_callback),
                            this,
                            &handled_err);
//                setHasError(false);
                break;
            }
            case BackupAll: {
                m_prehandle_hash.insert(err->code, BackupOne);
                goto retry;
                break;
            }
            case Retry: {
                goto retry;
            }
            case Cancel: {
                file->setState(FileNode::Unhandled);
                cancel();
                break;
            }
            default:
                break;
            }

            except.srcUri = srcUri;
            except.errorType = ET_GIO;
            except.errorCode = err->code;
            except.errorStr = err->message;
            except.op = FileOpMove;
            except.title = tr("Move file error");
            except.destDirUri = m_dest_dir_uri;
            except.isCritical = true;
            if (handled_err) {
                auto handledErr = GErrorWrapper::wrapFrom(handled_err);
                FileOperationError except;
                if (G_IO_ERROR_EXISTS == handled_err->code) {
                    except.dlgType = ED_CONFLICT;
                    Q_EMIT errored(except);
                } else {
                    except.dlgType = ED_WARNING;
                    Q_EMIT errored(except);
                }

                auto response = except.respCode;
            }
        } else {
            file->setState(FileNode::Handled);
        }
        //FIXME: ignore the total size when using native move.
        operationProgressedOne(file->uri(), file->destUri(), 0);
        fileSync(file->uri(), file->destUri());
    }
#endif
    //native move has not clear operation.
    operationProgressed();

    //FIXME: if native move function get into error,
    //such as the target is existed, the rollback might
    //get into error too.

    if (isCancelled()) {
        for (auto node : nodes) {
            rollbackNodeRecursively(node);
        }
    }

    //release node
    m_info.get()->m_src_uris.clear();
    m_info.get()->m_dest_uris.clear();
    m_burn_uris = m_src_uris;
    for (auto file : nodes) {
        m_info.get()->m_src_uris<<file->uri();
        m_info.get()->m_dest_uris<<file->destUri();
        if (!isCancelled() && m_is_udf_burn_work) {
            switch (file->responseType()) {
            case IgnoreOne:
            case OverWriteOne:
                m_burn_uris.removeOne(file->uri());
                break;
            case BackupOne:
                m_burn_uris.replaceInStrings(file->uri(), file->destUri());
                break;
            default:
                break;
            }
        }
        delete file;
    }
    nodes.clear();
}

void FileMoveOperation::rollbackNodeRecursively(FileNode *node)
{
    if (node->state() != FileNode::Handled) {
        operationRollbackedOne(node->destUri(), node->uri());
        return;
    }

    if (node->isFolder()) {
        if (m_info->m_type == FileOperationInfo::Move) {
            auto dir = wrapGFile(g_file_new_for_uri(node->uri().toUtf8().constData()));
            g_file_make_directory(dir.get()->get(), nullptr, nullptr);
        }
        for (auto child : *node->children()) {
            rollbackNodeRecursively(child);
        }
        if (m_info->m_type == FileOperationInfo::Copy) {
            auto destDir = wrapGFile(g_file_new_for_uri(node->destUri().toUtf8().constData()));
            g_file_delete(destDir.get()->get(), nullptr, nullptr);
        }

        operationRollbackedOne(node->destUri(), node->uri());
    } else {
        auto sourceFile = wrapGFile(g_file_new_for_uri(node->uri().toUtf8().constData()));
        auto destFile = wrapGFile(g_file_new_for_uri(node->destUri().toUtf8().constData()));
        if (m_info->m_type == FileOperationInfo::Move) {
            g_file_move(destFile.get()->get(), sourceFile.get()->get(), m_default_copy_flag, nullptr, nullptr, nullptr, nullptr);
        } else if (m_info->m_type == FileOperationInfo::Copy) {
            g_file_delete (destFile.get()->get(), nullptr, nullptr);
            //g_file_copy(destFile.get()->get(), sourceFile.get()->get(), m_default_copy_flag, nullptr, nullptr, nullptr, nullptr);
        }

        operationRollbackedOne(node->destUri(), node->uri());
    }

//    switch (node->state()) {
//    case FileNode::Handling: {
//        break;
//    }
//    case FileNode::Handled: {
//        //do not clear the dest file if ignored or overwrite or backuped.
//        if (node->responseType() != Other)
//            break;

//        if (node->isFolder()) {
//            auto children = node->children();
//            for (auto child : *children) {
//                rollbackNodeRecursively(child);
//            }
//            GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
//            g_file_delete(dest_file, nullptr, nullptr);
//            g_object_unref(dest_file);
//        } else {
//            GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
//            g_file_delete(dest_file, nullptr, nullptr);
//            g_object_unref(dest_file);
//        }
//        operationRollbackedOne(node->destUri(), node->uri());
//        break;
//    }
//    case FileNode::Cleared: {
//        switch (node->responseType()) {
//        case Other: {
//            if (node->isFolder()) {
//                GFile *src_file = g_file_new_for_uri(node->uri().toUtf8().constData());
//                g_file_make_directory(src_file, nullptr, nullptr);
//                g_object_unref(src_file);
//                auto children = node->children();
//                for (auto child : *children) {
//                    rollbackNodeRecursively(child);
//                }
//                //try deleting the dest directory
//                GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
//                g_file_delete(dest_file, nullptr, nullptr);
//                g_object_unref(dest_file);
//            } else {
//                GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
//                GFile *src_file = g_file_new_for_uri(node->uri().toUtf8().constData());
//                //"rollback"
//                GError *err = nullptr;
//                g_file_move(dest_file,
//                            src_file,
//                            m_default_copy_flag,
//                            nullptr,
//                            nullptr,
//                            nullptr,
//                            &err);
//                if (err) {
//                    qDebug()<<node->destUri();
//                    qDebug()<<node->uri();
//                    qDebug()<<err->message;
//                    g_error_free(err);
//                }
//                g_object_unref(dest_file);
//                g_object_unref(src_file);
//            }
//            operationRollbackedOne(node->destUri(), node->uri());
//            break;
//        }
//        default: {
//            //copy if err handle response type is valid.
//            if (node->isFolder()) {
//                GFile *src_file = g_file_new_for_uri(node->uri().toUtf8().constData());
//                g_file_make_directory(src_file, nullptr, nullptr);
//                g_object_unref(src_file);
//                auto children = node->children();
//                for (auto child : *children) {
//                    rollbackNodeRecursively(child);
//                }
//                GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
//                g_object_unref(dest_file);
//            } else {
//                GFile *dest_file = g_file_new_for_uri(node->destUri().toUtf8().constData());
//                GFile *src_file = g_file_new_for_uri(node->uri().toUtf8().constData());
//                //"rollback"
//                GError *err = nullptr;
//                g_file_copy(dest_file,
//                            src_file,
//                            m_default_copy_flag,
//                            nullptr,
//                            nullptr,
//                            nullptr,
//                            &err);
//                if (err) {
//                    qDebug()<<node->destUri();
//                    qDebug()<<node->uri();
//                    qDebug()<<err->message;
//                    g_error_free(err);
//                }
//                g_object_unref(dest_file);
//                g_object_unref(src_file);
//            }
//            operationRollbackedOne(node->destUri(), node->uri());
//            break;
//        }
//        }
//        break;
//    }
//    default: {
//        //make sure all nodes were rollbacked.
//        if (node->isFolder()) {
//            auto children = node->children();
//            for (auto child : *children) {
//                rollbackNodeRecursively(child);
//            }
//        }
//        break;
//    }
//    }
}

void FileMoveOperation::copyRecursively(FileNode *node)
{
    if (isCancelled())
        return;

    node->setState(FileNode::Handling);

    QString relativePath = node->getRelativePath();
    //FIXME: the smart pointers' deconstruction spends too much time.
    GFileWrapperPtr destRoot = wrapGFile(g_file_new_for_uri(m_dest_dir_uri.toUtf8().constData()));
    GFileWrapperPtr destFile = wrapGFile(g_file_resolve_relative_path(destRoot.get()->get(), relativePath.toUtf8().constData()));

    char *dest_file_uri = g_file_get_uri(destFile.get()->get());
    node->setDestUri(dest_file_uri);
    m_current_src_uri = node->uri();
    if (relativePath.isEmpty()) {
        qDebug() << "node relative path is empty";
        if (m_current_src_uri.startsWith("filesafe:///")) {
            m_current_dest_dir_uri = dest_file_uri;
            m_current_dest_dir_uri = FileUtils::urlEncode(m_current_dest_dir_uri);
        }
    } else {
        GFile *dest_parent = g_file_get_parent(destFile.get()->get());
        char *dest_dir_uri = g_file_get_uri(dest_parent);
        m_current_dest_dir_uri = dest_dir_uri;
        g_free(dest_dir_uri);
        g_object_unref(dest_parent);
    }
    g_free(dest_file_uri);
    QString destName = "";
    QString destDir = m_dest_dir_uri;
    g_autoptr(GFileInfo) srcFileInfo = nullptr;
    g_autoptr(GFile) srcFile = g_file_new_for_uri(m_current_src_uri.toUtf8().constData());
    srcFileInfo = g_file_query_info(srcFile, G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET
                                    , G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
fallback_retry:
    if (G_FILE_TYPE_SYMBOLIC_LINK == g_file_info_get_file_type(srcFileInfo)) {
        QString destFileUri = node->resolveDestFileUri(m_dest_dir_uri);
        destFileUri = FileUtils::urlEncode(destFileUri);
        node->setDestUri(destFileUri);
        GFileWrapperPtr destFileWrap = wrapGFile(g_file_new_for_uri(destFileUri.toUtf8().constData()));
        if (!copyLinkedFile(node, srcFileInfo, destFileWrap)) {
            goto fallback_retry;
        } else {
            return;
        }
    }

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
        auto realDestUri = node->resolveDestFileUri(m_dest_dir_uri);
        destFile = wrapGFile(g_file_new_for_uri(realDestUri.toUtf8().constData()));
        GFileWrapperPtr srcFile = wrapGFile(g_file_new_for_uri(m_current_src_uri.toUtf8().constData()));
        GError *err = nullptr;
        GError *error = nullptr;
        auto fileIconName = FileUtilsPrivate::getFileIconName(m_current_src_uri);
        auto destFileName = FileUtils::isFileDirectory(m_current_dest_dir_uri) ? nullptr : m_current_dest_dir_uri;
        // check if valid
        g_autofree gchar *relative_path = g_file_get_relative_path(srcFile.get()->get(), destFile.get()->get());
        if (relative_path) {
            node->setState(FileNode::Invalid);
            setHasError(true);
            invalidOperation(tr("Invalid move operation, cannot move a file into its sub directories."));
            invalidExited(tr("Invalid Operation."));
            FileOperationError except;
            except.errorType = ET_GIO;
            except.dlgType = ED_WARNING;
            except.srcUri = nullptr;
            except.destDirUri = nullptr;
            except.op = FileOpMove;
            except.title = tr("Invalid Operation");
            except.errorCode = G_IO_ERROR_INVAL;
            except.errorStr = tr("Invalid move operation, cannot move a file into its sub directories.");
            Q_EMIT errored(except);
            auto response = except.respCode;
            switch (response) {
            case Cancel:
                cancel();
                break;
            default:
                break;
            }
            return;
        }

        //NOTE: mkdir doesn't have a progress callback.
        Q_EMIT FileProgressCallback(m_current_src_uri, destFileName, fileIconName, node->size(), node->size());
        g_file_make_directory(destFile.get()->get(),getCancellable().get()->get(), &err);
        if (err) {
//            setHasError(true);
            FileOperationError except;
            if (err->code == G_IO_ERROR_CANCELLED) {
                return;
            }
            auto errWrapperPtr = GErrorWrapper::wrapFrom(err);
            int handle_type = prehandle(err);
            except.errorType = ET_GIO;
            except.op = FileOpMove;
            except.title = tr("Move file error");
            except.errorCode = err->code;
            except.errorStr = err->message;
            except.srcUri = m_current_src_uri;
            except.destDirUri = m_current_dest_dir_uri;
            except.isCritical = false;
            if (handle_type == Other) {
                auto typeData = Invalid;
                switch (err->code) {
                case G_IO_ERROR_EXISTS: {
                    except.dlgType = ED_CONFLICT;
                    Q_EMIT errored(except);
                    typeData = except.respCode;
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
                    except.destDirUri = realDestUri;
                    except.dlgType = ED_RENAME;
                    Q_EMIT errored(except);
                    typeData = except.respCode;
                    break;
                }
                default: {
                    except.dlgType = ED_WARNING;
                    Q_EMIT errored(except);
                    typeData = except.respCode;
                    if (typeData != Cancel) {
                        return;
                    }
                    break;
                }
                }
                // ignore multiple bounces
                if (except.errorCode == G_IO_ERROR_NOT_SUPPORTED) {
                    m_prehandle_hash.insert(err->code, IgnoreOne);
                }
                handle_type = typeData;
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
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                setHasError(true);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    return;
                }
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case IgnoreAll: {
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                setHasError(true);
                m_prehandle_hash.insert(err->code, IgnoreOne);
                return;
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    return;
                }
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case OverWriteOne: {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
                setHasError(true);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    auto result = udfCopyWarningDialog();
                    if (Cancel == result) {
                        cancel();
                    } else if (IgnoreOne == result || IgnoreAll == result) {
                        return;
                    }
                    break;
                }
                if (node->destBaseName().length() > 255 &&
                    m_dest_dir_uri.startsWith(QString("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展"))) {
                    QString msg = tr("The file name exceeds the limit");
                    Q_EMIT operationInfoMsgBox(msg);
                    if (m_is_long_name_file_operation) {
                        m_is_long_name_file_operation = false;
                    }
                    return;
                }
//                g_file_copy_attributes(srcFile.get()->get(),
//                                       destFile.get()->get(),
//                                       GFileCopyFlags(flags),
//                                       nullptr,
//                                       &error);
//                if (error) {
//                    qDebug() << __func__ << error->code << error->message;
//                    setHasError(true);
//                }else{
//                    setHasError(false);
//                }
//                g_error_free(error);

                //make dir has no overwrite
//                setHasError(false);
                break;
            }
            case OverWriteAll: {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
                setHasError(true);
                m_prehandle_hash.insert(err->code, OverWriteOne);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    m_is_udf_warning = true;
                    auto result = udfCopyWarningDialog();
                    if (Cancel == result) {
                        cancel();
                    } else if (IgnoreOne == result || IgnoreAll == result) {
                        return;
                    }
                    break;
                }
//                g_file_copy_attributes(srcFile.get()->get(),
//                                       destFile.get()->get(),
//                                       GFileCopyFlags(flags),
//                                       nullptr,
//                                       &error);
//                if (error) {
//                    qDebug() << __func__ << error->code << error->message;
//                    setHasError(true);
//                }else{
//                    setHasError(false);
//                }
//                g_error_free(error);
//                setHasError(false);
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
                    } else if ("" == endStr) {
                        node->setDestFileName(name);
                    }
                }
                node->resolveDestFileUri(m_dest_dir_uri);
                while (FileUtils::isFileExsit(node->destUri())) {
                    handleDuplicate(node);
                    node->resolveDestFileUri(m_dest_dir_uri);
                }
                g_object_unref(destFile.get());
//                destFile = wrapGFile(g_file_new_for_uri(node->destUri().toUtf8().constData()));
//                g_file_copy_attributes(srcFile.get()->get(),
//                                       destFile.get()->get(),
//                                       GFileCopyFlags(flags),
//                                       nullptr,
//                                       &error);
//                if (error) {
//                    qDebug() << __func__ << error->code << error->message;
//                }
//                g_error_free(error);

//                setHasError(false);
                goto fallback_retry;
            }
            case BackupAll: {
                node->setErrorResponse(BackupOne);
                m_prehandle_hash.insert(err->code, BackupOne);
                goto fallback_retry;
            }
            case Retry: {
                goto fallback_retry;
            }
            case RenameOne: {
                node->setErrorResponse(RenameOne);
                node->setDestFileName(except.respValue.value("newName").toString());
                // fixme: 目前无法undo，原文件名不能保留
                //setHasError(true);
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
            //node->setState(FileNode::Handled);
            //related bug#172512
            //g_file_copy_attributes(srcFile.get()->get(),
            //                       destFile.get()->get(),
            //                       G_FILE_COPY_ALL_METADATA,
            //                       nullptr,
            //                       &error);
            //if (error) {
            //    qDebug() << __func__ << error->code << error->message;
            //}
            //g_error_free(error);
            node->setState(FileNode::Handled);
        }

        fileIconName = FileUtilsPrivate::getFileIconName(m_current_src_uri);
        destFileName = FileUtils::isFileDirectory(m_current_dest_dir_uri) ? nullptr : m_current_dest_dir_uri;
        //assume that make dir finished anyway
        m_current_offset += node->size();
        Q_EMIT FileProgressCallback(m_current_src_uri, destFileName, fileIconName, m_current_offset, m_total_size);
        Q_EMIT operationProgressedOne(node->uri(), node->destUri(), node->size());
        if (SaveOne == node->responseType() || SaveAll == node->responseType()) {
            m_dest_dir_uri = destDir;
        }
        for (auto child : *(node->children())) {
            copyRecursively(child);
        }

        //related bug#172512
        if (node->state() == FileNode::Handled || node->responseType() == OverWriteOne || node->responseType() == OverWriteAll) {
            g_file_copy_attributes(srcFile.get()->get(),
                                   destFile.get()->get(),
                                   m_parent_flags,
                                   nullptr,
                                   &error);
        }

        if (error) {
            g_error_free(error);
            qDebug() << __func__ << error->code << error->message;
        }
    } else {
        GError *err = nullptr;
        GFileWrapperPtr sourceFile = wrapGFile(g_file_new_for_uri(node->uri().toUtf8().constData()));
        auto realDestUri = node->resolveDestFileUri(m_dest_dir_uri);
        destFile = wrapGFile(g_file_new_for_uri(realDestUri.toUtf8().constData()));
        if (SaveOne == node->responseType() || SaveAll == node->responseType()) {
           GFileWrapperPtr parentPtr = FileUtils::getFileParent(destFile);
           auto path = g_file_peek_path(parentPtr.get()->get());
           QDir destFileParentPath(path);
           if (!destFileParentPath.exists()) {
               destFileParentPath.mkpath(path);
           }
        }
        FileCopy fileCopy (node->uri(), realDestUri, m_default_copy_flag,
                           getCancellable().get()->get(),
                           GFileProgressCallback(progress_callback),
                           this,
                           &err);
        fileCopy.setParentFlags(m_parent_flags);
        fileCopy.connect(this, &FileOperation::operationPause, &fileCopy, &FileCopy::pause, Qt::DirectConnection);
        fileCopy.connect(this, &FileOperation::operationResume, &fileCopy, &FileCopy::resume, Qt::DirectConnection);
        fileCopy.connect(this, &FileOperation::operationCancel, &fileCopy, &FileCopy::cancel, Qt::DirectConnection);
        if (m_is_pause) fileCopy.pause();
        fileCopy.run();

        if (err) {
            switch (err->code) {
            case G_IO_ERROR_CANCELLED:
                return;
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
            default:
                break;
            }

            FileOperationError except;
            auto errWrapperPtr = GErrorWrapper::wrapFrom(err);
            int handle_type = prehandle(err);
            except.isCritical = true;
            except.errorType = ET_GIO;
            except.errorCode = err->code;
            except.errorStr = err->message;
            except.op = FileOpMove;
            except.title = tr("Create file error");
            except.srcUri = m_current_src_uri;
            except.destDirUri = m_current_dest_dir_uri;

            //fix bug#121093, paste deleted file issue
            if (err->code == G_IO_ERROR_PERMISSION_DENIED) {
                except.errorStr = tr("Cannot opening file, permission denied!");
            }else if (err->code == G_IO_ERROR_NOT_FOUND) {
                except.errorStr = tr("File:%1 was not found.").arg(except.srcUri);
            }

            if (handle_type == Other) {
                auto typeData = Invalid;
                switch (err->code) {
                case G_IO_ERROR_EXISTS: {
                    except.dlgType = ED_CONFLICT;
                    Q_EMIT errored(except);
                    typeData = except.respCode;
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
                    except.destDirUri = realDestUri;
                    except.dlgType = ED_RENAME;
                    Q_EMIT errored(except);
                    typeData = except.respCode;
                    break;
                }
                default: {
                    except.dlgType = ED_WARNING;
                    Q_EMIT errored(except);
                    typeData = except.respCode;
                    break;
                }
                }
                handle_type = typeData;
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

            GError *nodeErr = nullptr;
            //handle.
            node->setState(FileNode::Handling);
            switch (handle_type) {
            case IgnoreOne: {
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                setHasError(true);
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case IgnoreAll: {
                node->setState(FileNode::Unhandled);
                node->setErrorResponse(IgnoreOne);
                setHasError(true);
                m_prehandle_hash.insert(err->code, IgnoreOne);
                if (m_is_long_name_file_operation) {
                    m_is_long_name_file_operation = false;
                }
                return;
            }
            case OverWriteOne: {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
                setHasError(true);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
                    auto result = udfCopyWarningDialog();
                    if (Cancel == result) {
                        cancel();
                    } else if (IgnoreOne == result || IgnoreAll == result) {
                        return;
                    }
                    break;
                }
                FileCopy fileCopy (node->uri(), realDestUri, GFileCopyFlags(m_default_copy_flag | G_FILE_COPY_OVERWRITE),
                                   getCancellable().get()->get(),
                                   GFileProgressCallback(progress_callback),
                                   this,
                                   &nodeErr);
                fileCopy.setParentFlags(m_parent_flags);
                fileCopy.connect(this, &FileOperation::operationPause, &fileCopy, &FileCopy::pause, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationResume, &fileCopy, &FileCopy::resume, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationCancel, &fileCopy, &FileCopy::cancel, Qt::DirectConnection);
                if (m_is_pause) fileCopy.pause();
                fileCopy.run();
//                node->setErrorResponse(OverWriteOne);
                if (nodeErr){
                    node->setErrorResponse(Invalid);
                    g_error_free(nodeErr);
                }else{
                }
                break;
            }
            case OverWriteAll: {
                node->setState(FileNode::Invalid);
                node->setErrorResponse(OverWriteOne);
                setHasError(true);
//                g_file_copy(sourceFile.get()->get(),
//                            destFile.get()->get(),
//                            GFileCopyFlags(m_default_copy_flag | G_FILE_COPY_OVERWRITE),
//                            getCancellable().get()->get(),
//                            GFileProgressCallback(progress_callback),
//                            this,
//                            nullptr);
                if (!m_is_udf_warning && m_is_udf_burn_work) {
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
                FileCopy fileCopy (node->uri(), realDestUri, GFileCopyFlags(m_default_copy_flag | G_FILE_COPY_OVERWRITE),
                                   getCancellable().get()->get(),
                                   GFileProgressCallback(progress_callback),
                                   this,
                                   &nodeErr);
                fileCopy.setParentFlags(m_parent_flags);
                fileCopy.connect(this, &FileOperation::operationPause, &fileCopy, &FileCopy::pause, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationResume, &fileCopy, &FileCopy::resume, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationCancel, &fileCopy, &FileCopy::cancel, Qt::DirectConnection);
                if (m_is_pause) fileCopy.pause();
                fileCopy.run();
                //node->setState(FileNode::Handled);
//                node->setErrorResponse(OverWriteAll);
                m_prehandle_hash.insert(err->code, OverWriteAll);
                if (nodeErr){
                    node->setErrorResponse(Invalid);
                    g_error_free(nodeErr);
                }else{
                }
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
                while (FileUtils::isFileExsit(node->destUri())) {
                    handleDuplicate(node);
                    node->resolveDestFileUri(m_dest_dir_uri);
                }
                auto handledDestFileUri = node->resolveDestFileUri(m_dest_dir_uri);
                auto handledDestFile = wrapGFile(g_file_new_for_uri(handledDestFileUri.toUtf8()));
                if (handledDestFileUri.length() > 255 &&
                    m_dest_dir_uri.startsWith(QString("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展"))) {
                    QString msg = tr("The file name exceeds the limit");
                    Q_EMIT operationInfoMsgBox(msg);
                    if (m_is_long_name_file_operation) {
                        m_is_long_name_file_operation = false;
                    }
                    return;
                }
                FileCopy fileCopy (node->uri(), realDestUri, GFileCopyFlags(m_default_copy_flag | G_FILE_COPY_BACKUP),
                                   getCancellable().get()->get(),
                                   GFileProgressCallback(progress_callback),
                                   this,
                                   &nodeErr);
                fileCopy.setParentFlags(m_parent_flags);
                fileCopy.connect(this, &FileOperation::operationPause, &fileCopy, &FileCopy::pause, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationResume, &fileCopy, &FileCopy::resume, Qt::DirectConnection);
                fileCopy.connect(this, &FileOperation::operationCancel, &fileCopy, &FileCopy::cancel, Qt::DirectConnection);
                if (m_is_pause) fileCopy.pause();
                fileCopy.run();
                if (nodeErr) {
                    node->setErrorResponse(Invalid);
                    g_error_free (nodeErr);
                } else {
                    // 设置node的状态为handled用于后续删除原文件的流程
                    node->setState(FileNode::Handled);
                }
                //node->setState(FileNode::Handled);
//                node->setErrorResponse(BackupOne);
                break;
            }
            case BackupAll: {
                node->setErrorResponse(BackupOne);
                m_prehandle_hash.insert(err->code, BackupOne);
                goto fallback_retry;
                break;
            }
            case Retry: {
                goto fallback_retry;
            }
            case RenameOne: {
                node->setErrorResponse(RenameOne);
                node->setDestFileName(except.respValue.value("newName").toString());
                // fixme: 目前无法undo，原文件名不能保留
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
                node->setErrorResponse(Cancel);
                cancel();
                break;
            }
            default:
                break;
            }
        }else{
            node->setState(FileNode::Handled);
        }
        if (SaveOne == node->responseType() || SaveAll == node->responseType()) {
            m_dest_dir_uri = destDir;
        }
//        fileSync(node->uri(), realDestUri);
        if(node->uri().endsWith(".dsps") && realDestUri.endsWith(".dsps")){
            m_srcUrisOfCopyDspsFiles.append(FileUtils::urlDecode(node->uri()));
            m_destUrisOfCopyDspsFiles.append(FileUtils::urlDecode(realDestUri));
        }
        m_current_offset += node->size();
        auto fileIconName = FileUtilsPrivate::getFileIconName(m_current_src_uri);
        auto destFileName = FileUtils::isFileDirectory(node->destUri()) ? nullptr : node->destUri();
        Q_EMIT FileProgressCallback(node->uri(), destFileName, fileIconName, m_current_offset, m_total_size);
        Q_EMIT operationProgressedOne(node->uri(), node->destUri(), node->size());
    }
    destFile.reset();
    destRoot.reset();
}

void FileMoveOperation::deleteRecursively(FileNode *node)
{
    qDebug() << "deleteRecursively:"<<node->uri()<<isCancelled()<<hasError();
    if (isCancelled()/* || hasError()*/)
        return;

    // 在外部判断可以节约时间
//    if (m_info->m_type != FileOperationInfo::Move) {
//        return;
//    }

    g_autoptr(GFile) file = g_file_new_for_uri(node->uri().toUtf8().constData());
    if (node->state() == FileNode::Handled || node->responseType() == OverWriteOne || node->responseType() == OverWriteAll) {
        if (node->isFolder()) {
            for (auto child : *(node->children())) {
                deleteRecursively(child);
            }
            g_file_delete(file, /*getCancellable().get()->get()*/nullptr, nullptr);
        } else {
            // targetmove替换时原文件也需要删除，需要注意替换可能会失败的情况，这里只是一层保险，
            // 在overwrite操作时应该判断是否overwrite成功，如果不成功需要设置response type为invalid
            if (FileUtils::isFileExsit(node->destUri())) {
                g_file_delete(file, /*getCancellable().get()->get()*/nullptr, nullptr);
            }
        }
    }

    operationAfterProgressedOne(node->uri());
}

void FileMoveOperation::moveForceUseFallback()
{
    if (isCancelled())
        return;

    Q_EMIT operationRequestShowWizard();
    m_reporter = new FileNodeReporter;
    connect(m_reporter, &FileNodeReporter::nodeFound, this, &FileMoveOperation::operationPreparedOne);

    //FIXME: total size should not compute twice. I should get it from ui-thread.
    goffset *total_size = new goffset(0);

    QList<FileNode*> nodes;
    bool hasFolder = false;
    for (auto uri : m_src_uris) {
        FileNode *node = new FileNode(uri, nullptr, m_reporter);
        if (uri.startsWith("trash:///")) {
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
        nodes<<node;
        if (node->isFolder())
            hasFolder = true;
    }
    operationPrepared();

    m_total_size = *total_size;
    delete total_size;

    m_srcUrisOfCopyDspsFiles.clear();
    m_destUrisOfCopyDspsFiles.clear();

    for (auto node : nodes) {
        copyRecursively(node);
    }
    operationProgressed();

    if (m_move_action == Qt::TargetMoveAction) {
        m_info.get()->m_type = FileOperationInfo::Move;
        m_info.get()->m_opposite_type = FileOperationInfo::Move;
        for (auto node : nodes) {
            deleteRecursively(node);
        }
    } else if(m_move_action == Qt::MoveAction){
        //only when action is copy can delete when ctrl+z
        //当前文件夹替换其实做的是合并操作，撤销操作不能删除文件夹，有丢失文件的风险
        if (m_is_same_fs && ! hasFolder){
            m_info.get()->m_type = FileOperationInfo::Copy;
            m_info.get()->m_opposite_type = FileOperationInfo::Delete;
        }
    } else{
        m_info.get()->m_type = FileOperationInfo::Copy;
        m_info.get()->m_opposite_type = FileOperationInfo::Delete;
    }

    if (isCancelled())
        Q_EMIT operationStartRollbacked();

    for (auto file : nodes) {
        if (isCancelled()) {
            rollbackNodeRecursively(file);
        }
    }

    m_info.get()->m_src_uris.clear();
    m_info.get()->m_dest_uris.clear();
    for (auto node : nodes) {
        m_info.get()->m_src_uris<<node->uri();
        m_info.get()->m_dest_uris<<node->destUri();
        delete node;
    }

    nodes.clear();
}

void FileMoveOperation::moveForceUseFallback(FileNode* node)
{
    if (isCancelled() || nullptr == node)
        return;

    operationPrepared();

    copyRecursively(node);

    if (isCancelled()) {
        Q_EMIT operationStartRollbacked();
    }

    //如果是同一个文件系统的文件，直接做移动操作
    //related change with bug#164742
    if (m_is_same_fs && m_move_action == Qt::MoveAction){
        m_move_action = Qt::TargetMoveAction;
        setCopyMove(false);
    }

    if (m_move_action == Qt::TargetMoveAction) {
        m_info->m_opposite_type = FileOperationInfo::Move;
        deleteRecursively(node);
    } else if (!m_is_same_fs) {
        setCopyMove(true);
    }

//    fix bux 172591,此处若是设置Handel，后续无法区分rollback场景
//    node->setState(FileNode::Handled);

    if (isCancelled()) {
        rollbackNodeRecursively(node);
    }
}

bool FileMoveOperation::isValid()
{
    int index = 0;
    bool isInvalid = false;
    for (auto srcUri : m_src_uris) {
        auto srcFile = wrapGFile(g_file_new_for_uri(srcUri.toUtf8().constData()));
        auto destFile = wrapGFile(g_file_new_for_uri(m_dest_dir_uri.toUtf8().constData()));
        auto parentFile = wrapGFile(g_file_get_parent(srcFile.get()->get()));
        if (g_file_equal(destFile.get()->get(), parentFile.get()->get())) {
            m_src_uris.removeAt(index);
            --index;
        }
        //BUG: some special basename like test and test2, will lead the operation invalid.
        /*
        if (m_dest_dir_uri.contains(srcUri)) {
            isInvalid = true;
            invalidOperation(tr("Invalid move operation, cannot move a file into its sub directories."));
        }
        */
        //FIXME: find if destUriDirFile is srcFile's child.
        //it will call G_IO_ERROR_INVALID_FILENAME
        ++index;
    }
    //can not move StandardPath to any dir. see:#87912
    if (FileUtils::containsStandardPath(m_src_uris)) {
        isInvalid = true;
    }
    if (isInvalid)
        invalidExited(tr("Invalid Operation."));
    return !isInvalid;
}

void FileMoveOperation::run()
{
    Q_EMIT operationStarted();

    if (m_is_sort && m_is_search) {
        m_src_uris = sortUris(m_src_uris, m_sort_type);
        m_info.get()->m_is_search = true;
    }

    if (hook_check_operation_valid) {
        if (!hook_check_operation_valid(m_src_uris, m_dest_dir_uri, FILE_OPERATION_MOVE)) {
            setHasError(true);
            cancel();
            Q_EMIT operationFinished();
            return;
        }
    }

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
            if (isCancelled())
               return;
            //should block and wait for other object prepared.
            setCopyMove(true);
            setAction(Qt::CopyAction);
            moveForceUseFallback();
            goto end;
        }
    }
#endif

start:
    if (!isValid()) {
        FileOperationError except;
        except.errorType = ET_GIO;
        except.dlgType = ED_WARNING;
        except.srcUri = nullptr;
        except.destDirUri = nullptr;
        except.op = FileOpMove;
        except.title = tr("File delete error");
        except.errorCode = G_IO_ERROR_INVAL;
        except.errorStr = tr("Invalid Operation");
        Q_EMIT errored(except);
        auto response = except.respCode;
        switch (response) {
        case Retry:
            goto start;
        case Cancel:
            cancel();
            break;
        default:
            break;
        }
        goto end;
    }

    if (isCancelled())
        return;

    //should block and wait for other object prepared.
    move();
//    if (!m_force_use_fallback) {
//        move();
//    }

end:
#ifdef LINGMO_UDF_BURN
    if (mHelper->isUnixCDDevice() && !isCancelled()) {
        if(!mHelper->discWriteOperation(m_burn_uris, m_dest_dir_uri)) {
            FileOperationError except;
            except.errorType = ET_CUSTOM;
            except.op = FileOpMove;
            except.title = tr("Move file error");
            except.srcUri = m_src_uris.first();
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

bool FileMoveOperation::copyLinkedFile(FileNode *node, GFileInfo *info, GFileWrapperPtr file)
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
        except.op = FileOpMove;
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

        node->setState(FileNode::Handling);
        switch (handle_type) {
        case IgnoreOne: {
            setHasError(true);
            node->setErrorResponse(IgnoreOne);
            return true;
        }
        case IgnoreAll: {
            setHasError(true);
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
            m_prehandle_hash.insert(err->code, OverWriteOne);
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

ExceptionResponse FileMoveOperation::udfCopyWarningDialog()
{
    FileOperationError except;
    ExceptionResponse typeData = Invalid;
    except.errorType = ET_CUSTOM;
    except.op = FileOpMove;
    except.title = tr("File move error");
    except.srcUri = m_src_uris.first();
    except.errorStr = tr("Burning does not support replacement");
    except.destDirUri = m_dest_dir_uri;
    except.dlgType = ED_WARNING;
    Q_EMIT errored(except);
    typeData = except.respCode;
    return typeData;
}

bool FileMoveOperation::saveAsOtherPath()
{
    QString destUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展";
    if (!FileUtils::isFileExsit(destUri)) {
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        dir.mkdir("扩展");
        QDBusInterface iface ("com.lingmo.file.system.fuse","/com/lingmo/file/system/fuse","com.lingmo.file.system.fuse",QDBusConnection::systemBus());
        QDBusReply<bool> reply = iface.call("SetSetings", "true", dir.path());
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

void FileMoveOperation::cancel()
{
    setHasError(true);
    FileOperation::cancel();
    if (m_reporter)
        m_reporter->cancel();
}
