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

#include "file-operation-utils.h"
#include "file-operation-manager.h"

#include "file-move-operation.h"
#include "file-copy-operation.h"
#include "file-trash-operation.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"
#include "file-delete-operation.h"
#include "file-link-operation.h"

#include "file-untrash-operation.h"
#include "file-count-operation.h"

#include "file-info-job.h"
#include "file-info.h"
#include "file-enumerator.h"
#include "audio-play-manager.h"

#include "file-operation-internal-dialog.h"
#include "sound-effect.h"
#include "global-settings.h"

#include <QUrl>
#include <QFileInfo>
#include <gio/gio.h>

#include <QMessageBox>

#include <QStandardPaths>

using namespace Peony;

static FileOperation *trashInternal(const QStringList &uris, bool addHistory, bool isFromSearchTab, bool forceShowDialog)
{
    FileOperation *op = nullptr;
    bool canNotTrash = false;
    bool isBigFile = false;

    QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString newBoxPath;
    QString oldBoxPath;

    //1.0保护箱路径为“/box/用户名/保护箱名称”,2.1保护箱路径为“/home/用户名/.box/保护箱名称”，只要包含这些路径就是保护箱下文件，是不可以删除到回收站的；
    //root下没有home；
    if(userPath.startsWith("/home")){
        newBoxPath = userPath+"/.box";

        QString user = userPath.remove("/home/");

        oldBoxPath = "/box/"+user;

    } else {
        newBoxPath = userPath+"/.box";
        oldBoxPath = "/box/root";
    }

    if (!uris.isEmpty()) {
        auto uri = uris.first();
        if (!uri.startsWith("file:/") || (uri.contains(newBoxPath)) || (uri.contains(oldBoxPath))) {
            canNotTrash = true;
        }
        if (uri.contains("kydrive") && uri.startsWith("file:///media")) {
            canNotTrash = true;
        }
    }

    if (!canNotTrash) {
        quint64 total_size = 0;
        const quint64 ONE_GIB_SIZE = 1024*1024*1024;
        for (auto uri : uris) {
            QUrl url(uri);
            QFile file(url.path());
            //fix iso symbolLink delete prompt can not trash issue
            //FIXME: replace BLOCKING api in ui thread. and no type.
            auto info = FileInfo::fromUri(uri);
            if (info->isSymbolLink())
                continue;

            //folder need recursive calculate file size
            //屏蔽本部分代码，文件和文件夹走一样的异常处理流程，关联bug#92483
//            if(! info->isDir()){
//                total_size += file.size();
//            }

//            //file total size more than 10G, not trash but delete, task#56444,  bug#88871, bug#88894
//            if (total_size/10 > ONE_GIB_SIZE) {
//                canNotTrash = true;
//                isBigFile = true;
//                break;
//            }
//            qDebug() <<"total_size:" <<total_size<<ONE_GIB_SIZE<<canNotTrash<<isBigFile;

            //task#147972 【删除回收站】选项需求
            //如果是长城的机器并且带了9215控制器，不再区分移动设备，统一右键删除到回收站
            bool trashSettings = GlobalSettings::getInstance()->getValue(TRASH_MOBILE_FILES).toBool();
            if (trashSettings)
                break;

            //file total size more than 10G, not trash but delete, task#56444
            //FIXME 判断是否是移动设备文件，可能不准确, 目前暂未找到好的判断方法
            bool isMobileDeviece = FileUtils::isMobileDeviceFile(uri);
            if(isMobileDeviece){
                canNotTrash = true;
                break;
            }
        }
    }

    // comment for fixing #82054
//    if (!canNotTrash) {
//        FileEnumerator e;
//        e.setEnumerateDirectory("trash:///");
//        e.enumerateSync();
//        if (e.getChildrenUris().count() > 1000) {
//            canNotTrash = true;
//        }
//    }

    if (canNotTrash) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        //task #155670,155671 improve delete file permanently message
        QString message;
        if (isBigFile){
            message = QObject::tr("The file is too large to be moved to the recycle bin. "
                                  "Do you want to permanently delete it?");

            if (uris.length() > 1)
                message = QObject::tr("These files are too large to be moved to the recycle bin. "
                                      "Do you want to permanently delete these %1 files?").arg(uris.length());
        }
        else if (uris.length() == 1){
            message = QObject::tr("Are you sure you want to permanently delete this file?"
                                  " Once deletion begins, "
                                  "the file will not be recoverable.");
        }else{
            message = QObject::tr("Are you sure you want to permanently delete these %1 files?"
                                  " Once deletion begins, "
                                  "these file will not be recoverable.").arg(uris.length());
        }

        auto result = QMessageBox::question(nullptr, "", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (result == QMessageBox::Yes) {
            op = FileOperationUtils::remove(uris);
        }
        return op;
    }

    auto fileOpMgr = FileOperationManager::getInstance();
    auto trashOp = new FileTrashOperation(uris);
    if (isFromSearchTab) {
        trashOp->setSearchOperation(isFromSearchTab);
    }
    fileOpMgr->startOperation(trashOp, addHistory, forceShowDialog);

    return trashOp;
}

FileOperationUtils::FileOperationUtils()
{

}

FileOperation *FileOperationUtils::move(const QStringList &srcUris, const QString &destUri, bool addHistory, bool copyMove)
{
    FileOperation *op;
    QString destDir = nullptr;
    auto fileOpMgr = FileOperationManager::getInstance();
    if (destUri != "trash:///") {
        if (true == destUri.startsWith("computer:///")) {
            destDir = FileUtils::getTargetUri(destUri);
            if (nullptr == destDir){
                qWarning()<<"get target uri failed, from uri:"
                          <<destUri;
                destDir = destUri;
            }
        }
        else {
            destDir = destUri;
        }

        auto moveOp = new FileMoveOperation(srcUris, destDir);
        moveOp->setAction(copyMove? Qt::MoveAction: Qt::TargetMoveAction);
        //moveOp->setCopyMove(copyMove);
        op = moveOp;
        fileOpMgr->startOperation(moveOp, addHistory);
    } else {
        op = FileOperationUtils::trash(srcUris, true);
    }
    return op;
}

FileOperation *FileOperationUtils::copy(const QStringList &srcUris, const QString &destUri, bool addHistory)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto copyOp = new FileCopyOperation(srcUris, destUri);
    fileOpMgr->startOperation(copyOp, addHistory);
    return copyOp;
}

FileOperation *FileOperationUtils::trash(const QStringList &uris, bool addHistory)
{
    return trash(uris, addHistory, false);
}

FileOperation *FileOperationUtils::trash(const QStringList &uris, bool addHistory, bool isFromSearchTab)
{
    return trashInternal(uris, addHistory, isFromSearchTab, false);
}

FileOperation *FileOperationUtils::trashWithDialog(const QStringList &uris, bool addHistory)
{
    return trashInternal(uris, addHistory, false, true);
}

FileOperation *FileOperationUtils::rename(const QString &uri, const QString &newName, bool addHistory)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto renameOp = new FileRenameOperation(uri, newName);
    fileOpMgr->startOperation(renameOp, addHistory);
    return renameOp;
}

FileOperation *FileOperationUtils::remove(const QStringList &uris)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto removeOp = new FileDeleteOperation(uris);
    fileOpMgr->startOperation(removeOp);
    return removeOp;
}

FileOperation *FileOperationUtils::link(const QString &srcUri, const QString &destUri, bool addHistory)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto linkOp = new FileLinkOperation(srcUri, destUri);
    fileOpMgr->startOperation(linkOp, addHistory);
    return linkOp;
}

std::shared_ptr<FileInfo> FileOperationUtils::queryFileInfo(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(uri);
    auto job = new FileInfoJob(info);
    job->querySync();
    job->deleteLater();
    return info;
}

FileOperation *FileOperationUtils::moveWithAction(const QStringList &srcUris, const QString &destUri, bool addHistory, Qt::DropAction action)
{
    return moveWithAction(srcUris, destUri, addHistory, action, false);
}

FileOperation *FileOperationUtils::moveWithAction(const QStringList &srcUris, const QString &destUri, bool addHistory, Qt::DropAction action, bool bMoveFromSearchTab)
{
    FileOperation *op;
    QString destDir = nullptr;
    auto fileOpMgr = FileOperationManager::getInstance();
    if (destUri != "trash:///") {
        if (true == destUri.startsWith("computer:///")) {
            destDir = FileUtils::getTargetUri(destUri);
            if (nullptr == destDir){
                qWarning()<<"get target uri failed, from uri:"
                          <<destUri;
                destDir = destUri;
            }
        }
        else {
            destDir = destUri;
        }

        if (action == Qt::CopyAction) {
            // fix 71411
            op = new FileCopyOperation(srcUris, destDir);
        } else {
            auto moveOp = new FileMoveOperation(srcUris, destDir);
            moveOp->setAction(action);
            moveOp->setSearchOperation(bMoveFromSearchTab);
            op = moveOp;
        }

        fileOpMgr->startOperation(op, addHistory);
    } else {
        op = FileOperationUtils::trash(srcUris, true, bMoveFromSearchTab);
    }
    return op;
}

FileOperation *FileOperationUtils::clearRecycleBinWithDialog(const QStringList &list)
{
    return clearRecycleBinWithDialog(list, nullptr);
}

FileOperation *FileOperationUtils::clearRecycleBinWithDialog(const QStringList &list, QWidget *parent)
{
    FileOperationInternalDialog questionbox((QDialog*)parent);
    auto okButton = questionbox.addButton(QObject::tr("Clean the Trash"));
    okButton->setProperty("isImportant", true);
    questionbox.connect(okButton, &QPushButton::clicked, &questionbox, [&]{
        questionbox.accept();
    });
    auto cancelButton = questionbox.addButton(QObject::tr("Cancel"));
    questionbox.connect(cancelButton, &QPushButton::clicked, &questionbox, [&]{
        questionbox.reject();
    });
    cancelButton->setProperty("useButtonPalette", true);

    okButton->setFocus();
    questionbox.setText(QObject::tr("Do you want to empty the recycle bin and delete the files permanently? Once it has begun there is no way to restore them."));
    questionbox.setIcon("user-trash-full");
    Peony::AudioPlayManager::getInstance()->playWarningAudio();
    if (questionbox.exec()) {
        FileOperation *operation = nullptr;
        if (!list.isEmpty()) {
            operation = FileOperationUtils::remove(list);
        } else {
            auto uris = FileUtils::getChildrenUris("trash:///");
            if (uris.isEmpty()) {
                return nullptr;
            }
            operation = FileOperationUtils::remove(uris);
        }
        SoundEffect::getInstance()->recycleBinDeleteMusic();
        return operation;
    } else {
        return nullptr;
    }
}

FileOperation *FileOperationUtils::restore(const QString &uriInTrash)
{
    QStringList uris;
    uris<<uriInTrash;
    auto fileOpMgr = FileOperationManager::getInstance();
    auto untrashOp = new FileUntrashOperation(uris);
    fileOpMgr->startOperation(untrashOp, true);
    return untrashOp;
}

FileOperation *FileOperationUtils::restore(const QStringList &urisInTrash)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto untrashOp = new FileUntrashOperation(urisInTrash);
    //FIXME: support undo?
    fileOpMgr->startOperation(untrashOp, false);
    return untrashOp;
}

FileOperation *FileOperationUtils::create(const QString &destDirUri, const QString &name, CreateTemplateOperation::Type type)
{
    auto fileOpMgr = FileOperationManager::getInstance();
    auto createOp = new CreateTemplateOperation(destDirUri, type, name);
    fileOpMgr->startOperation(createOp, true);
    return createOp;
}

void FileOperationUtils::executeRemoveActionWithDialog(const QStringList &uris)
{
    if (uris.isEmpty())
        return;

    Peony::AudioPlayManager::getInstance()->playWarningAudio();
    int result = 0;
    QString message = QObject::tr("Are you sure you want to permanently delete this file?"
                          " Once deletion begins, "
                          "the file will not be recoverable.");
    if (uris.count() > 1) {
        message = QObject::tr("Are you sure you want to permanently delete these %1 files?"
                              " Once deletion begins, "
                              "these file will not be recoverable.").arg(uris.length());
    }

    result = QMessageBox::question(nullptr, "", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (result == QMessageBox::Yes) {
        FileOperationUtils::remove(uris);
    }
}

//not accurate of process, name has tr("duplicate") not processed
//do not use this function before you fixed it
bool FileOperationUtils::leftNameIsDuplicatedFileOfRightName(const QString &left, const QString &right)
{
    auto tmpl = left;
    auto tmpr = right;
    tmpl.remove(QRegExp("\\(\\d+\\)"));
    tmpr.remove(QRegExp("\\(\\d+\\)"));
    return tmpl == tmpr;
}

static int getNumOfFileName(const QString &name)
{
    QRegExp regExp("\\(\\d+\\)");
    int num = 0;

    if (name.contains(regExp)) {
        int pos = 0;
        QString tmp;
        while ((pos = regExp.indexIn(name, pos)) != -1) {
            tmp = regExp.cap(0).toUtf8();
            pos += regExp.matchedLength();
        }
        tmp.remove(0,1);
        tmp.chop(1);
        num = tmp.toInt();
    }
    return num;
}

bool FileOperationUtils::leftNameLesserThanRightName(const QString &left, const QString &right)
{
    auto tmpl = left;
    auto tmpr = right;
    int numl = getNumOfFileName(tmpl);
    int numr = getNumOfFileName(tmpr);
    //fix bug#97408,change indicator meanings
    return numl == numr? left > right: numl > numr;
}
