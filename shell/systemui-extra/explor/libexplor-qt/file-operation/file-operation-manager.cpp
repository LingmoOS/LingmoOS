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

#include "file-operation-manager.h"
#include "file-operation.h"

#include "global-settings.h"

#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QtConcurrent>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QMutexLocker>

#include "file-copy-operation.h"
#include "file-delete-operation.h"
#include "file-link-operation.h"
#include "file-move-operation.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"
#include "file-trash-operation.h"
#include "file-untrash-operation.h"

#include "file-operation-error-dialog.h"
#include "file-operation-internal-dialog.h"
#include "file-operation-progress-wizard.h"

#include "file-watcher.h"
#include "audio-play-manager.h"

//#include "properties-window.h"
#include "sound-effect.h"
#include "remote-file-event-helper.h"

#include <kballontip.h>
#ifdef LINGMO_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif

#include <QVector4D>

#include <QDebug>
#include <unistd.h>

using namespace Peony;
#ifdef LINGMO_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

static FileOperationManager *global_instance = nullptr;

FileOperationManager::FileOperationManager(QObject *parent) : QObject(parent)
{
    m_allow_parallel = GlobalSettings::getInstance()->getValue(ALLOW_FILE_OP_PARALLEL).toBool();

    qRegisterMetaType<Peony::GErrorWrapperPtr>("Peony::GErrorWrapperPtr");
    qRegisterMetaType<Peony::GErrorWrapperPtr>("Peony::GErrorWrapperPtr&");
    m_thread_pool = new QThreadPool(this);
    m_progressbar = FileOperationProgressBar::getInstance();
    m_mount_operation_list = new QHash<QString, totalOperationInfo>;
    m_operation_use_list = new QHash<FileOperation*, currentOpertionInfo>;
    if (!m_allow_parallel) {
        //Imitating queue execution.
        m_thread_pool->setMaxThreadCount(1);
    }

    //
    connect(m_progressbar, &FileOperationProgressBar::canceled, [=] () {
        m_progressbar->removeAllProgressbar();
    });

    // 智能数据管理远程事件转发
    m_replicaThread = new QThread;
    m_replica = new RemoteFileEventHelper;
    connect(this, &FileOperationManager::remoteFileEvent, m_replica, &RemoteFileEventHelper::handleFileEventRequest);
    m_replica->moveToThread(m_replicaThread);
    m_replicaThread->start();

    // 休眠检测
    GDBusConnection* pconnection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if (pconnection) {
        g_dbus_connection_signal_subscribe(pconnection, "org.freedesktop.login1", "org.freedesktop.login1.Manager", "PrepareForSleep", "/org/freedesktop/login1", NULL, G_DBUS_SIGNAL_FLAGS_NONE, systemSleep, this, NULL);
    }

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected()) {
        qCritical()<<"failed to init mDbusDateServer, can not connect to session dbus";
        return;
    }

    m_iface = new QDBusInterface("org.lingmo.explor", "/org/lingmo/explor", "org.lingmo.explor", QDBusConnection::sessionBus());
    if (!m_iface->isValid()){
        qCritical() << "Create /org/lingmo/explor Interface Failed " << QDBusConnection::sessionBus().lastError();
        return;
    }

    QDBusConnection::sessionBus().connect("org.lingmo.explor",
                                          "/org/lingmo/explor",
                                          "org.lingmo.explor",
                                          "opreateFinishedOfEngrampa",
                                          this,
                                          SLOT(slot_opreateFinishedOfEngrampa(QString, bool)));

    QDBusConnection::sessionBus().connect("",
                                          "/org/lingmo/explor",
                                          "org.lingmo.explor",
                                          "moveFilesToAnotherProcCompleted",
                                          this,
                                          SLOT(slot_moveFilesToAnotherProcCompleted(QStringList)));
}

FileOperationManager::~FileOperationManager()
{
    if(m_iface){
        delete m_iface;
        m_iface = nullptr;
    }

    m_replicaThread->quit();
    m_replicaThread->wait(500);
    delete m_replicaThread;
    delete m_replica;
}

FileOperationManager *FileOperationManager::getInstance()
{
    if (global_instance == nullptr) {
        global_instance = new FileOperationManager;
    }
    return global_instance;
}

void FileOperationManager::close()
{
    disconnect();
    deleteLater();
    global_instance = nullptr;
    Q_EMIT closed();
}

void FileOperationManager::setAllowParallel(bool allow)
{
    m_allow_parallel = allow;
    if (allow) {
        m_thread_pool->setMaxThreadCount(9999);
    } else {
        m_thread_pool->setMaxThreadCount(1);
    }
    GlobalSettings::getInstance()->setValue(ALLOW_FILE_OP_PARALLEL, allow);
}

bool FileOperationManager::isAllowParallel()
{
    return m_allow_parallel;
}

void FileOperationManager::setFsyncStatus(bool synchronizing)
{
    QMutexLocker lk(&m_fsyncMutex);
    m_isFsynchronizing = synchronizing;
}

bool FileOperationManager::isFsynchronizing()
{
    QMutexLocker lk(&m_fsyncMutex);
    return m_isFsynchronizing;
}

QStringList FileOperationManager::getFilesOpenedByProc(const QString &procName)
{
    QStringList occupiedFiles;

    QProcess process;
    QString cmd = QString("/usr/bin/lsof -c %1").arg(procName);
    process.start(cmd);
    process.waitForFinished();
    QString infos = QString(process.readAll());
    if(infos.isEmpty())
        return occupiedFiles;

    qDebug()<<infos;
    QStringList infoList = infos.split("\n");
    for(auto& info : infoList){
        QString fileName = QString(info).section(" ", -1, -1);
        occupiedFiles.append(fileName);
    }

    return occupiedFiles;
}

void FileOperationManager::startOperation(FileOperation *operation, bool addToHistory)
{    
    startOperation(operation, addToHistory, false);
}

void FileOperationManager::startOperation(FileOperation *operation, bool addToHistory, bool forceShowDialog)
{
    auto operationInfo = operation->getOperationInfo();

    QStringList uriList = operationInfo.get()->sources();
    if(!uriList.count())
        return;

    bool bDocIsOccupiedByWps = Peony::GlobalSettings::getInstance()->isExist(DOC_IS_OCCUPIED_BY_WPS)?
                Peony::GlobalSettings::getInstance()->getValue(DOC_IS_OCCUPIED_BY_WPS).toBool() : false;
    if(bDocIsOccupiedByWps){
        /* 文件被wps-office占用时处理流程;link to task#79151 文档防护软件适配---打开的文档可以被删除或重命名 */
        QString occupiedFiles;
        auto operationType = operationInfo->operationType();
        if (operationType == FileOperationInfo::Trash
             || operationType == FileOperationInfo::Delete
             || operationType == FileOperationInfo::Move
             || operationType == FileOperationInfo::Copy && operation->isCopyMove()/* 鼠标拖动文件情形 */
             || operationType == FileOperationInfo::Rename)

        {
            QStringList filesOpenedByWps = getFilesOpenedByProc("wps");
            QStringList filesOpenedByWpp = getFilesOpenedByProc("wpp");
            QStringList filesOpenedByEt = getFilesOpenedByProc("et");
            QStringList filesOpenedByproc = (filesOpenedByWps + filesOpenedByWpp + filesOpenedByEt).toSet().toList();/* 去重 */
            for(auto& uri :uriList){
                std::shared_ptr<FileInfo> fileinfo = FileInfo::fromUri(uri);
                if(fileinfo && !fileinfo->isDir()){
                    if(!uri.endsWith(".txt") && !(fileinfo->type().startsWith("application/wps-office")))
                        continue;
                }

                QString absolutePath = FileUtils::urlDecode(uri).remove("file://");
                for(auto& fileName :filesOpenedByproc){
                    if(!fileName.startsWith(absolutePath))
                        continue;

                    operationInfo.get()->m_src_uris.removeOne(uri);
                    operation->m_src_uris.removeOne(uri);
                    occupiedFiles.append(QString("file://").append(fileName)).append("  ");
                }
            }

        }
        if(!occupiedFiles.isEmpty()){
            QMessageBox::warning(nullptr, tr("Warn"), tr("'%1' is occupied，you cannot operate!").arg(occupiedFiles));
            return;
        }
    }
    //end

    if (operationInfo.get()->operationType() == FileOperationInfo::Trash) {
        auto value = GlobalSettings::getInstance()->getValue("showTrashDialog");
        if (!forceShowDialog) {
            if (value.isValid()) {
                if (value.toBool() == false) {
                    goto start;
                }
            }
        }

        // check dialog
        QWidget *widget = QApplication::topLevelAt(QCursor::pos());
        qDebug()<<"top level widget:"<<widget<<",current QPoint:"<<QCursor::pos();
        FileOperationInternalDialog questionbox((QDialog*)widget);
        auto okButton = questionbox.addButton(tr("OK"));
        connect(okButton, &QPushButton::clicked, &questionbox, [&]{
            questionbox.accept();
        });
        auto cancelButton = questionbox.addButton(tr("Cancel"));
        connect(cancelButton, &QPushButton::clicked, &questionbox, [&]{
            questionbox.reject();
        });
        okButton->setFocus();
        questionbox.setText(tr("Do you want to put selected %1 item(s) into trash?").arg(operationInfo.get()->sources().count()));
        questionbox.setIcon("user-trash");
        auto checkbox = questionbox.addCheckBoxLeft(tr("Do not show again"));
        checkbox->setVisible(!forceShowDialog);
        if (questionbox.exec()) {
//            SoundEffect::getInstance()->recycleBinDeleteMusic();
            if (checkbox->isChecked()) {
                GlobalSettings::getInstance()->setGSettingValue(SHOW_TRASH_DIALOG, false);
            }
        } else {
            return;
        }

//        QMessageBox questionBox;
//        questionBox.setIcon(QMessageBox::Question);
//        questionBox.setIconPixmap(QIcon::fromTheme("user-trash").pixmap(64, 64));
//        questionBox.addButton(QMessageBox::Yes);
//        questionBox.addButton(QMessageBox::No);
//        //questionBox.addButton(tr("No, go to settings"), QMessageBox::ActionRole);
//        QCheckBox checkbox(tr("Do not show again"), &questionBox);
//        questionBox.setCheckBox(&checkbox);
//        questionBox.setText(tr("Do you want to put selected %1 item(s) into trash?").arg(operationInfo.get()->sources().count()));
//        auto result = questionBox.exec();
//        if(result == QMessageBox::Yes){
//            SoundEffect::getInstance()->recycleBinDeleteMusic();
//            if (checkbox.isChecked()) {
//                GlobalSettings::getInstance()->setGSettingValue(SHOW_TRASH_DIALOG, false);
//            }
//        }
//        if (result != QMessageBox::Yes) {
//            if (result != QMessageBox::No) {
//                // settings
//                QStringList uris;
//                uris<<"trash:///";
//                auto propertyWindow = new PropertiesWindow(uris);
//                propertyWindow->show();
//            }
//            return;
//        }
    }

    // do not add move operation between favorite:///
    // FIXME: due to some desgin issues, we can not
    // support undo/redo with favorite:/// yet. that
    // should be fixed in the future.

    // dnd use copy move mode here between different
    // file system, so we should better check operation class,
    // not operation info type.
    if (dynamic_cast<FileMoveOperation *>(operation)) {
        if (addToHistory) {
            for (QString uri : operationInfo.get()->sources()) {
                if (uri.startsWith("favorite:///")) {
                    addToHistory = false;
                    break;
                }
            }
            if (addToHistory) {
                for (QString uri : operationInfo.get()->dests()) {
                    if (uri.startsWith("favorite:///")) {
                        addToHistory = false;
                        break;
                    }
                }
            }
        }
    }

start:
    static bool oldQuitOnLastWindow = QApplication::quitOnLastWindowClosed();
    QApplication::setQuitOnLastWindowClosed(false);

    connect(operation, &FileOperation::operationFinished, this, [=]() {
        operation->notifyFileWatcherOperationFinished();
        if (qApp->property("isPeony").toBool()) {
            auto settings = GlobalSettings::getInstance();
            bool runbackend = settings->getInstance()->getValue(RESIDENT_IN_BACKEND).toBool();
            QApplication::setQuitOnLastWindowClosed(!runbackend);
            QTimer::singleShot(1000, this, [=]() {
                int last_op_count = m_thread_pool->children().count();
                if (last_op_count == 0) {
                    if (qApp->allWidgets().isEmpty()) {
                        if (!runbackend) {
                            qApp->quit();
                        }
                    }
                }
            });
        } else {
            QApplication::setQuitOnLastWindowClosed(oldQuitOnLastWindow);
        }
    }, Qt::BlockingQueuedConnection);

    bool allowParallel = m_allow_parallel;

    connect(operation, &FileOperation::operationTotalFileSize, this, [=](const qint64& total_file_size) {
        // fix #171449
        if (m_progressbar->isHidden()) {
            m_progressbar->m_error = true;
        }

        //story 19796 空间不足时预处理
        // Check if the operation is a copy or move operation
        auto info = operation->getOperationInfo();
        if (!info || (info->operationType() != FileOperationInfo::Copy
                && info->operationType() != FileOperationInfo::Move)) {
            return;
        }
        // Check if the operation is already in the list of operations
        if (m_operation_use_list->contains(operation)) {
            qWarning() << "this operation already exist";
            return;
        }

        // Get the destination path of the operation
        g_autoptr (GFile) destGfile = g_file_new_for_uri(info->target().toUtf8().constData());
        g_autofree gchar* destPath = g_file_get_path(destGfile);

        //Get the available disk space using QStorageInfo
        QStorageInfo storage(destPath);

        // Check if the file path is mounted correctly
        if (!storage.isValid()) {
            if (m_progressbar->m_error) {
                m_progressbar->m_error = false;
            }
            qWarning() << "The file path is not mounted correctly";
            return;
        }

        quint64 diskFreeSpace = storage.bytesAvailable();

        // Check if there is an error getting the disk free space
        if (0 >= diskFreeSpace) {
            if (m_progressbar->m_error) {
                m_progressbar->m_error = false;
            }
            qWarning() << "get disk free space error!";
            return;
        }

        QString mountRootName = storage.rootPath();

        // Calculate the total size of the operation
        qint64 currentTotalSize = 0;
        totalOperationInfo totalInfo;

        // Check if the mount root name is already in the list of mount operations
        if (m_mount_operation_list->contains(mountRootName)) {
            qint64 currentUseSize = m_mount_operation_list->value(mountRootName).totalSize;
            currentTotalSize = currentUseSize + total_file_size;
            totalInfo.totalSize = currentTotalSize;
            totalInfo.preoccupationSize = m_mount_operation_list->value(mountRootName).preoccupationSize;
        } else {
            currentTotalSize = total_file_size;
            totalInfo.totalSize = currentTotalSize;
            totalInfo.preoccupationSize = diskFreeSpace;
        }

        // Check if there is enough disk space for the operation
        if(currentTotalSize > diskFreeSpace) {
            // Cancel the operation
            operation->cancel();

            // Create an error message
            FileOperationError except;
            FileOperationErrorDialogWarning dialog;
            QString name;

            if (mountRootName == "/") {
                name = tr("File System");
            } else if (mountRootName == "/data") {
                name = tr("Data");
            } else {
                name = storage.name();
            }
            except.title = tr("Insufficient storage space");
            double total_file_size_gb = (double)total_file_size / (1024 * 1024 * 1024);
            double need_total_size_gb = (double)(total_file_size - totalInfo.preoccupationSize) / (1024 * 1024 * 1024);
            except.errorStr = tr("%1 no space left on device. "
                                 "Copy file size: %2 GB, "
                                 "Space needed: %3 GB.").arg(name).arg(QString::number(total_file_size_gb, 'f', 3))
                    .arg(QString::number(need_total_size_gb, 'f', 3));

            // Display the error message
            dialog.handle(except);
            if (m_progressbar->m_error) {
                m_progressbar->m_error = false;
            }
            return;
        }

        // Record the operation information
        currentOpertionInfo opertionInfo;
        opertionInfo.mountRootName = mountRootName;
        opertionInfo.opertionFileSize = total_file_size;
        totalInfo.preoccupationSize -= total_file_size;
        m_mount_operation_list->insert(mountRootName, totalInfo);
        m_operation_use_list->insert(operation, opertionInfo);

        m_progressbar->m_error = false;
        m_progressbar->showDelay(300);
    }, Qt::BlockingQueuedConnection);

    auto opType = operationInfo->operationType();
    switch (opType) {
    case FileOperationInfo::Trash:
    case FileOperationInfo::Delete: {
        allowParallel = true;
        auto operationSrcs = operationInfo->sources();
        auto currentOps = m_thread_pool->children();
        QList<FileOperation *> ops;
        for (auto child : currentOps) {
            auto op = qobject_cast<FileOperation *>(child);
            auto opInfo = op->getOperationInfo();
            {
                for (auto src : operationSrcs) {
                    if (opInfo->sources().contains(src)) {
                        Peony::AudioPlayManager::getInstance()->playWarningAudio();
                        //do not allow operation.
                        QMessageBox::critical(nullptr,
                                              tr("Can't delete."),
                                              tr("You can't delete a file when"
                                                 "the file is doing another operation"));
                        return;
                    }
                }
            }
        }
        break;
    }
    default:
        break;
    }

    // progress bar
   ProgressBar* proc = m_progressbar->addFileOperation();
   if (nullptr == proc) {
       qDebug() << "malloc error!";
       return;
   }

   // begin
   proc->connect(operation, &FileOperation::operationPreparedOne, proc, &ProgressBar::onElementFoundOne);
   proc->connect(operation, &FileOperation::operationPrepared, proc, &ProgressBar::onElementFoundAll);
   proc->connect(operation, &FileOperation::operationProgressedOne, proc, &ProgressBar::onFileOperationProgressedOne);
   proc->connect(operation, &FileOperation::FileProgressCallback, proc, &ProgressBar::updateProgress);
   proc->connect(operation, &FileOperation::operationProgressed, proc, &ProgressBar::onFileOperationProgressedAll);
   proc->connect(operation, &FileOperation::operationAfterProgressedOne, proc, &ProgressBar::onElementClearOne);
   proc->connect(operation, &FileOperation::operationAfterProgressed, proc, &ProgressBar::switchToRollbackPage);
   proc->connect(operation, &FileOperation::operationStartRollbacked, proc, &ProgressBar::switchToRollbackPage);
   proc->connect(operation, &FileOperation::operationRollbackedOne, proc, &ProgressBar::onFileRollbacked);
   proc->connect(operation, &FileOperation::operationStartSnyc, proc, &ProgressBar::onStartSync);
   proc->connect(operation, &FileOperation::operationFinished, proc, &ProgressBar::onFinished);
   proc->connect(proc, &ProgressBar::cancelled, operation, &FileOperation::cancel);
   proc->connect(proc, &ProgressBar::cancelled, operation, &FileOperation::operationCancel);
   proc->connect(proc, &ProgressBar::pause, operation, &FileOperation::operationPause);
   proc->connect(proc, &ProgressBar::resume, operation, &FileOperation::operationResume);

   operation->connect(operation, &FileOperation::errored, [=]() {
       if (operation->getOperationInfo()->m_type == FileOperationInfo::Copy || operation->getOperationInfo()->m_type == FileOperationInfo::Move) {
           return;
       }
       operation->setHasError(true);
   });

   operation->connect(operation, &FileOperation::errored, this, &FileOperationManager::handleError, Qt::BlockingQueuedConnection);
   operation->connect(operation, &FileOperation::operationFinished, this, [=](){
       //story 19796,后续数据处理
       if (m_operation_use_list->contains(operation)) {
           // Get the operation info
           currentOpertionInfo operationInfo = m_operation_use_list->value(operation);
           // Get the mount root name and total size of the operation
           QString name = operationInfo.mountRootName;
           quint64 size = operationInfo.opertionFileSize;
           // Calculate the new size of the mount operation list
           size = m_mount_operation_list->value(name).totalSize - size;
           if (size == 0){
               m_mount_operation_list->remove(name);
           } else {
               totalOperationInfo info;
               info.totalSize = size;
               info.preoccupationSize = m_mount_operation_list->value(name).preoccupationSize;
               if (operation->isCancelled()) {
                   info.preoccupationSize += operationInfo.opertionFileSize;
               }
               m_mount_operation_list->insert(name, info);
           }
           m_operation_use_list->remove(operation);
       }
       Q_EMIT this->operationFinished(operation->getOperationInfo(), !operation->hasError());
       if (operation->hasError()) {
           this->clearHistory();
           return ;
       }

       if (addToHistory) {
           auto info = operation->getOperationInfo();
           if (!info)
               return;
           if (info->m_type == FileOperationInfo::BatchRename) {
               info->m_type = FileOperationInfo::BatchRenameInternal;
           }
           if (info->m_is_search) {
               this->clearHistory();
               return;
           }
           if (info->operationType() != FileOperationInfo::Delete) {
               //fix bug#162024, play sound when operation finished
               if ((info->operationType() == FileOperationInfo::Copy ||
                   info->operationType() == FileOperationInfo::Move) &&
                    ! info->m_has_error){
                   //SoundEffect::getInstance()->copyOrMoveSucceedMusic();
                   //Task#152997, use sdk play sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
                   kdk::KSoundEffects::playSound(SoundType::OPERATION_FILE);
#endif
               }
               if(info->getOperationRecording()) {
                   m_undo_stack.push(info);
                   m_redo_stack.clear();
               }
           } else {
               this->clearHistory();
           }
       }
   }, Qt::BlockingQueuedConnection);

   // fix #92481
   if (auto trashOp = qobject_cast<FileTrashOperation *>(operation)) {
       connect(trashOp, &FileTrashOperation::deleteRequest, this, [=](const QStringList &srcUris){
           auto deleteOperation = new FileDeleteOperation(srcUris);
           // wait trash operation finished. otherwise manager will asume
           // the operation is invalid due to deletion checkment.
           QTimer::singleShot(1000, this, [=]{
               startOperation(deleteOperation, true);
           });
       }, Qt::BlockingQueuedConnection);
   }

    if (!allowParallel) {
        if (m_thread_pool->activeThreadCount() > 0) {
            QMessageBox::warning(nullptr,
                                 tr("File Operation is Busy"),
                                 tr("There have been one or more file"
                                    "operation(s) executing before. Your"
                                    "operation will wait for executing"
                                    "until it/them done. If you really "
                                    "want to execute file operations "
                                    "parallelly anyway, you can change "
                                    "the default option \"Allow Parallel\" "
                                    "in option menu."));
        }
        operation->setParent(m_thread_pool);
        m_thread_pool->start(operation);
    } else {
        operation->setParent(m_thread_pool);
        m_thread_pool->start(operation);
    }

    connect(operation, &FileOperation::operationWithoutRecording, this, [=]() {
        auto info = operation->getOperationInfo();
        if (info->getOperationRecording()) {
            info->setOperationRecording(false);
        }
    }, Qt::BlockingQueuedConnection);
    connect(operation, &FileOperation::operationSaveAsLongNameFile, this, [=](const QString &uri){
        if (operation->hasError()) {
            return;
        }
        QString text = QString(tr("The long name file is saved to %1")).arg(uri);
        QMessageBox::information(nullptr,nullptr,text);
    }, Qt::BlockingQueuedConnection);
    connect(operation, &FileOperation::operationInfoMsgBox, this, [=](const QString &msg){
        QString text = msg;
        QMessageBox::information(nullptr,nullptr,text);
    }, Qt::BlockingQueuedConnection);
    Q_EMIT this->operationStarted(operation->getOperationInfo());

    m_progressbar->showDelay();
}

void FileOperationManager::startUndoOrRedo(std::shared_ptr<FileOperationInfo> info)
{
    FileOperation *op = nullptr;
    switch (info->m_type) {
    case FileOperationInfo::Copy: {
        op = new FileCopyOperation(info->m_src_uris, info->m_dest_dir_uri);
        break;
    }
    case FileOperationInfo::Delete: {
        if (info->m_node_map.isEmpty())
            op = new FileDeleteOperation(info->m_src_uris);
        else
            op = new FileDeleteOperation(info->m_node_map.keys());
        break;
    }
    case FileOperationInfo::Link: {
        op = new FileLinkOperation(info->m_src_uris.at(0), info->m_dest_dir_uri);
        break;
    }
    case FileOperationInfo::Move: {
        op = new FileMoveOperation(info->m_src_uris, info->m_dest_dir_uri);
        auto moveOp = qobject_cast<FileMoveOperation *>(op);
        moveOp->setAction(info->m_drop_action);
        break;
    }
    case FileOperationInfo::Rename: {
        if (info->m_node_map.isEmpty()) {
            op = new FileRenameOperation(info->m_src_uris.isEmpty()? nullptr: info->m_src_uris.at(0),
                                         info->m_dest_dir_uri);
        } else {
            op = new FileRenameOperation(info->m_node_map.firstKey(), info.get()->m_newname);
        }
        break;
    }
    case FileOperationInfo::Trash: {
        op = new FileTrashOperation(info->m_src_uris);
        break;
    }
    case FileOperationInfo::Untrash: {
        op = new FileUntrashOperation(info->m_src_uris);
        break;
    }
    case FileOperationInfo::BatchRenameInternal: {
        op = new FileBatchRenameInternalOperation(info);
        break;
    }
    default:
        break;
    }
    //do not record the undo/redo operation to history again.
    //this had been handled at undo() and redo() yet.
    //FIXME: if an undo/redo work went error (usually won't),
    //should i remove the operation info from stack?
    if (op) {
        startOperation(op, false);
    }
}

bool FileOperationManager::canUndo()
{
    return !m_undo_stack.isEmpty();
}

bool FileOperationManager::canRedo()
{
    return !m_redo_stack.isEmpty();
}

std::shared_ptr<FileOperationInfo> FileOperationManager::getUndoInfo()
{
    return m_undo_stack.top();
}

std::shared_ptr<FileOperationInfo> FileOperationManager::getRedoInfo()
{
    return m_redo_stack.top();
}

void FileOperationManager::undo()
{
    if(!canUndo())
        return;

    auto undoInfo = m_undo_stack.pop();
    m_redo_stack.push(undoInfo);

    auto oppositeInfo = undoInfo->getOppositeInfo(undoInfo.get());
    startUndoOrRedo(oppositeInfo);
}

void FileOperationManager::redo()
{
    if (!canRedo())
        return;

    auto redoInfo = m_redo_stack.pop();
    m_undo_stack.push(redoInfo);

    startUndoOrRedo(redoInfo);
}

void FileOperationManager::clearHistory()
{
    m_undo_stack.clear();
    m_redo_stack.clear();
}

void FileOperationManager::onFilesDeleted(const QStringList &uris)
{
    qDebug()<<uris;
    //FIXME: improve the handling here of file deleted event.
    clearHistory();
}

// optimize: Gets Windows should be created conditionally and errors handled so that memory is allocated in the stack space
void FileOperationManager::handleError(FileOperationError &error)
{
    // Empty files in the recycle bin without reminding
    if (error.srcUri.startsWith("trash://")
        && FileOpDelete == error.op)
    {
        error.respCode = IgnoreAll;
        return;
    }

    // Handle errors according to the error type
    FileOperationErrorHandler* handle = FileOperationErrorDialogFactory::getDialog(error);
    if (nullptr != handle) {
        if (m_progressbar->isHidden()) {
            m_progressbar->m_error = true;
        }
#if 0
        auto dialog = dynamic_cast<QWidget *>(handle);
        if (dialog) {
            dialog->setProperty("useCustomShadow", true);
            dialog->setProperty("customShadowDarkness", 0.5);
            dialog->setProperty("customShadowWidth", 30);
            dialog->setProperty("customShadowRadius", QVector4D(1, 1, 1, 1));
            dialog->setProperty("customShadowMargins", QVector4D(30, 30, 30, 30));
        }
#endif
        handle->handle(error);
        delete handle;
        m_progressbar->m_error = false;
        m_progressbar->showDelay(300);
    }
}

void FileOperationManager::registerFileWatcher(FileWatcher *watcher)
{
    m_watchers<<watcher;
}

void FileOperationManager::unregisterFileWatcher(FileWatcher *watcher)
{
    m_watchers.removeOne(watcher);
}

void FileOperationManager::manuallyNotifyDirectoryChanged(FileOperationInfo *info)
{
    if (!info) {
        return;
    }

    // skip create template opeartion, it will be handled by operation itself.
    if (info->m_src_dir_uri == QStandardPaths::writableLocation(QStandardPaths::TempLocation)) {
        return;
    }

    for (auto watcher : m_watchers) {
        if (!watcher->supportMonitor()) {

            auto srcDir = info->m_src_dir_uri;
            auto destDir = info->m_dest_dir_uri;
            auto firstUri = info->m_src_uris.first();
            
            //'file:///run/user/1000/gvfs/smb-share:server=xxx,share=xxx/' converted to 'smb://xxx'
            GFile * file  = g_file_new_for_uri(destDir.toUtf8().data());
            char *uri = g_file_get_uri(file);	
            if (uri) {
                destDir = uri;
            }
            g_object_unref(file);
            g_free(uri);
            
            if (info->operationType() == FileOperationInfo::Link || info->operationType() == FileOperationInfo::Rename) {
                srcDir = FileUtils::getParentUri(firstUri);
            }

            if ("" == srcDir) {
                if (firstUri.endsWith("/")) {
                    firstUri.chop(1);
                }

                QStringList fileSplit = firstUri.split("/");
                fileSplit.pop_back();

                srcDir = fileSplit.join("/");
            }

            // check watcher directory
            // srcDir is null in samba filesystem, so that it not work
            // currentUri maybe is 'file:///run/user/1000/gvfs/smb-share:server=xxx,share=xxx/' or 'smb://xxx'
            if (watcher->currentUri() == srcDir || watcher->currentUri() == destDir) {
                // tell the view/model the directory should be updated
                watcher->requestUpdateDirectory();
            }

            qDebug() << "src:" << srcDir << " == dest:" << destDir << " == type:" << info->operationType();

            if (srcDir.startsWith("smb://")
                 || srcDir.startsWith("ftp://")
                 || srcDir.startsWith("sftp://")
                 || destDir.startsWith("smb://")
                 || destDir.startsWith("ftp://")
                 || destDir.startsWith("sftp://")) {
                if(!(watcher->currentUri().startsWith("search:///") &&
                     (info->operationType() == FileOperationInfo::Rename
                      ||info->operationType() == FileOperationInfo::Delete
                      || info->operationType() == FileOperationInfo::Trash
                      ||info->operationType() == FileOperationInfo::Move))){
                    watcher->requestUpdateDirectory();
                }
            }

            /* story23915 【文件管理器】搜索后的文件夹和文档右键剪切、删除、重命名选项 */
            if(watcher->currentUri().startsWith("search:///")){
                if(info->m_has_error){
                    watcher->requestUpdateDirectory();
                }else{
                    if(info->operationType() == FileOperationInfo::Delete || info->operationType() == FileOperationInfo::Trash){
                        for(auto& uri:info->sources()){
                            Q_EMIT watcher->fileDeleted(uri);
                        }
                    }
                    if(info->operationType() == FileOperationInfo::Rename){
                        for(auto& uri:info->sources()){
                            Q_EMIT watcher->fileRenamed(uri, destDir);
                        }
                    } else if (info->operationType() == FileOperationInfo::Move) {
                        for(auto& uri:info->sources()){
                            Q_EMIT watcher->fileDeleted(uri);
                        }
                    }
                }
            }//end
        }
    }

    /* 跨进程move操作，例如从搜索页面剪切或拖拽到桌面 */
    if (info->m_is_search && info->operationType() == FileOperationInfo::Move) {
        QStringList uris;
        if (!info->m_has_error) {
            uris = info->sources();
        }
        qDebug()<<"send moveFilesToAnotherProcCompleted signal"<<uris.size();
        QDBusMessage msg = QDBusMessage::createSignal("/org/lingmo/explor", "org.lingmo.explor", "moveFilesToAnotherProcCompleted");
        msg << uris;
        QDBusConnection::sessionBus().send(msg);
    }//end
}

void FileOperationManager::slot_opreateFinishedOfEngrampa(const QString &path, bool finish)
{
    if(!finish || path.isEmpty())
        return;

    if(!path.startsWith("smb://") && !path.startsWith("ftp://") && !path.startsWith("sftp://"))
        return;

    for (auto watcher : m_watchers) {
        if(watcher->supportMonitor())
            continue;
        QString watcherUri = watcher->currentUri();
        //'file:///run/user/1000/gvfs/smb-share:server=xxx,share=xxx/' converted to 'smb://xxx'
        if(watcherUri.startsWith("file:///run/user/1000/gvfs/smb-share:")){
            GFile * file  = g_file_new_for_uri(watcherUri.toUtf8().data());
            char *uri = g_file_get_uri(file);
            if (uri) {
                watcherUri = uri;
            }
            g_object_unref(file);
            g_free(uri);
        }
        //auto watcherDecodeUri = FileUtils::urlDecode(watcherUri);
        //auto destDecodePath = FileUtils::urlDecode(path);
        if (watcherUri == path || watcherUri == path + QString("/")){
            watcher->requestUpdateDirectory();
        }
    }
}

void FileOperationManager::slot_moveFilesToAnotherProcCompleted(const QStringList &srcUris)
{

    for (auto& watcher : m_watchers) {
        if(!watcher->currentUri().startsWith("search:///"))
            continue;

        if(watcher->supportMonitor())
            continue;

        if(srcUris.size()){
            for(auto& uri:srcUris){
                Q_EMIT watcher->fileDeleted(uri);
            }
        }else{
            watcher->requestUpdateDirectory();
        }
    }

}

//FIXME: get opposite info correcty.
FileOperationInfo::FileOperationInfo(QStringList srcUris,
                                     QString destDirUri,
                                     Type type,
                                     QObject *parent): QObject(parent)
{
    m_src_uris = srcUris;
    m_dest_dir_uri = destDirUri;

    oppositeInfoConstruct(type);
}

//FIXME: get opposite info correcty.
FileOperationInfo::FileOperationInfo(QStringList srcUris,
                                     QStringList destDirUris,
                                     Type type,
                                     QObject *parent): QObject(parent)
{
    m_src_uris = srcUris;
    m_dest_dir_uris = destDirUris;

    oppositeInfoConstruct(type);
}

void FileOperationInfo::oppositeInfoConstruct(Type type)
{
    m_type = type;

    switch (type) {
        case Move: {
            m_opposite_type = Move;
            commonOppositeInfoConstruct();
            break;
        }
        case Trash: {
            m_opposite_type = Untrash;
            trashOppositeInfoConstruct();
            break;
        }
        case Untrash: {
            m_opposite_type = Trash;
            UntrashOppositeInfoConstruct();
            break;
        }
        case Delete: {
            m_opposite_type = Other;
            break;
        }
        case Copy: {
            m_opposite_type = Delete;
            commonOppositeInfoConstruct();
            break;
        }
        case Rename: {
            m_opposite_type = Rename;
            RenameOppositeInfoConstruct();
            break;
        }
        case Link: {
            m_opposite_type = Delete;
            LinkOppositeInfoConstruct();
            break;
        }
        case CreateTxt: {
            m_opposite_type = Delete;
            commonOppositeInfoConstruct();
            break;
        }
        case CreateFolder: {
            m_opposite_type = Delete;
            commonOppositeInfoConstruct();
            break;
        }
        case CreateTemplate: {
            m_opposite_type = Delete;
            commonOppositeInfoConstruct();
            break;
        }
        case BatchRenameInternal:
        case BatchRename: {
            m_opposite_type = BatchRenameInternal;
            break;
        }
        default: {
            m_opposite_type = Other;
        }
    }

    return;
}

void FileOperationInfo::commonOppositeInfoConstruct()
{
    for (auto srcUri : m_src_uris) {
        auto srcFile = wrapGFile(g_file_new_for_uri(srcUri.toUtf8().constData()));
        if (m_src_dir_uri.isNull()) {
            auto srcParent = FileUtils::getFileParent(srcFile);
            m_src_dir_uri = FileUtils::getFileUri(srcParent);
        }
        QString relativePath = FileUtils::getFileBaseName(srcFile);
        auto destDirFile = wrapGFile(g_file_new_for_uri(m_dest_dir_uri.toUtf8().constData()));
        auto destFile = FileUtils::resolveRelativePath(destDirFile, relativePath);
        QString destUri = FileUtils::getFileUri(destFile);
        m_dest_uris<<destUri;
    }
}
void FileOperationInfo::LinkOppositeInfoConstruct()
{
    QUrl url = m_src_uris.first();
    if (url.fileName().startsWith(".")) {
        m_dest_uris<<m_dest_dir_uri + "/" + url.fileName() + " - " + tr("Symbolic Link");
    } else {
        auto dest_uri = m_dest_dir_uri + "/" + tr("Symbolic Link") + " - " + url.fileName();
        m_dest_uris<<dest_uri;
    }
}
void FileOperationInfo::RenameOppositeInfoConstruct()
{
    //Rename also use the common args format.
    QString src = m_src_uris.at(0);
    m_dest_uris<<src;
    m_src_dir_uri = m_dest_dir_uri;
}

void FileOperationInfo::BatchRenameOppositeInfoConstruct()
{
    m_dest_dir_uris = m_src_uris;
    m_src_dir_uri = m_dest_dir_uri;
}

void FileOperationInfo::UntrashOppositeInfoConstruct()
{
    m_dest_uris = m_dest_dir_uris;
    m_src_dir_uri = "trash:///";
    return;
}

void FileOperationInfo::trashOppositeInfoConstruct()
{
    // note that this function is unreliable.
    // the info would be updated while FileTrashOperation::run()
    // again.
    commonOppositeInfoConstruct();
}

std::shared_ptr<FileOperationInfo> FileOperationInfo::getOppositeInfo(FileOperationInfo *info) {

    auto oppositeInfo = std::make_shared<FileOperationInfo>(info->m_dest_uris, info->m_src_dir_uri, info->m_opposite_type);
    oppositeInfo->m_drop_action = info->m_drop_action;
    if (info->m_drop_action == Qt::TargetMoveAction) {
        //oppositeInfo->m_drop_action = Qt::TargetMoveAction;
        oppositeInfo->m_type = FileOperationInfo::Move;
    }
    QMap<QString, QString> oppsiteMap;
    for (auto key : m_node_map.keys()) {
        auto value = m_node_map.value(key);
        oppsiteMap.insert(value, key);
    }
    oppositeInfo->m_node_map = oppsiteMap;
    oppositeInfo->m_newname = this->m_oldname;
    oppositeInfo->m_oldname = this->m_newname;
    oppositeInfo->m_newnames = this->m_newnames;
    oppositeInfo->m_oldnames = this->m_oldnames;

    return oppositeInfo;
}

void FileOperationInfo::setOperationRecording(bool state)
{
    if (m_operation_recording != state) {
        m_operation_recording = state;
    }
}

// S3/S4
void FileOperationManager::systemSleep (GDBusConnection *connection, const gchar *senderName, const gchar *objectPath, const gchar *interfaceName, const gchar *signalName, GVariant *parameters, gpointer udata)
{
    FileOperationProgressBar* pb = static_cast<FileOperationManager*>(udata)->m_progressbar;
    if (pb) {
        Q_EMIT pb->pause();
    }

    Q_UNUSED(connection)
    Q_UNUSED(senderName)
    Q_UNUSED(objectPath)
    Q_UNUSED(signalName)
    Q_UNUSED(parameters)
    Q_UNUSED(interfaceName)
}
