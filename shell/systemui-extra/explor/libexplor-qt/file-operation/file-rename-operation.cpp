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

#include "file-rename-operation.h"
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

static QString handleDuplicate(QString name)
{
    return FileUtils::handleDuplicateName(name);
}

FileRenameOperation::FileRenameOperation(QString uri, QString newName)
{
    m_uri = uri;
    m_new_name = newName;
    m_old_name = FileUtils::getFileDisplayName(uri);
    QStringList srcUris;
    srcUris<<uri;
    QString destUri = FileUtils::getParentUri(FileUtils::urlEncode(uri));
    if (destUri != nullptr) {
        destUri = FileUtils::urlEncode(FileUtils::urlDecode(destUri) + "/" + m_new_name);
    }

    m_info = std::make_shared<FileOperationInfo>(srcUris, destUri, FileOperationInfo::Rename);
}

/*!
 * \brief FileRenameOperation::run
 * \bug
 * how to keep the permission?
 */
void FileRenameOperation::run()
{
    QString destUri;
    Q_EMIT operationStarted();

    bool needHidden = false;
    if (m_new_name == "/" || m_new_name == "." || !nameIsValid(m_new_name)) {
        FileOperationError except;
        except.srcUri = m_uri;
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
            except.srcUri = m_uri;
            except.errorType = ET_GIO;
            except.op = FileOpRenameToHideFile;
            except.dlgType = ED_WARNING;
            except.title = tr("File Rename warning");
            except.errorStr = tr("Are you sure to hidden this file?");

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
                needHidden = true;
                qDebug() << "Q_EMIT updateHiddenFile："<<m_new_name;
                Q_EMIT GlobalSettings::getInstance()->updateHiddenFile(m_new_name);
            }
        }
    }

    //task#144488, support cancel rename operation when change file type
    //修改了文件类型后缀名，提示用户改变文件类型可能导致文件不可用
    //修复新建文件夹改名错误弹框提示问题，无后缀名的文件不处理
    bool isFolder = FileUtils::getFileIsFolder(m_uri);
    if (! isFolder && (m_new_name.split(".").length() >1 || m_old_name.split(".").length() >1) &&
        m_new_name.split(".").last() != m_old_name.split(".").last()){
        FileOperationError except;
        except.srcUri = m_uri;
        except.errorType = ET_GIO;
        except.op = FileOpRenameChangeType;
        except.dlgType = ED_WARNING;
        except.title = tr("File Rename warning");
        except.errorStr = tr("When change the file suffix, the file may be invalid. "
                             "Are you sure to change it ?");

        Q_EMIT errored(except);

        //support cancel rename operation when change file type
        if (except.respCode == Cancel) {
            cancel();
            setHasError(true);
            //未做重命名操作，恢复之前的目标文件，仍然选中原来的文件
            getOperationInfo().get()->m_dest_dir_uri = getOperationInfo().get()->sources().first();
            Q_EMIT operationFinished();
            return;
        }else if (needHidden) {
            //fix bug#205715, 同时修改为隐藏文件和设置隐藏，需要及时隐藏该文件
            Q_EMIT GlobalSettings::getInstance()->updateHiddenFile(m_new_name);
        }
    }

    std::shared_ptr<FileInfo> fileinfo = FileInfo::fromUri(m_uri);
    if(fileinfo && !fileinfo->isDir()){
        bool showFileExtension = Peony::GlobalSettings::getInstance()->isExist(SHOW_FILE_EXTENSION)?
                    Peony::GlobalSettings::getInstance()->getValue(SHOW_FILE_EXTENSION).toBool():true;
        if(!showFileExtension){
            QString oldSuffix = getFileExtensionOfFile(m_old_name);
            QString newSuffix = getFileExtensionOfFile(m_new_name);
            if((oldSuffix != newSuffix) && !oldSuffix.isEmpty())
            {
                m_new_name = m_new_name.append(".").append(oldSuffix);
            }
        }
    }
    auto file = wrapGFile(g_file_new_for_uri(FileUtils::urlEncode(m_uri).toUtf8().constData()));
    auto info = wrapGFileInfo(g_file_query_info(file.get()->get(), "*",
                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                              getCancellable().get()->get(),
                              nullptr));

    bool is_local_desktop_file = false;
    QUrl url = m_uri;
    QString oldDesktopName = "";
    //change the content of .desktop file;
    if (url.isLocalFile() && m_uri.endsWith(".desktop")) {
        GDesktopAppInfo *desktop_info = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
        if (G_IS_DESKTOP_APP_INFO(desktop_info)) {
            bool is_executable = g_file_test (url.path().toUtf8().constData(), G_FILE_TEST_IS_EXECUTABLE);
            is_local_desktop_file = is_executable;
            if (is_executable) {
                g_autoptr (GError) error = nullptr;
                oldDesktopName = set_desktop_name (url.path (), m_new_name, &error);
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

    QString targetName = m_new_name;
    if (is_local_desktop_file) {
        targetName = m_new_name+".desktop";
    }

    auto parent = FileUtils::getFileParent(file);
    auto newFile = FileUtils::resolveRelativePath(parent, targetName);
    getOperationInfo().get()->m_dest_dir_uri = FileUtils::getFileUri(newFile);

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
            except.srcUri = m_uri;
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
        FileOperationError except;
        except.srcUri = m_uri;
        except.errorType = ET_GIO;
        except.op = FileOpRename;
        except.dlgType = ED_WARNING;
        except.title = tr("Rename file error");
        except.destDirUri = FileUtils::getFileUri(newFile);
        qDebug() << "rename: " << g_file_get_uri(newFile.get()->get());
        g_autofree char* newName = g_file_get_basename(newFile.get()->get());

        g_autoptr (GFile) target_file = g_file_set_display_name(file.get()->get(), newName, nullptr, &err);

        if (err) {
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
                    setAutoBackup();
                case BackupOne:{
                    while (FileUtils::isFileExsit(g_file_get_uri(newFile.get()->get()))) {
                        QString fileUri = handleDuplicate(FileUtils::getFileUri(newFile));
                        m_new_name = FileUtils::getUriBaseName(fileUri);
                        newFile = FileUtils::resolveRelativePath(parent, m_new_name);
                        getOperationInfo().get()->m_dest_dir_uri = FileUtils::getFileUri(newFile);
                    }
                    if (needHidden) {
                        Q_EMIT GlobalSettings::getInstance()->updateHiddenFile(m_new_name);
                    }
                    goto retry;
                }
                case OverWriteAll:
                    setAutoOverwrite();
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
                    g_clear_error(&err);
                    g_file_delete(newFile.get()->get(), nullptr, &err);
                    if (err) {
                        except.dlgType = ED_WARNING;
                        except.errorStr = err->message;
                        Q_EMIT errored(except);
                        break;
                    }
                    goto retry;
                }
                case IgnoreAll:
                    setAutoIgnore();
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
                Q_EMIT errored(except);
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
        m_info->m_node_map.insert(m_uri, destUri);
        m_info->m_newname = m_new_name;
        m_info->m_oldname = m_old_name;
    }

    if (successed)
        Q_EMIT remoteFileEvent(103, m_uri, destUri);
    fileSync(m_uri, destUri);

#ifdef LINGMO_UDF_BURN
    std::shared_ptr<FileOperationHelper> mHelper = std::make_shared<FileOperationHelper>(m_uri);
    if (mHelper->isUnixCDDevice()) {
        mHelper->judgeSpecialDiscOperation();
        QString oldNamePath = mHelper->getDestName(m_uri);
        mHelper->discRenameOperation(oldNamePath, m_new_name);
    }
#endif

    Q_EMIT operationFinished();
    //notifyFileWatcherOperationFinished();

}

QString FileRenameOperation::getFileExtensionOfFile(const QString& file)
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

ExceptionResponse FileRenameOperation::prehandle(GError *err)
{
    if (err && G_IO_ERROR_EXISTS == err->code) {
        return m_apply_all;
    }

    return Other;
}


/**
 * @brief
 *  If the desktop shortcut rename fails, you need to roll back to the original name.
 * @param file: .desktop file path
 * @param name: .desktop file's Name[Local]=name
 * @param error: out param
 *
 * @return oldName
 */
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
