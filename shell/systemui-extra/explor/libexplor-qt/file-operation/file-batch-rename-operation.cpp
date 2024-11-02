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
 * Authors: ZhuoAn Li <lizhuoan@kylinos.cn>
 *
 */

#include "file-batch-rename-operation.h"
#include "file-operation-manager.h"
#include "file-utils.h"
#include "file-info.h"
#include <gio/gdesktopappinfo.h>
#include <glib/gprintf.h>
#include <global-settings.h>
#include <QUrl>

#include <QProcess>
#include <QFileInfo>

static QString set_desktop_name (QString file, QString& name, GError** error);

using namespace Peony;

FileBatchRenameOperation::FileBatchRenameOperation(QStringList uris, QString newName)
{
    m_uris = uris;
    m_new_name = newName;

    QString destUri = FileUtils::getParentUri(FileUtils::urlEncode(uris.first()));
    QStringList destUris;
    if (destUri != nullptr) {
        for (QString uri : uris) {
            destUri = FileUtils::urlEncode(FileUtils::urlDecode(uri) + "/" + m_new_name);
            m_old_names.append(FileUtils::getFileDisplayName(uri));
            destUris.append(destUri);
        }
    } else {
        qDebug() << "rename operation get dest uri error";
    }

    m_info = std::make_shared<FileOperationInfo>(uris, destUris, FileOperationInfo::BatchRename);
}

FileBatchRenameOperation::~FileBatchRenameOperation()
{

}

void FileBatchRenameOperation::run()
{
    QString destUri;
    Q_EMIT operationStarted();

    if (m_new_name == "/" || m_new_name == "." || !nameIsValid(m_new_name)) {
        FileOperationError except;
        except.srcUri = m_uris.first();
        except.errorType = ET_GIO;
        except.op = FileOpRename;
        except.dlgType = ED_WARNING;
        except.title = tr("File Rename error");
        except.errorStr = tr("Invalid file name %1%2%3 .").arg("\“").arg(m_new_name).arg("\”");

        Q_EMIT errored(except);

        Q_EMIT operationFinished();
        return;
    } else if (m_new_name.startsWith(".")) {
        auto showHidden = GlobalSettings::getInstance()->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
        if (! showHidden)
        {
            FileOperationError except;
            except.srcUri = m_uris.first();
            except.errorType = ET_GIO;
            except.op = FileOpRenameToHideFile;
            except.dlgType = ED_WARNING;
            except.title = tr("File Rename warning");
            except.errorStr = tr("Are you sure to hidden these files?");

            Q_EMIT errored(except);

            //fix bug#161394, support cancel rename operation
            if (except.respCode == Cancel) {
                cancel();
                setHasError(true);
                //未做重命名操作，恢复之前的目标文件，仍然选中原来的文件
                getOperationInfo().get()->m_dest_dir_uri = getOperationInfo().get()->sources().first();
                Q_EMIT operationFinished();
                return;
            }else{
                //fix bug#174512, can not hide file immediately
                qDebug() << "Q_EMIT updateHiddenFile："<<m_new_name;
                Q_EMIT GlobalSettings::getInstance()->updateHiddenFile(m_new_name);
            }
        }
    }
    m_total_size = m_uris.count();
    m_current_offset = 0;
    for (auto uri : m_uris) {
        if (isCancelled())
            break;
        Q_EMIT operationPreparedOne (uri, 0);
    }

    for (QString uri :m_uris) {
        OperatorThreadPause();
        if (isCancelled())
            break;
        QString oldName = FileUtils::getFileDisplayName(uri);
        QString newName = m_new_name;
        auto fileIconName = FileUtilsPrivate::getFileIconName(FileUtils::urlEncode(uri));
        std::shared_ptr<FileInfo> fileinfo = FileInfo::fromUri(uri);
        if(fileinfo && !fileinfo->isDir()){
//            bool showFileExtension = Peony::GlobalSettings::getInstance()->isExist(SHOW_FILE_EXTENSION)?
//                        Peony::GlobalSettings::getInstance()->getValue(SHOW_FILE_EXTENSION).toBool():true;
            QString oldSuffix = getFileExtensionOfFile(oldName);
            QString newSuffix = getFileExtensionOfFile(newName);
            if (newSuffix.isEmpty()) {
                newName = newName.append(".").append(oldSuffix);
            } else if ((oldSuffix != newSuffix) && !oldSuffix.isEmpty()) {
                newName = newName.replace(newSuffix, oldSuffix);
            }
        } else if (fileinfo->isDir()) {
            QString newSuffix = getFileExtensionOfFile(newName);
            if (!newSuffix.isEmpty()) {
                newName.chop(newSuffix.length() + 1);
            }
        }
        auto file = wrapGFile(g_file_new_for_uri(FileUtils::urlEncode(uri).toUtf8().constData()));
        auto info = wrapGFileInfo(g_file_query_info(file.get()->get(), "*",
                                  G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                  getCancellable().get()->get(),
                                  nullptr));

        bool is_local_desktop_file = false;
        QUrl url = uri;
        QString oldDesktopName = "";
        //change the content of .desktop file;
        if (url.isLocalFile() && uri.endsWith(".desktop")) {
            GDesktopAppInfo *desktop_info = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
            if (G_IS_DESKTOP_APP_INFO(desktop_info)) {
                bool is_executable = g_file_test (url.path().toUtf8().constData(), G_FILE_TEST_IS_EXECUTABLE);
                is_local_desktop_file = is_executable;
                if (is_executable) {
                    g_autoptr (GError) error = nullptr;
                    oldDesktopName = set_desktop_name (url.path (), newName, &error);
                    if (error) {
                        qDebug() << "set_desktop_name error code: " << error->code << " msg:" << error->message;
                    }

                    //set attributes again.
                    g_file_set_attributes_from_info(file.get()->get(), info.get()->get(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &error);
                    if (error) {
                        qDebug() << "g_file_set_attributes_from_info error code: " << error->code << " msg: " << error->message;
                    }
                }
                g_object_unref(desktop_info);
            }
        }

        QString targetName = newName;
        if (is_local_desktop_file) {
            targetName = newName+".desktop";
        }

        auto parent = FileUtils::getFileParent(file);
        auto newFile = FileUtils::resolveRelativePath(parent, targetName);
        while (FileUtils::isFileExsit(g_file_get_uri(newFile.get()->get()))) {
            QString fileUri = handleDuplicate(FileUtils::getFileUri(newFile));
            newName = FileUtils::getUriBaseName(fileUri);
            newFile = FileUtils::resolveRelativePath(parent, newName);
            getOperationInfo().get()->m_dest_dir_uri = FileUtils::getFileUri(newFile);
        }        
//        getOperationInfo().get()->m_dest_dir_uri = FileUtils::getFileUri(newFile);
        m_new_names.append(newName);

        bool successed = false;

        if (is_local_desktop_file) {
            GError *err = nullptr;
            g_file_move(file.get()->get(),
                        newFile.get()->get(),
                        G_FILE_COPY_ALL_METADATA,
                        nullptr,
                        nullptr,
                        nullptr,
                        &err);
            if (err) {
                qDebug()<<err->message;
                FileOperationError except;
                except.srcUri = uri;
                except.destDirUri = FileUtils::getFileUri(newFile);
                except.isCritical = true;
                except.op = FileOpRename;
                except.title = tr("Rename file error");
                except.errorType = ET_GIO;
                except.dlgType = ED_WARNING;
                except.errorCode = err->code;
                except.errorStr = err->message;

                errored (except);

                // fallback
                g_autoptr (GError) error = nullptr;
                set_desktop_name (url.path (), oldDesktopName, &error);
                if (error) {
                    qDebug() << "fallback error: " << error->code << "  msg: " << error->message;
                }

                g_file_set_attributes_from_info(file.get()->get(), info.get()->get(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &error);
                if (error) {
                    qDebug() << "g_file_set_attributes_from_info error code: " << error->code << " msg: " << error->message;
                }

                cancel ();
            } else {
                successed = true;
            }
        } else {
    retry:
            GError* err = nullptr;
            g_autofree char* newName = g_file_get_basename(newFile.get()->get());

            g_autoptr (GFile) target_file = g_file_set_display_name(file.get()->get(), newName, getCancellable().get()->get(), &err);

            if (err) {
                FileOperationError except;
                except.srcUri = uri;
                except.errorType = ET_GIO;
                except.op = FileOpRename;
                except.dlgType = ED_WARNING;
                except.title = tr("Rename file error");
                except.destDirUri = FileUtils::getFileUri(newFile);
                qDebug() << "rename: " << g_file_get_uri(newFile.get()->get());
                except.dlgType = g_error_matches(err, g_io_error_quark(), G_IO_ERROR_FILENAME_TOO_LONG)? ED_RENAME: ED_WARNING;
                except.errorCode = err->code;
                except.errorStr = err->message;
                qDebug() << err->message;

                if (G_IO_ERROR_EXISTS == err->code) {
                    ExceptionResponse resp = prehandle(err);
                    if (Other == resp) {
                        except.dlgType = ED_CONFLICT;
                        except.errorCode = G_IO_ERROR_EXISTS;
                    }
                    Q_EMIT errored(except);
                    resp = except.respCode;
                    switch (resp) {
                    case BackupAll:
                    case BackupOne:
                    case OverWriteAll:
                        break;
                    case OverWriteOne: {
                        // 避免重名替换
                        //fix bug#143435, use m_src_uris is null cause crash issue
                        if (FileUtils::isSamePath(except.srcUri, except.destDirUri)
                                || !FileUtils::isFileExsit(except.srcUri)
                                || !FileUtils::isFileExsit(except.destDirUri)) {
                            break;
                        }
                        auto fileSystemType = FileUtils::getFsTypeFromFile(except.srcUri);
                        if (fileSystemType.contains("exfat") && err->code == G_IO_ERROR_EXISTS) {
                            break;
                        }
                        g_file_delete(newFile.get()->get(), nullptr, nullptr);
                        goto retry;
                    }
                    case IgnoreAll:
                    case IgnoreOne:
                        break;
                    case Cancel:
                        cancel();
                        goto cancel;
                    case RenameOne: {
                        m_new_name = except.respValue.value("newName").toString();
                        newFile = FileUtils::resolveRelativePath(parent, m_new_name);
                        getOperationInfo().get()->m_dest_dir_uri = FileUtils::getFileUri(newFile);
                        setHasError(false);
                        goto retry;
                    }
                    default:
                        break;
                    }
                } else {
                    if (!G_IO_ERROR_CANCELLED == err->code) {
                        Q_EMIT errored(except);
                    }
                    switch (except.respCode) {
                    case Retry:
                        goto retry;
                    case Cancel:
                        cancel();
                        break;
                    case RenameOne: {
                        m_new_name = except.respValue.value("newName").toString();
                        newFile = FileUtils::resolveRelativePath(parent, m_new_name);
                        getOperationInfo().get()->m_dest_dir_uri = FileUtils::getFileUri(newFile);
                        setHasError(false);
                        goto retry;
                    }
                    default:
                        break;
                    }
                }

                g_error_free(err);
            } else {
                successed = true;
            }
        }

        getOperationInfo().get()->m_dest_dir_uri = FileUtils::getFileUri(newFile);
    cancel:
        if (!isCancelled()) {
            auto string = g_file_get_uri(newFile.get()->get());
            destUri = string;
            if (string)
                g_free(string);
            m_info->m_node_map.insert(uri, destUri);
            m_info->m_newname = newName;
            m_info->m_oldname = oldName;
        }

        if (successed)
            Q_EMIT remoteFileEvent(103, uri, destUri);
        fileSync(uri, destUri);
        m_current_offset += 1;
        Q_EMIT FileProgressCallback(uri, destUri, fileIconName, m_current_offset, m_total_size);
    }
    m_info->m_newnames = m_new_names;
    m_info->m_oldnames = m_old_names;
    if (isCancelled()) {
        Q_EMIT operationStartRollbacked();
        rollback(m_info);
    }

    Q_EMIT operationFinished();
    //notifyFileWatcherOperationFinished();
}

void FileBatchRenameOperation::rollback(std::shared_ptr<FileOperationInfo> info)
{
    m_info->m_src_uris = info->m_node_map.keys();
    auto nodes = info->m_node_map;

    for (auto srcUri : nodes.keys()) {
        auto destUri = nodes.value(srcUri);
        g_autoptr (GFile) srcFile = g_file_new_for_uri(srcUri.toUtf8().constData());
        g_autoptr (GFile) destFile = g_file_new_for_uri(destUri.toUtf8().constData());
        auto name = g_file_get_basename(srcFile);

        g_file_set_display_name(destFile, name, 0, 0);

    }
}

QString FileBatchRenameOperation::getFileExtensionOfFile(const QString& file)
{
    /* 一些常见扩展名处理，特殊情况以后待完善 */
    QFileInfo qFileInfo(file);
    QString suffix = qFileInfo.suffix();
    QString fileBaseName = qFileInfo.fileName().left(qFileInfo.fileName().length() - suffix.length() - 1);
    if (fileBaseName.isEmpty()){
        return QString();
    }
    else if(fileBaseName.endsWith(".tar")){
        return QString("tar").append(".").append(suffix);
    }
    else if(fileBaseName.endsWith(".7z")){
        return QString("7z").append(".").append(suffix);
    }

    return suffix;
}

ExceptionResponse FileBatchRenameOperation::prehandle(GError *err)
{
    if (err && G_IO_ERROR_EXISTS == err->code) {
        return m_apply_all;
    }

    return Other;
}

QString FileBatchRenameOperation::handleDuplicate(QString uri)
{
    QString handledName = nullptr;
    QString name = QUrl(uri).toDisplayString().split("/").last();

    QRegExp regExpNum("\\(\\d+\\)");
    QRegExp regExp (QString("\\(\\d+\\)(\\.[0-9a-zA-Z\\.]+|)$"));

    QString dupReg = nullptr;

    if (name.contains(regExp)) {
        int num = 0;
        QString numStr = "";

        QString ext = regExp.cap(0);
        if (ext.contains(regExpNum)) {
            numStr = regExpNum.cap(0);
        }

        numStr.remove(0, 1);
        numStr.chop(1);
        num = numStr.toInt();
        ++num;
        handledName = name.replace(regExp, ext.replace(regExpNum, QString("(%1)").arg(num)));
    } else {
        if (name.contains(".")) {
            auto list = name.split(".");
            if (list.count() <= 1) {
                handledName = name + QString("(1)");
            } else {
                int pos = list.count() - 1;
                if (list.last() == "gz"
                        || list.last() == "xz"
                        || list.last() == "Z"
                        || list.last() == "sit"
                        || list.last() == "bz"
                        || list.last() == "bz2") {
                    --pos;
                }
                if (pos < 0) {
                    pos = 0;
                }
                auto tmp = list;
                QStringList suffixList;
                for (int i = 0; i < list.count() - pos; i++) {
                    suffixList.prepend(tmp.takeLast());
                }
                auto suffix = suffixList.join(".");

                auto basename = tmp.join(".");
                name = basename + QString("(1)") + "." + suffix;
                if (name.endsWith(".")) {
                    name.chop(1);
                }
                handledName = name;
            }
        } else {
            handledName = name + QString("(1)");
        }
    }

    return handledName;
}

static QString set_desktop_name (QString file, QString& name, GError** error)
{
    QString oldName = "";
    GKeyFile *keyFile = g_key_file_new();
    g_key_file_load_from_file(keyFile, file.toUtf8().constData(), G_KEY_FILE_KEEP_COMMENTS, nullptr);
    QString locale_name = QLocale::system().name();
    QString local_name_key = QString("Name[%1]").arg(locale_name);
    if (g_key_file_has_key(keyFile, G_KEY_FILE_DESKTOP_GROUP, local_name_key.toUtf8().constData(), nullptr)) {
        oldName = g_key_file_get_string (keyFile, G_KEY_FILE_DESKTOP_GROUP, local_name_key.toUtf8().constData(), nullptr);
        g_key_file_set_value(keyFile, G_KEY_FILE_DESKTOP_GROUP, local_name_key.toUtf8().constData(), name.toUtf8().constData());
    } else {
        g_key_file_set_value(keyFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, name.toUtf8().constData());
    }

    g_key_file_save_to_file(keyFile, file.toUtf8().constData(), error);

    if (keyFile)    g_key_file_free(keyFile);

    return oldName;
}

FileBatchRenameInternalOperation::FileBatchRenameInternalOperation(std::shared_ptr<FileOperationInfo> info)
{
    m_info = info;
    m_info->m_src_uris = m_info->m_node_map.keys();
}

void FileBatchRenameInternalOperation::run()
{
    Q_EMIT operationStarted();

    auto nodes = m_info->m_node_map;

    for (auto uri : nodes) {
        Q_EMIT operationPreparedOne(uri, 1);
    }
    Q_EMIT operationPrepared();

    for (auto srcUri : nodes.keys()) {
        auto destUri = nodes.value(srcUri);
        g_autoptr (GFile) srcFile = g_file_new_for_uri(srcUri.toUtf8().constData());
        g_autoptr (GFile) destFile = g_file_new_for_uri(destUri.toUtf8().constData());

        // FIXME: 桌面配置文件undo不生效问题
//        bool is_local_desktop_file = false;
//        if (srcUri.startsWith("file:///") && srcUri.endsWith(".desktop")) {
//            g_autoptr (GFile) srcFile = g_file_new_for_uri(srcUri.toUtf8().constData());
//            g_autofree gchar* path = g_file_get_path(srcFile);
//            g_autoptr (GDesktopAppInfo) app_info = g_desktop_app_info_new_from_filename(path);
//            if (app_info) {
//                is_local_desktop_file = true;
//                QUrl destUrl = destUri;
//                auto destName = destUrl.fileName();
//                destName.remove(".desktop");
//                set_desktop_name(path, destName, 0);
//            }
//        }

//        if (!is_local_desktop_file) {
          g_file_move(srcFile, destFile, GFileCopyFlags(G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA), 0, 0, 0, 0);
//        }

        Q_EMIT operationProgressedOne(srcUri, destUri, 1);
    }

    Q_EMIT operationFinished();
}
