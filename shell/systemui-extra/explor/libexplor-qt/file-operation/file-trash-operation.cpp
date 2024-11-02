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

#include "file-trash-operation.h"
#include "file-operation-manager.h"
#include "file-enumerator.h"

#include <QProcess>
#include <file-info-job.h>
#include <file-info.h>
#include <QDateTime>
#include <QStandardPaths>

#define TRASH_TIME "trash-time"

using namespace Peony;

FileTrashOperation::FileTrashOperation(QStringList srcUris, QObject *parent) : FileOperation (parent)
{
    m_src_uris = srcUris;
    m_info = std::make_shared<FileOperationInfo>(srcUris, "trash:///", FileOperationInfo::Trash);
    m_total_count = m_src_uris.length ();
}

void FileTrashOperation::run()
{
    Q_EMIT operationStarted();
    Peony::ExceptionResponse response = Invalid;

    if (m_is_search) {
        m_info.get()->m_is_search = true;
    }

    QSet<QString> trashBefore;
    FileEnumerator e;
    e.setEnumerateDirectory("trash:///");
    e.enumerateSync();
    QStringList lsBefore = e.getChildrenUris ();
    trashBefore = lsBefore.toSet ();

    //add file_count para for file trash progress
    quint64 file_count = 0;
    //all file total size, should use changed the calculate way
    quint64 total_size = 0;
    const quint64 ONE_GIB_SIZE = 1024*1024*1024;
    for (auto src : m_src_uris) {
        // pre-check for trash special directory
        if (src == "file:///data/home" || src == "file:///data/usershare" ||
                src == "file:///data/root" || src == "file:///home" ||
                FileUtils::isStandardPath(src) || src == "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation)) {
            FileOperationError except;
            except.srcUri = src;
            except.destDirUri = tr("trash:///");
            except.isCritical = true;
            except.op = FileOpTrash;
            except.title = tr("Trash file error");
            except.errorType = ET_GIO;
            except.dlgType = ED_WARNING;
            except.errorStr = tr("Invalid Operation! Can not trash \"%1\".").arg(FileUtils::urlDecode(src));
            errored(except);
            cancel();
        }

        if (isCancelled())
            break;

        //need count file total size, more than 10GB file should delete forever, fix bug#101592
        total_size += FileUtils::getFileTotalSize(src);;

        Q_EMIT operationPreparedOne (src, 1);
        ++file_count;
    }

    FileOperationError except;
    except.srcUri = "";
    except.destDirUri = tr("trash:///");
    except.isCritical = true;
    except.op = FileOpTrash;
    except.title = tr("Trash file error");
    except.errorType = ET_GIO;

    //file total size more than 10G, not trash but delete, task#56444, bug#88871, bug#88894
    qDebug() <<"total_size in trash operation:" <<total_size<<ONE_GIB_SIZE;
    if (total_size/10 > ONE_GIB_SIZE){
        except.dlgType = ED_NOT_SUPPORTED;
        except.errorCode = G_IO_ERROR_NOT_SUPPORTED;
        except.title = "";
        //task #155670,155671 improve delete file permanently message
        except.errorStr = QObject::tr("The file is too large to be moved to the recycle bin. "
                                      "Do you want to permanently delete it?");
        if (m_total_count >1){
            except.errorStr = QObject::tr("These files are too large to be moved to the recycle bin. "
                                          "Do you want to permanently delete these %1 files?").arg(m_total_count);
        }

        Q_EMIT errored(except);

        auto responseType = except.respCode;
        auto responseData = responseType;

        if (response != Invalid)
            responseData = response;

        switch (responseData) {
        case Force:
        case ForceAll: {
            deleteRequest(m_src_uris);
            break;
        }
        default:
            cancel();
            break;
        }
    } else {
        int curSize = 0;
        for (auto src : m_src_uris) {
            OperatorThreadPause();
            if (isCancelled())
                break;

            // cache file info
            m_src_infos<<FileInfo::fromUri(src);

retry:
            GError *err = nullptr;
            auto srcFile = wrapGFile(g_file_new_for_uri(src.toUtf8().constData()));

            g_file_trash(srcFile.get()->get(), getCancellable().get()->get(), &err);
            if (err) {
                if (response == IgnoreAll) {
                    g_error_free(err);
                    Q_EMIT FileProgressCallback("trash:///", src, "", ++curSize, file_count);
                    continue;
                }

                // ref the error, released after error handling.
                auto refPtr = GErrorWrapper::wrapFrom(err);

                except.errorCode = err->code;
                except.errorStr = err->message;

                if (G_IO_ERROR_EXISTS == err->code) {
                    except.dlgType = ED_CONFLICT;
                    Q_EMIT errored(except);
                    auto responseType = except.respCode;
                    auto responseData = responseType;
                    switch (responseData) {
                    case Retry:
                        goto retry;
                    case Cancel:
                        cancel();
                        break;
                    case IgnoreAll:
                        response = IgnoreAll;
                        break;
                    default:
                        break;
                    }
                } else {
                    if (err->code == G_IO_ERROR_NOT_SUPPORTED) {
                        except.dlgType = ED_NOT_SUPPORTED;
                        //task #155670,155671 improve delete file permanently message
                        except.errorStr = QObject::tr("Are you sure you want to permanently delete this file?"
                                                      " Once deletion begins, "
                                                      "the file will not be recoverable.");
                        if (m_total_count >1){
                            except.errorStr = QObject::tr("Are you sure you want to permanently delete these %1 files?"
                                                          " Once deletion begins, "
                                                          "these file will not be recoverable.").arg(m_total_count);
                        }
                    } else if (err->code == G_IO_ERROR_FILENAME_TOO_LONG) {
                        GError *error = nullptr;
                        char *orig_path = g_file_get_path(srcFile.get()->get());
                        char *basename = g_file_get_basename(srcFile.get()->get());
                        QString trashDir = QString(g_get_user_data_dir()) + "/Trash/files";
                        GFile *trash = g_file_new_for_path(trashDir.toUtf8().constData());
                        GFile *dest_file = g_file_resolve_relative_path(trash, basename);
                        g_object_unref(trash);
                        g_file_move(srcFile.get()->get(), dest_file, G_FILE_COPY_NOFOLLOW_SYMLINKS, nullptr, nullptr, nullptr, &error);
                        if (error && error->code == G_IO_ERROR_EXISTS) {
                            except.errorCode = error->code;
                            except.dlgType = ED_WARNING;
                            except.errorStr = tr("An unmanageable conflict exists. Please check the recycle bin.");
                            Q_EMIT errored(except);
                            g_error_free(error);
                        }
                        g_file_set_attribute_string(dest_file, "metadata::orig-path", orig_path, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
                        g_object_unref(dest_file);
                        if (orig_path) {
                            g_free(orig_path);
                        }
                        if (basename) {
                            g_free(basename);
                        }

                        response = IgnoreOne;
                    } else {
                        except.dlgType = ED_WARNING;
                    }
                    if (response == Invalid)
                        Q_EMIT errored(except);
                    auto responseType = except.respCode;
                    auto responseData = responseType;
                    if (response != Invalid)
                        responseData = response;
                    switch (responseData) {
                    case Retry:
                        goto retry;
                    case OverWriteOne: {
                        GError *err1 = nullptr;
                        g_file_delete(srcFile.get()->get(), getCancellable().get()->get(), &err1);
                        if (err1) {
                            g_error_free(err1);
                        }
                        break;
                    }
                    case OverWriteAll: {
                        response = OverWriteOne;
                        goto retry;
                    }
                    case Cancel:
                        cancel();
                        break;
                    case IgnoreAll:
                        response = IgnoreAll;
                        break;
                    case ForceAll:
                        response = ForceAll;
                    case Force:
                        forceDelete(src);
                        break;
                    default:
                        break;
                    }
                }
            } else {
                Q_EMIT remoteFileEvent(102, src, nullptr);
                // fileinfo关联删除时间
                quint64 time = QDateTime::currentMSecsSinceEpoch();
                auto info = FileInfo::fromUri(src);
                info.get()->setProperty(TRASH_TIME, time);
            }

            Q_EMIT FileProgressCallback("trash:///", src, "", ++curSize, file_count);
        }

        // fix dest uris in trash in FileOperationInfo
        // fileinfo关联被删除的时间，然后枚举trash中displayname相同的文件，对比时间差最近的文件作为待恢复的文件
        if (!isCancelled()) {
            QSet<QString> trashAfter;
            FileEnumerator e;
            e.setEnumerateDirectory("trash:///");
            e.enumerateSync();
            QStringList lsAfter = e.getChildrenUris ();
            trashBefore = lsAfter.toSet ();//QSet<QString>(lsAfter.begin (), lsAfter.end ());

            QStringList diff = (trashAfter - trashBefore).values ();

            if (diff.length () == m_src_uris.length ()) {
                m_info.get()->m_dest_uris.clear();
                m_info.get()->m_dest_uris<<diff;
            } else {
                QStringList destUris;
                for (auto trashUri : diff) {
                    auto trashFileInfo = FileInfo::fromUri (trashUri);
                    FileInfoJob trashInfoJob(trashFileInfo);
                    trashInfoJob.querySync();
                    for (auto info : m_src_infos) {
                        if (info.get()->displayName() == trashFileInfo.get()->displayName()) {
                            // allow 5 seconds deviation
                            int a = trashFileInfo.get()->getDeletionDateUInt64();
                            int b = info.get()->property(TRASH_TIME).toUInt();
                            int abs = qAbs(a - b);
                            if (abs < 5000) {
                                destUris<<trashFileInfo.get()->uri();
                            }
                        }
                    }
                }
                if (!destUris.isEmpty()) {
                    m_info.get()->m_dest_uris.clear();
                    m_info.get()->m_dest_uris<<destUris;
                }
            }
        }
    }

    Q_EMIT operationFinished();
    //notifyFileWatcherOperationFinished();
}

void FileTrashOperation::forceDelete(QString uri)
{
    qDebug() << "force delete:" << uri;
    FileNode *node = new FileNode(uri, nullptr, nullptr);
    node->findChildrenRecursively();
    deleteRecursively(node);

    if (node)  delete node;
}


void FileTrashOperation::deleteRecursively(FileNode *node)
{
    if (isCancelled())
        return;

    g_autoptr(GFile) file = g_file_new_for_uri(FileUtils::urlEncode(node->uri()).toUtf8().constData());
    if (node->isFolder()) {
        for (auto child : *(node->children())) {
            deleteRecursively(child);
        }
        GError *err = nullptr;
        g_file_delete(file, getCancellable().get()->get(), &err);
        if (err) {
            qDebug() << "force delete folder:" << node->uri() << " error: " << err->message;
            g_error_free(err);
        }
    } else {
        GError *err = nullptr;
        g_file_delete(file, getCancellable().get()->get(), &err);
        if (err) {
            qDebug() << "force delete file:" << node->uri() << " error: " << err->message;
            g_error_free(err);
        }
    }
}

void FileTrashOperation::setSearchOperation(const bool &isSearch)
{
    if (isSearch != m_is_search) {
        m_is_search = isSearch;
    }
}

void FileTrashOperation::setErrorMessage(GError** err)
{

}
