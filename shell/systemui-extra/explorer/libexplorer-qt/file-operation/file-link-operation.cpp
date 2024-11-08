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

#include "file-link-operation.h"
#include "file-operation-manager.h"

#include "gerror-wrapper.h"
#include "sound-effect.h"
#ifdef LINGMO_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif

#include <QUrl>
#include <QProcess>

using namespace Peony;
#ifdef LINGMO_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

FileLinkOperation::FileLinkOperation(QString srcUri, QString destDirUri, QObject *parent) : FileOperation (parent)
{
    m_src_uri = srcUri;
    QUrl url = srcUri;
    //If it starts with a ".", add it directly to the end
    if(url.fileName().startsWith('.')){
        m_dest_uri = destDirUri + "/" + url.fileName() + " - " + tr("Symbolic Link");
    }else{
        // Otherwise, add it directly to the front
        m_dest_uri = destDirUri + "/" + tr("Symbolic Link") + " - " + url.fileName();
    }

    m_dest_uri = QUrl::fromEncoded(m_dest_uri.toUtf8()).toDisplayString();

    QStringList fake_uris;
    fake_uris<<srcUri;
    m_info = std::make_shared<FileOperationInfo>(fake_uris, destDirUri, FileOperationInfo::Link);
}

FileLinkOperation::~FileLinkOperation()
{

}

void FileLinkOperation::linkrun()
{
    auto destFile = wrapGFile(g_file_new_for_uri(FileUtils::urlEncode(m_dest_uri).toUtf8().constData()));
    GError *err = nullptr;

retry:
    QUrl url = m_src_uri;
    auto path = url.path();
    auto utf8 = path.toUtf8();
    auto symlinkValue = utf8.constData();
    //使用这种方式获取路径在路径中含有较多生僻字的时候会失效,经过定位，这种情况发生在调用constData获取指针时
//    const char* symlinkValue = url.path().toUtf8().constData();
    g_file_make_symbolic_link(destFile.get()->get(), symlinkValue, nullptr, &err);
    if (err) {
        //fix bug#162416, empty pointer err cause crash issue
        qDebug() << "linkrun:" << err->message;
        setHasError(true);
        //forbid response actions except retry and cancel.
        FileOperationError except;
        except.srcUri = m_src_uri;
        except.errorType = ET_GIO;
        except.isCritical = true;
        except.errorStr = err->message;
        except.errorCode = err->code;
        except.op = FileOpLink;
        except.title = tr("Link file error");
        except.destDirUri = m_dest_uri;
        auto responseType = Invalid;
        if (G_IO_ERROR_EXISTS == err->code) {
            except.dlgType = ED_WARNING;
            Q_EMIT errored(except);
            responseType = except.respCode;
        } else {
            except.dlgType = ED_WARNING;
            Q_EMIT errored(except);
            responseType = except.respCode;
        }

        if (responseType == Peony::Retry) {
            goto retry;
        } else if (responseType == Peony::Cancel) {
            goto end;
        }
    }
    else{
        //SoundEffect::getInstance()->copyOrMoveSucceedMusic();
        //Task#152997, use sdk play sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
        kdk::KSoundEffects::playSound(SoundType::OPERATION_FILE);
#endif
    }

    g_file_set_display_name(destFile.get()->get(), QUrl::fromPercentEncoding(m_dest_uri.split("/").last().toUtf8()).toUtf8().constData(), nullptr, nullptr);

end:

    // maybe not need sync ???
    fileSync(m_src_uri, m_dest_uri);
}

void FileLinkOperation::run()
{
    operationStarted();
    linkrun();
    // judge if the operation should sync.
//    bool needSync = false;
//    GFile *src_first_file = g_file_new_for_uri(m_src_uri.toUtf8().constData());
//    GMount *src_first_mount = g_file_find_enclosing_mount(src_first_file, nullptr, nullptr);
//    if (src_first_mount) {
//        needSync = g_mount_can_unmount(src_first_mount);
//        g_object_unref(src_first_mount);
//    } else {
//        // maybe a vfs file.
//        needSync = true;
//    }
//    g_object_unref(src_first_file);

//    GFile *dest_dir_file = g_file_new_for_uri(m_dest_uri.toUtf8().constData());
//    GMount *dest_dir_mount = g_file_find_enclosing_mount(dest_dir_file, nullptr, nullptr);
//    if (src_first_mount) {
//        needSync = g_mount_can_unmount(dest_dir_mount);
//        g_object_unref(dest_dir_mount);
//    } else {
//        needSync = true;
//    }
//    g_object_unref(dest_dir_file);

//    //needSync = true;

//    if (needSync) {
//        auto path = g_file_get_path(destFile.get()->get());
//        if (path) {
//            operationStartSnyc();
//            QProcess p;
//            p.start(QString("sync -f '%1'").arg(path));
//            p.waitForFinished(-1);
//            g_free(path);
//        }
//    }
    operationFinished();
    //notifyFileWatcherOperationFinished();
}

