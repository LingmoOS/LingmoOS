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

#include "file-item.h"
#include "file-enumerator.h"
#include "file-info-job.h"
#include "file-info-manager.h"
#include "file-watcher.h"
#include "file-utils.h"
#include "file-operation-utils.h"

#include "file-item-model.h"

#include "thumbnail-manager.h"
#include "emblem-provider.h"
#include "usershare-manager.h"

#include "gerror-wrapper.h"
#include "bookmark-manager.h"
#include "audio-play-manager.h"
#include "file-label-model.h"

#ifndef LINGMO_UDF_BURN
#include "disccontrol.h"
#else
#include <libkyudfburn/disccontrol.h>
using namespace UdfBurn;
#endif

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QUrl>
#include <QTimer>
#include <KWindowSystem>

#include <QApplication>

#include <QTime>

using namespace Peony;

static const int maxNumberOfDeletesByOne = 30;      /* 按个删除最大数量 */
static const int maxNumberOfDeletesPerBatch = 3000; /* 每次批量最多删除数量 */

QString uri2FavoriteUri(const QString &sourceUri)
{
    QUrl url = sourceUri;
    QString favoriteUri = "favorite://" + url.path() + "?schema=" + url.scheme();
    return favoriteUri;
}

FileItem::FileItem(std::shared_ptr<Peony::FileInfo> info, FileItem *parentItem, FileItemModel *model, QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QVector<FileItem*>* >("QVector<FileItem*>*");
    qRegisterMetaType<QHash<QString, FileItem*>>("QHash<QString, FileItem*>");
    qRegisterMetaType<std::vector<std::shared_ptr<FileInfo> >>("std::vector<std::shared_ptr<FileInfo> >&");

    m_parent = parentItem;
    m_info = info;
    m_children = new QVector<FileItem*>();
    m_uri_item_hash.clear();
    m_model = model;

    m_backend_enumerator = new FileEnumerator(this);

    m_batchProcessThread = new QThread();
    m_extraInfoRecorder = FileInfoManager::getInstance()->getExtraInfoRecorderByUri(m_info.get()->uri());

    m_addChildTimer = new QTimer(this);
    m_addChildTimer->setSingleShot(true);
    m_changeChildTimer = new QTimer(this);
    m_changeChildTimer->setSingleShot(true);

    m_idle = new QTimer(this);
    m_idle->setInterval(30);
    m_idle->setSingleShot(true);
    connect(m_idle, &QTimer::timeout, this, [=]{
        if (m_uris_to_be_removed.isEmpty())
            return;

        QStringList favoriteUris;
        if (m_uris_to_be_removed.count() < maxNumberOfDeletesByOne && !m_batchProcessThread->isRunning()) {
            // do normal remove
            for (auto uri : m_uris_to_be_removed) {
                for (int row = 0; row < m_children->count(); row++) {
                    auto child = m_children->at(row);
                    // 此处实际可靠性还有待验证
                    if (FileUtils::isSamePath(uri, child->uri())) {
                        auto info = child->m_info;
                        if (info->isDir())
                        {
                            favoriteUris.append(uri2FavoriteUri(uri));
                        }
                        m_model->beginRemoveRows(this->firstColumnIndex(), row, row);
                        m_uris_to_be_removed.removeOne(uri);
                        m_children->remove(row);
                        m_uri_item_hash.remove(child->uri());
                        m_model->endRemoveRows();
                        FileLabelModel::getGlobalModel()->removeFileLabel(uri);
                        delete child;
                        break;
                    }
                }
            }
            m_model->updated();
            BookMarkManager::getInstance()->removeBookMark(favoriteUris);
            return;
        }

        // do reset model
        batchRemoveItems();

    });

    m_thumbnail_watcher = std::make_shared<Peony::FileWatcher>("thumbnail://");
    connect(m_thumbnail_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri){
        auto index = m_model->indexFromUri(uri);
        if (index.isValid()) {
            auto item = m_model->itemFromIndex(index);
            if (item) {
                /*!
                  \note
                  fix the probabilistic jamming while thumbnailing with list view.

                  we have to only trigger first column index dataChanged signal,
                  otherwise there will be probility stucked whole program.

                  i'm not sure if it is a bug of qtreeview.
                  */

                //m_model->dataChanged(item->firstColumnIndex(), item->lastColumnIndex());
                //m_model->dataChanged(item->firstColumnIndex(), item->firstColumnIndex());
                m_model->updated();
                m_model->thumbnailUpdated(uri);
            }
        }
    });

    // avoid call any method when model is deleted.
    setParent(m_model);
}

FileItem::~FileItem()
{
    //qDebug()<<"~FileItem"<<m_info->uri();
    // try fix #164883, error cusor while searching
    if (!property("isCancelled").toBool()) {
        Q_EMIT cancelFindChildren();
    }
    //disconnect();

    for (auto child : *m_children) {
        delete child;
    }
    m_children->clear();

    delete m_children;
    m_uri_item_hash.clear();

    if (m_batchProcessThread->isRunning()) {
        m_batchProcessThread->quit();
        m_batchProcessThread->wait();
    }
    m_batchProcessThread->deleteLater();

    disconnect(m_model->m_fileManagerThread, &FileManagerThread::finishQueryFileInfos, this, nullptr);

}

bool FileItem::operator==(const FileItem &item)
{
    //qDebug()<<m_info->uri()<<item.m_info->uri();
    return this->m_info->uri() == item.m_info->uri();
}

const QString FileItem::uri()
{
    return m_info->uri();
}

#include"file-operation-manager.h"

QVector<FileItem*> *FileItem::findChildrenSync()
{
    Q_EMIT m_model->findChildrenStarted();
    std::shared_ptr<Peony::FileEnumerator> enumerator = std::make_shared<Peony::FileEnumerator>();
    enumerator->setEnumerateDirectory(m_info->uri());
    enumerator->enumerateSync();
    auto infos = enumerator->getChildren();
    for (auto info : infos) {
        FileItem *child = new FileItem(info, this, m_model);
        m_children->append(child);
        m_uri_item_hash.insert(child->uri(), child);
        FileInfoJob *job = new FileInfoJob(info);
        job->setAutoDelete();
        job->querySync();
    }
    //qDebug() << "FileItem findChildrenSync";
    Q_EMIT m_model->findChildrenFinished();
    return m_children;
}

void FileItem::findChildrenAsync()
{
    auto info = FileInfo::fromUri(m_info.get()->uri());
    auto infoJob = new FileInfoJob(info);
    infoJob->setAutoDelete();
    infoJob->queryAsync();

    if (m_expanded)
        return;

    Q_EMIT m_model->findChildrenStarted();
    m_expanded = true;
    Peony::FileEnumerator *enumerator = new Peony::FileEnumerator;
    enumerator->setEnumerateDirectory(m_info->uri());
    //NOTE: entry a new root might destroyed the current enumeration work.
    //the root item will be delete, so we should cancel the previous enumeration.
    enumerator->connect(this, &FileItem::cancelFindChildren, enumerator, &FileEnumerator::cancel);
    enumerator->connect(enumerator, &FileEnumerator::cancelled, m_model, [=](){
        if (enumerator->getEnumerateUri() != m_model->getRootUri()) {
            // try fix #164883, error cusor while searching
            return;
        }
        m_model->findChildrenFinished();
    });
    enumerator->connect(enumerator, &FileEnumerator::prepared, this, [=](std::shared_ptr<GErrorWrapper> err, const QString &targetUri, bool critical) {
        if (critical) {
            if (G_IO_ERROR_NOT_FOUND == err->code() || G_IO_ERROR_EXISTS == err->code()) {
                QMessageBox::warning(nullptr, tr("Warning"), err->message());
            } else {
                QMessageBox::critical(nullptr, tr("Error"), err->message());
            }
            //QMessageBox::critical(nullptr, tr("Error"), err->message());
            //Peony::AudioPlayManager::getInstance()->playWarningAudio();
            enumerator->cancel();
            //fix bug#77594
            enumerator->deleteLater();
            return;
        }

        if (!targetUri.isNull()) {
            if (targetUri != this->uri()) {
                this->m_info = FileInfo::fromUri(targetUri);

                GFile *targetFile = g_file_new_for_uri(targetUri.toUtf8().constData());
                QUrl targetUrl = targetUri;
                auto path = g_file_get_path(targetFile);
                enumerator->setEnumerateDirectory(targetFile);
                g_object_unref(targetFile);
            }

            enumerator->enumerateAsync();
            return;
        }

        auto target = FileUtils::getTargetUri(m_info->uri());
        if (!target.isEmpty()) {
            enumerator->cancel();
            //enumerator->deleteLater();

            //! \note fix direct setRootUri() prevents view switch error
            m_model->setRootUri(target);
            //m_model->sendPathChangeRequest(target, this->uri());
            return;
        }

        if (err) {
            qDebug()<<"file item error:" <<err->message()<<enumerator->getEnumerateUri();
            //Peony::AudioPlayManager::getInstance()->playWarningAudio();

            //fix bug#214724, 214924， access smb-root error issue
            if ((err.get()->code() == G_IO_ERROR_NOT_FOUND || err.get()->code() == G_IO_ERROR_PERMISSION_DENIED) &&
                    this->uri() != "smb:///" && this->uri() != "network:///smb-root") {
                enumerator->cancel();
                //fix goto removed path in case device is ejected
                if (this->uri().startsWith("file:///media"))
                {
                    //check bookmark and delete
                    BookMarkManager::getInstance()->removeBookMark(uri2FavoriteUri(this->uri()));
                    FileLabelModel::getGlobalModel()->removeFileLabel(this->uri());
                    m_model->sendPathChangeRequest("computer:///", this->uri());
                }
                else
                    //! \note fix direct setRootUri() prevents view switch error
                    // m_model->setRootUri(FileUtils::getParentUri(this->uri()));
                    m_model->sendPathChangeRequest(FileUtils::getParentUri(this->uri()), this->uri());

                auto fileInfo = FileInfo::fromUri(this->uri());
                if (err.get()->code() == G_IO_ERROR_NOT_FOUND && fileInfo->isSymbolLink())
                {
                    auto result = QMessageBox::question(nullptr, tr("Open Link failed"),
                                          tr("File not exist, do you want to delete the link file?"),
                                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    if (result == QMessageBox::Yes) {
                        qDebug() << "Delete unused symbollink.";
                        QStringList selections;
                        selections.push_back(this->uri());
                        FileOperationUtils::trash(selections, true, this->uri().startsWith("search:///"));
                    }
                }
                else if (err.get()->code() == G_IO_ERROR_PERMISSION_DENIED)
                {
                    QMessageBox *msgBox = new QMessageBox();
                    msgBox->setWindowTitle(tr("Error"));
                    QString errorInfo = tr("Can not open path \"%1\"，permission denied.").arg(this->uri().unicode());
                    msgBox->setText(errorInfo);
                    msgBox->setModal(false);
                    msgBox->setAttribute(Qt::WA_DeleteOnClose);
                    KWindowSystem::setState(msgBox->winId(), KWindowSystem::KeepAbove);
                    msgBox->show();
                   //QMessageBox::critical(nullptr, tr("Error"), errorInfo);
                }
                else if(err.get()->code() == G_IO_ERROR_NOT_FOUND)
                {
                    QString errorInfo = tr("Can not find path \"%1\"，are you moved or renamed it?").arg(fileInfo->uri().unicode());
                    QMessageBox::critical(nullptr, tr("Error"), errorInfo);
                }
                enumerator->deleteLater();
                return;
            }
            else {
                enumerator->cancel();
                enumerator->deleteLater();
                QMessageBox::critical(nullptr, tr("Error"), err->message());
                return;
            }
        }
        enumerator->enumerateAsync();
    });

    if (!m_model->isPositiveResponse()) {
        enumerator->connect(enumerator, &Peony::FileEnumerator::enumerateFinished, this, [=](bool successed) {
            if (successed) {
                auto infos = enumerator->getChildren();
                m_async_count = infos.count();
                if (infos.count() == 0) {
                    Q_EMIT m_model->findChildrenFinished();
                }

                for (auto info : infos) {
                    FileItem *child = new FileItem(info, this, m_model);
                    m_children->prepend(child);
                    m_uri_item_hash.insert(child->uri(), child);
                    FileInfoJob *job = new FileInfoJob(info);
                    job->setAutoDelete();
                    /*
                    FileInfo *shared_info = info.get();
                    int row = infos.indexOf(info);
                    //qDebug()<<info->uri()<<row;
                    job->connect(job, &FileInfoJob::infoUpdated, this, [=](){
                        qDebug()<<shared_info->iconName()<<row;
                    });
                    */
                    connect(job, &FileInfoJob::queryAsyncFinished, this, [=]() {
                        //the query job is finished and will be deleted soon,
                        //whatever info was updated, we need decrease the async count.
                        m_async_count--;
                        if (m_async_count == 0) {
                            m_model->insertRows(0, m_children->count(), this->firstColumnIndex());
                            Q_EMIT this->m_model->findChildrenFinished();
                            Q_EMIT m_model->updated();
                            for (auto info : infos) {
                                ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher);
                            }
                        }
                    });

                    connect(job, &FileInfoJob::infoUpdated, child, [=](){
                        m_model->dataChanged(child->firstColumnIndex(), child->lastColumnIndex());
                    });

                    infoJob->connect(this, &FileItem::cancelFindChildren, infoJob, &FileInfoJob::cancel);

                    job->queryAsync();
                }
            } else {
                //qDebug() << "enumerateFinished false" <<successed;
                Q_EMIT m_model->findChildrenFinished();
                return;
            }


            /* 如果是R类型光盘，遍历完当前目录后，再遍历将要刻录的缓冲数据加入item的m_children列表 */
            showFilesForBurningOnRTypeDisc();

            enumerator->cancel();
            delete enumerator;

            m_watcher = std::make_shared<FileWatcher>(this->m_info->uri(), nullptr, true);
            m_watcher->setMonitorChildrenChange(true);
            connect(m_watcher.get(), &FileWatcher::fileCreated, this, [=](QString uri) {
                //add new item to m_children
                //tell the model update
                this->onChildAdded(uri);
                Q_EMIT this->childAdded(uri);
                qDebug() << "inpositive Model onChildAdded:" <<uri;
            });
            connect(m_watcher.get(), &FileWatcher::fileDeleted, this, [=](QString uri) {
                this->onChildRemoved(uri);
            });
            connect(m_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri) {
                onChanged(uri);
            });
            connect(m_watcher.get(), &FileWatcher::fileRenamed, this, [=](const QString &oldUri, const QString &newUri) {
                Q_EMIT this->renamed(oldUri, newUri);
                this->onRenamed(oldUri, newUri);
            });
            connect(m_watcher.get(), &FileWatcher::thumbnailUpdated, this, [=](const QString &uri) {
                m_model->dataChanged(m_model->indexFromUri(uri), m_model->indexFromUri(uri));
            });
            connect(m_watcher.get(), &FileWatcher::directoryDeleted, this, [=](QString uri) {
                //clean all the children, if item index is root index, cd up.
                //this might use FileItemModel::setRootItem()
                Q_EMIT this->deleted(uri);
                this->onDeleted(uri);
            });

            connect(m_watcher.get(), &FileWatcher::locationChanged, this, [=](QString oldUri, QString newUri) {
                //this might use FileItemModel::setRootItem()
                Q_EMIT this->renamed(oldUri, newUri);
                this->onRenamed(oldUri, newUri);
            });

            connect(m_watcher.get(), &FileWatcher::directoryUnmounted, this, [=](const QString &sourceUri) {
                m_model->sendPathChangeRequest("computer:///", sourceUri);
            });
            //qDebug()<<"startMonitor";

            connect(m_watcher.get(), &FileWatcher::requestUpdateDirectory, this, &FileItem::onUpdateDirectoryRequest);
            m_watcher->startMonitor();
        });
    } else {
        enumerator->connect(enumerator, &Peony::FileEnumerator::childrenUpdated, this, [=](const QStringList &uris, bool isEnding) {
            //qDebug()<<"FileEnumerator::childrenUpdated:"<<uris.size()<<isEnding;
            if (uris.isEmpty()) {
                if (isEnding) {
                    //qDebug() << "enumerateFinished childrenUpdated:" <<isEnding;
                    m_isEndOfEnumerate = isEnding;
                    if(!m_ending_uris.size()){
                        Q_EMIT m_model->findChildrenFinished();
                        Q_EMIT m_model->updated();
                    }
                }
            }

            if (!m_children) {
                enumerator->disconnect();
                delete enumerator;
                return ;
            }

            childrenUpdateOfEnumerate(uris, isEnding);
        });

        enumerator->connect(enumerator, &Peony::FileEnumerator::enumerateFinished, this, [=](bool successed) {
            delete enumerator;

            if (!successed) {
                Q_EMIT m_model->findChildrenFinished();
                return;
            }

            if (!m_model||!m_children||!m_info)
                return;

            /* 如果是R类型光盘，遍历完当前目录后，再遍历将要刻录的缓冲数据加入item的m_children列表 */
            showFilesForBurningOnRTypeDisc();

            m_watcher = std::make_shared<FileWatcher>(this->m_info->uri(), nullptr, true);
            m_watcher->setMonitorChildrenChange(true);
            connect(m_watcher.get(), &FileWatcher::fileCreated, this, [=](QString uri) {
                //add new item to m_children
                //tell the model update
                this->onChildAdded(uri);
                Q_EMIT this->childAdded(uri);
                //qDebug() << "positive onChildAdded:" <<uri;
                //file changed, force create thubnail, link tobug#83108
                //ThumbnailManager::getInstance()->createThumbnail(uri, m_thumbnail_watcher, true);
            });
            connect(m_watcher.get(), &FileWatcher::fileDeleted, this, [=](QString uri) {
                this->onChildRemoved(uri);
            });
            connect(m_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri) {
               onChanged(uri);
            });
            connect(m_watcher.get(), &FileWatcher::fileRenamed, this, [=](const QString &oldUri, const QString &newUri) {
                this->onRenamed(oldUri, newUri);
                FileLabelModel::getGlobalModel()->fileLabelRenamed(oldUri, newUri);
                BookMarkManager::getInstance()->bookmarkChanged(oldUri, newUri);
            });
            connect(m_thumbnail_watcher.get(), &FileWatcher::thumbnailUpdated, this, [=](const QString &uri) {
                m_model->updated();
                //m_model->dataChanged(m_model->indexFromUri(uri), m_model->indexFromUri(uri));
            }, Qt::UniqueConnection);
            connect(m_watcher.get(), &FileWatcher::directoryDeleted, this, [=](QString uri) {
                //clean all the children, if item index is root index, cd up.
                //this might use FileItemModel::setRootItem()
                Q_EMIT this->deleted(uri);
                this->onDeleted(uri);
            });
            connect(m_watcher.get(), &FileWatcher::locationChanged, this, [=](QString oldUri, QString newUri) {
                //this might use FileItemModel::setRootItem()
                Q_EMIT this->renamed(oldUri, newUri);
                this->onRenamed(oldUri, newUri);
            });

            connect(m_watcher.get(), &FileWatcher::directoryUnmounted, this, [=](const QString &sourceUri) {
                m_model->sendPathChangeRequest("computer:///", sourceUri);
            });
            //qDebug()<<"startMonitor";
            connect(m_watcher.get(), &FileWatcher::requestUpdateDirectory, this, &FileItem::onUpdateDirectoryRequest);
            m_watcher->startMonitor();
        });
    }

    enumerator->prepare();
}

QModelIndex FileItem::firstColumnIndex()
{
    return m_model->firstColumnIndex(this);
}

QModelIndex FileItem::lastColumnIndex()
{
    return m_model->lastColumnIndex(this);
}

bool FileItem::hasChildren()
{
    //qDebug()<<"has children"<<m_info->uri()<<(m_info->isDir() || m_info->isVolume() || m_children->count() > 0);
    return m_info->isDir() || m_info->isVolume() || m_children->count() > 0;
}

FileItem *FileItem::getChildFromUri(QString uri)
{
    // optimize
    auto index = m_model->indexFromUri(uri);
    if (index.isValid()) {
        return m_model->itemFromIndex(index);
    }
    return nullptr;
    /*
    for (auto item : *m_children) {
        QUrl itemUrl = item->uri();
        QUrl url = uri;
        QString decodedUri = url.toDisplayString();
        if (decodedUri == itemUrl.toDisplayString())
            return item;
    }
    return nullptr;
    */
}

void FileItem::onChildAdded(const QString &uri)
{
    if(m_uris_to_be_removed.contains(uri))
        m_uris_to_be_removed.removeOne(uri);

    //qDebug()<<"add child:" << uri;
    FileItem *child = getChildFromUri(uri);
    if (child) {
        qDebug()<<"has added, return";
        //child info maybe changed, so need sync update again
        child->updateInfoAsync();
        //m_model->updated();
        return;
    }

    m_waiting_add_queue.append(uri);
    m_addChildTimer->start();
    return;
}

void FileItem::onChildRemoved(const QString &uri)
{
    // fix #62925
    m_waiting_add_queue.removeOne(uri);
    m_uris_to_be_removed.append(uri);
    if(!m_idle->isActive())
        m_idle->start();

    if (m_uris_to_be_removed.count() == 1) {
        auto info = FileInfo::fromUri(uri);
        if (info->isDir()) {
            QString displayName = info->displayName();
            if (UserShareInfoManager::getInstance()->getUsershareLists().contains(displayName)) {
                SharedDeleteInfoThread *thread = new SharedDeleteInfoThread(info->uri());
                connect(thread, &SharedDeleteInfoThread::finished, thread, &SharedDeleteInfoThread::deleteLater);
                thread->start();
            }
        }
    }
    return;
}

void FileItem::onDeleted(const QString &thisUri)
{
    qDebug()<<"deleted";
    //FIXME: when a mount point unmounted, it was aslo assumed as "deleted",
    //in this case we should not delete this item here.
    //actually i don't think this desgin is good enough. maybe there is a
    //better choice.
    //another problem is that if we just clear the children of this item,
    //it will be "unexpandable". but the item index hasChildren() is true.
    //doublue clicked twice it will be expanded. a qt's bug?
    if (m_parent) {
        if (m_parent->m_info->uri() == thisUri) {
            m_model->removeRow(m_parent->m_children->indexOf(this), m_parent->firstColumnIndex());
            m_parent->m_children->removeOne(this);
            m_parent->m_uri_item_hash.remove(this->uri());
        } else {
            //if just clear children, there will be a small problem.
            clearChildren();
            m_model->removeRow(m_parent->m_children->indexOf(this), m_parent->firstColumnIndex());
            m_parent->m_children->removeOne(this);
            m_parent->m_uri_item_hash.remove(this->uri());
            m_parent->onChildAdded(m_info->uri());
        }
        this->deleteLater();
    } else {
        //cd up.
        auto tmpItem = this;
        auto tmpUri = FileUtils::getParentUri(tmpItem->uri());
        while(tmpItem && tmpUri.isNull()) {
            tmpUri = FileUtils::getParentUri(tmpUri);
            tmpItem = tmpItem->m_parent;
        }
        if (!tmpUri.isNull()) {
            if(tmpUri.startsWith("file:///media")){
                m_model->sendPathChangeRequest("computer:///", tmpItem->uri());
            }else if(tmpUri.startsWith("file:///run/user")){
                m_model->sendPathChangeRequest("computer:///", thisUri);
            }else{
                m_model->setRootUri(tmpUri);
            }
        } else {
            //! \note Fix direct setRootUri() prevents view switch error
            // m_model->setRootUri("file:///");
            m_model->sendPathChangeRequest("file:///", tmpItem->uri());
        }
    }
    m_model->updated();
}

void FileItem::onRenamed(const QString &oldUri, const QString &newUri)
{
    qDebug()<<"renamed";

    // fix #77076, which caused by idle removing item due to #66255.
    if (m_uris_to_be_removed.contains(newUri)) {
        m_uris_to_be_removed.removeOne(newUri);
    }

    // note that some times new file has arealy in directory view,
    // and there is no delete event triggered. for example. copy
    // a .desktop file in current view. in this case there might
    // be an outdated tmp file left.
    //
    // to avoid that we add an existing checkment, and handle old
    // file with different situation.
    auto newChild = getChildFromUri(newUri);
    if (newChild) {
        qDebug()<<"new child has arealy in view";
        newChild->updateInfoAsync();
    }

    FileItem *child = getChildFromUri(oldUri);
    if (child) {
        if (!newChild) {
            int index = m_children->indexOf(child);
            m_children->at(index)->m_info= FileInfo::fromUri(newUri);
            m_uri_item_hash.remove(oldUri);
            child->updateInfoAsync();
            m_uri_item_hash.insert(child->uri(), child);

        } else {
            m_model->beginRemoveRows(this->firstColumnIndex(), m_children->indexOf(child), m_children->indexOf(child));
            m_children->removeOne(child);
            m_uri_item_hash.remove(child->uri());
            child->deleteLater();
            m_model->endRemoveRows();
        }
    }
}

void FileItem::onChanged(const QString &uri)
{
    m_waiting_update_queue.append(uri);
    if (!m_changeChildTimer->isActive()) {
        m_changeChildTimer->start(100);
    }
}

void FileItem::onUpdateDirectoryRequest()
{
    auto enumerator = new FileEnumerator(this);
    enumerator->setEnumerateDirectory(m_model->getRootUri());
    connect(enumerator, &FileEnumerator::enumerateFinished, m_model, [=](){
        if (m_model->getRootUri() != enumerator->getEnumerateUri())
            return;

        auto currentUris = enumerator->getChildrenUris();
        QStringList rawUris;
        QStringList removedUris;
        QStringList addedUris;
        QVector<FileItem*> removeFileItem ;

        for (auto child : *m_model->m_root_item->m_children) {
            rawUris<<child->uri();
            if (!currentUris.contains(child->uri())) {
                removedUris<<child->uri();
                removeFileItem.append(child);
                //m_model->m_root_item->onChildRemoved(child->uri());
            }
        }

        for (auto item : removeFileItem ) {
            m_model->m_root_item->onChildRemoved(item->uri());
        }


        for (auto uri : currentUris) {
            if (!rawUris.contains(uri)) {
                addedUris<<uri;
                m_model->m_root_item->onChildAdded(uri);
            }
        }

        for (auto uri : currentUris) {
            if (!addedUris.contains(uri) && !removedUris.contains(uri)) {
                //m_model->m_root_item->getChildFromUri(uri)->updateInfoAsync();
            }
        }

        enumerator->deleteLater();
    });

    enumerator->enumerateAsync();
}

void FileItem::updateInfoSync()
{
    FileInfoJob *job = new FileInfoJob(m_info);
    if (job->querySync()) {
        //m_model->dataChanged(this->firstColumnIndex(), this->lastColumnIndex());
        m_model->updated();
        ThumbnailManager::getInstance()->createThumbnail(this->uri(), m_thumbnail_watcher, true);
    }
    job->deleteLater();
}

void FileItem::updateInfoAsync()
{
    FileInfoJob *job = new FileInfoJob(m_info);
    job->setAutoDelete();
    job->connect(job, &FileInfoJob::infoUpdated, this, [=]() {
        //m_model->dataChanged(this->firstColumnIndex(), this->lastColumnIndex());
        m_model->updated();
        ThumbnailManager::getInstance()->createThumbnail(this->uri(), m_thumbnail_watcher, true);
    });

    job->connect(this, &FileItem::cancelFindChildren, job, &FileInfoJob::cancel);

    job->queryAsync();
}

void FileItem::removeChildren()
{

}

void FileItem::batchRemoveItems()
{
    /* 批量+异步方式解决大量数据删除时卡顿界面问题，link to bug#112062 删除一万个文件到回收站，文件管理器卡死，但删除成功 */
    if(!m_batchProcessThread->isRunning()){
        QStringList list;
        if(m_uris_to_be_removed.size() >= maxNumberOfDeletesPerBatch){/* 每次批量最多删除数量 */
            for(int i = 0; i < maxNumberOfDeletesPerBatch; i++){
                QString uri = m_uris_to_be_removed.takeFirst();
                list.append(uri);
            }
        }else{
            list.swap(m_uris_to_be_removed);
        }

        m_batchProcessItems = new BatchProcessItems();
        m_batchProcessItems->setBatchRemoveParam(list, m_uri_item_hash, m_children);
        m_batchProcessItems->moveToThread(m_batchProcessThread);
        connect(m_batchProcessThread, &QThread::started, m_batchProcessItems, &BatchProcessItems::slot_removeItems);
        connect(m_batchProcessItems, &BatchProcessItems::removeItemsFinished, this, [=](QVector<FileItem*> *children, const QHash<QString, FileItem*> &uri_item_hash, const QVector<QString>& needHandleLabelUris){
            FileLabelModel::getGlobalModel()->removeFileLabel(needHandleLabelUris);
            m_model->beginResetModel();
            auto old = m_children;
            m_children = children;
            delete old;
            m_uri_item_hash = uri_item_hash;
            m_model->endResetModel();
            m_model->updated();/* 更新状态栏 */
            qDebug()<<"remove items finished, children count,uri_item_hash count:"<<m_children->size()<<m_uri_item_hash.size();

            m_batchProcessThread->quit();
            if(m_batchProcessItems){
                delete m_batchProcessItems;
                m_batchProcessItems = nullptr;
            }

            if (m_uris_to_be_removed.size()>0 && !m_idle->isActive()) {
                m_idle->start();
            }

        }, Qt::BlockingQueuedConnection);
        m_batchProcessThread->start();
    } else {
        if (m_uris_to_be_removed.size()>0 && !m_idle->isActive()) {
            m_idle->start();
        }
    }
}

void FileItem::showFilesForBurningOnRTypeDisc()
{
#ifdef LINGMO_UDF_BURN
    QString unixDevice = m_info.get()->unixDeviceFile();
    if(!unixDevice.startsWith("/dev/sr"))
        return;

    DiscControl *discControl = new DiscControl(unixDevice);
    if(discControl->work()){
       connect(discControl, &DiscControl::workFinished, [=](DiscControl *discCtrl){
           if(discControl->isAllRType()){
               m_isRTypeDisc = true;
               FileEnumerator e;
               QString parentDirForBurnFiles = "file://" + QDir::homePath()+"/.cache/LingmoTransitBurner/";/* 例：file:///home/lingmo/.cache/LingmoTransitBurner */
               e.setEnumerateDirectory(parentDirForBurnFiles);
               e.enumerateSync();
               for(auto &fileInfo : e.getChildren()){
                   auto info = FileInfo::fromUri(fileInfo.get()->uri());
                   auto infoJob = new FileInfoJob(info);
                   infoJob->setAutoDelete();
                   infoJob->connect(infoJob, &FileInfoJob::infoUpdated, this, [=]() {
                       if(m_uri_item_hash.contains(info.get()->uri()))
                           return;
                       auto item = new FileItem(info, this, m_model);
                       item->setProperty("isFileForBurning", true);/* 所有"/home/家目录/.cache/LingmoTransitBurner/"中的子文件展示在挂载点时都应该半透明显示，区别于普通文件 */
                       m_model->beginInsertRows(QModelIndex(), m_children->count(), m_children->count());
                       m_children->append(item);
                       m_uri_item_hash.insert(item->uri(), item);
                       /* 解决：升级上来的版本点击标记以后无法显示原来已有的标记文件（兼容性问题） */
                       if(!item->uri().startsWith("label://")){
                           QList<int> labelIds = FileLabelModel::getGlobalModel()->getFileLabelIds(item->uri());
                           for(auto &labelId: labelIds){
                               if(labelId <= 0)
                                   continue;
                               FileLabelModel::getGlobalModel()->addLabelToFile(item->uri(), labelId);
                           }
                       }//end
                       m_model->endInsertRows();
                       ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher);
                   });

                   infoJob->connect(this, &FileItem::cancelFindChildren, infoJob, &FileInfoJob::cancel);

                   infoJob->queryAsync();
               }
               /* 监听 */
               m_rTypeDiscWatcher = std::make_shared<FileWatcher>(parentDirForBurnFiles, nullptr, true);
               m_rTypeDiscWatcher->setMonitorChildrenChange(true);
               connect(m_rTypeDiscWatcher.get(), &FileWatcher::fileCreated, this, [=](QString uri) {
                   this->onChildAdded(uri);
                   Q_EMIT this->childAdded(uri);
                   ThumbnailManager::getInstance()->createThumbnail(uri, m_thumbnail_watcher, true);
               });
               connect(m_rTypeDiscWatcher.get(), &FileWatcher::fileDeleted, this, [=](QString uri) {
                   this->onChildRemoved(uri);
               });
               connect(m_rTypeDiscWatcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri) {
                  onChanged(uri);
               });
               connect(m_rTypeDiscWatcher.get(), &FileWatcher::fileRenamed, this, [=](const QString &oldUri, const QString &newUri) {
                   this->onRenamed(oldUri, newUri);
                   BookMarkManager::getInstance()->bookmarkChanged(oldUri, newUri);
               });
               connect(m_rTypeDiscWatcher.get(), &FileWatcher::thumbnailUpdated, this, [=](const QString &uri) {
                   m_model->updated();
               });
               m_rTypeDiscWatcher->startMonitor();
           }
           if(discControl){
               discControl->deleteLater();
           }
       });

    }
#endif
}

void FileItem::connectFunc()
{
    connect(m_model->m_fileManagerThread, &FileManagerThread::finishQueryFileInfos, this, [=](std::vector<std::shared_ptr<FileInfo> >& retFileInfos, /*FileItemModel::OperateType*/int operateType, FileItem *parentItem){
        /* 查询结果返回，更新数据 */
        //qDebug()<<retFileInfos.size()<<this<<this->uri()<<operate<<m_ending_uris.size();
        if(parentItem && this != parentItem)
            return;

        FileInfoManager *info_manager = FileInfoManager::getInstance();
        if(FileItemModel::OperateType::Enumerate == FileItemModel::OperateType(operateType)){/* 遍历或搜索 */
            m_model->beginInsertRows(QModelIndex(), m_children->count(), m_children->count() + retFileInfos.size() -1);
            for (auto& info : retFileInfos) {
                info_manager->lock();
                info_manager->updateFileInfo(info);
                info_manager->unlock();

                auto item = new FileItem(info, this, m_model);
                m_children->append(item);
                m_uri_item_hash.insert(item->uri(), item);
                m_ending_uris.removeOne(info->uri());
                ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher);
                EmblemProviderManager::getInstance()->queryAsync(info->uri());

            }
            m_model->endInsertRows();
            Q_EMIT m_model->updated();/* 更新状态栏 */
            if (m_isEndOfEnumerate && m_ending_uris.isEmpty()) {
                //qDebug()<<"findChildrenFinished"<<m_children->size()<<m_isEndOfEnumerate<<m_ending_uris.size();
                if(this->uri().startsWith("search:///") && m_children->size()>30000)/* 搜索数量超过阈值不排序 */
                    return;
                Q_EMIT m_model->findChildrenFinished();
            }

        }else if(FileItemModel::OperateType::Add == FileItemModel::OperateType(operateType)){/* 新增 */
            m_model->beginInsertRows(QModelIndex(), m_children->count(), m_children->count() + retFileInfos.size() -1);
            for (auto& info : retFileInfos) {
                info_manager->lock();
                info_manager->updateFileInfo(info);
                info_manager->unlock();

                auto item = new FileItem(info, this, m_model);
                m_children->append(item);
                m_uri_item_hash.insert(item->uri(), item);
                ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher);
                EmblemProviderManager::getInstance()->queryAsync(info->uri());
            }
            m_model->endInsertRows();
            Q_EMIT m_model->updated();/* 更新状态栏 */
        }else if(FileItemModel::OperateType::Change == FileItemModel::OperateType(operateType)){
            for (auto& info : retFileInfos) {
                auto item = m_uri_item_hash.value(info->uri());
                if (!item)
                    continue;
                info_manager->lock();
                info_manager->updateFileInfo(info);
                item->m_info = info;
                info_manager->unlock();
                ThumbnailManager::getInstance()->createThumbnail(info.get()->uri(), m_thumbnail_watcher, true);
                EmblemProviderManager::getInstance()->queryAsync(info->uri());
            }
            m_model->updated();
        }
    },Qt::UniqueConnection);


    connect(m_addChildTimer, &QTimer::timeout, this, [=]{
        m_waiting_add_queue.removeDuplicates(); /* 去重 */
        if(m_waiting_add_queue.count() < 50){
            for (auto uri : m_waiting_add_queue) {
                m_waiting_add_queue.removeOne(uri);
                auto info = FileInfo::fromUri(uri);
                auto infoJob = new FileInfoJob(info);
                infoJob->setAutoDelete();
                infoJob->connect(infoJob, &FileInfoJob::infoUpdated, this, [=]() {
                    auto item = getChildFromUri(uri);
                    // add exsited checkment. link to: #66999
                    if (!item) {
                        item = new FileItem(info, this, m_model);
            #ifdef LINGMO_UDF_BURN
                        if(m_isRTypeDisc){
                            item->setProperty("isFileForBurning", true);
                        }
            #endif
                        m_model->beginInsertRows(QModelIndex(), m_children->count(), m_children->count());
                        m_children->append(item);
                        m_uri_item_hash.insert(item->uri(), item);
                        m_model->endInsertRows();

                        qDebug() <<"successfully added child:" <<uri;

                        /* Fixbug#82649:在手机内部存储里新建文件/文件夹时，名称不是可编辑状态,都是默认文件名/文件夹名 */
                        Q_EMIT m_model->signal_itemAdded(uri);//end

                        //QTimer::singleShot(1000, this, [=](){
                            ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher);
                       // });
                    } else {
                        qInfo()<<"file"<<uri<<"has arealy in file item model";
                    }
                });

                infoJob->connect(this, &FileItem::cancelFindChildren, infoJob, &FileInfoJob::cancel);
                QTimer::singleShot(100, this, [=](){/* 加延时避免查询结果有误；linkto bug#197886 */
                    infoJob->queryAsync();
                });
            }

        }else{
            QStringList list;
            if(m_waiting_add_queue.size() >= 2000){/* 每次批量处理最多数量 */
                for(int i = 0; i < 2000; i++){
                    QString uri = m_waiting_add_queue.takeFirst();
                    list.append(uri);
                }
            }else{
                list.swap(m_waiting_add_queue);
            }

            Q_EMIT m_model->setUrisForBatchQueryInfos(list, FileItemModel::OperateType::Add, this);
            if (m_waiting_add_queue.size()>0 && !m_addChildTimer->isActive()) {
                m_addChildTimer->start();
            }
        }
    },Qt::UniqueConnection);


    connect(m_changeChildTimer, &QTimer::timeout, this, [=]{
        m_waiting_update_queue.removeDuplicates(); /* 去重 */
        QStringList list;
        if(m_waiting_update_queue.size() >= 2000){/* 每次批量处理最多数量 */
            for(int i = 0; i < 2000; i++){
                QString uri = m_waiting_update_queue.takeFirst();
                list.append(uri);
            }
        }else{
            list.swap(m_waiting_update_queue);
        }
        Q_EMIT m_model->setUrisForBatchQueryInfos(list, FileItemModel::OperateType::Change, this);
        if (m_waiting_update_queue.size()>0 && !m_changeChildTimer->isActive()) {
            m_changeChildTimer->start();
        }
    },Qt::UniqueConnection);
}

void FileItem::clearChildren()
{
    auto parent = firstColumnIndex();
    m_model->removeRows(0, m_model->rowCount(parent), parent);
    for (auto child : *m_children) {
        delete child;
    }
    m_children->clear();
    m_uri_item_hash.clear();
    m_expanded = false;
    m_watcher.reset();
    m_watcher = nullptr;
    m_rTypeDiscWatcher.reset();
    m_rTypeDiscWatcher = nullptr;
}

void FileItem::childrenUpdateOfEnumerate(const QStringList &uris, bool isEnding)
{
    uris.toSet().toList();/* 去重 */
    if(uris.size()<=0)
        return;

    if (isEnding) {
        m_isEndOfEnumerate = isEnding;
    }
    m_ending_uris.clear();
    m_ending_uris = uris;

    QStringList originalList = uris; /* 原始列表 */
    /* 遍历时第一次先加载100个显示在桌面上，其余按大批量查询 */
    QStringList firstTimeUris;
    int count = 100;
    if(originalList.size() > count){
        for(int idx = 0; idx < count; idx++){
            QString uri = originalList.takeFirst();
            firstTimeUris.append(uri);
        }
        Q_EMIT m_model->setUrisForBatchQueryInfos(firstTimeUris, FileItemModel::OperateType::Enumerate, this);
    }//end

    /* 大批量查询 */
    int chunkSize = 1000; /* 每个列表的大小 */
    QList<QStringList> splitLists;
    if(originalList.size() > chunkSize){
        int numChunks = originalList.count() / chunkSize;
        if (originalList.count() % chunkSize)
            numChunks++;

        for (int i = 0; i < numChunks; i++) {
            QStringList chunkList;
            for (int j = 0; j < chunkSize && ((i * chunkSize) + j) < originalList.count(); j++) {
                chunkList.append(originalList.at((i * chunkSize) + j));
            }
            splitLists.append(chunkList);
        }
    }else{
        splitLists.append(originalList);
    }

    for(auto &queryUris : splitLists){
        //qDebug()<<"childrenUpdated:"<<queryUris.size()<<m_ending_uris.size()<<this->uri()<<m_isEndOfEnumerate<<isEnding;
        Q_EMIT m_model->setUrisForBatchQueryInfos(queryUris, FileItemModel::OperateType::Enumerate, this);
    }//end
}

/* Func: if it isn't a vaild volume device,it should not be displayed.
 */
bool FileItem::shouldShow()
{
    QString uri,unixDevice,displayName;

    uri = m_info->uri();
    if(uri.isEmpty())
        return false;
    if("computer:///root.link" == uri)
        return true;

    //non computer path, no need check
    //to fix sftp IO stuck issue
    if (! uri.startsWith("computer:///") || ! uri.endsWith(".drive"))
        return true;

    displayName = FileUtils::getFileDisplayName(uri);
    if(displayName.isEmpty())
        return false;

    if (displayName.contains(":"))
        return true;

    //if needed, comment to not use this IO stuck API
    unixDevice = FileUtils::getUnixDevice(uri);
    if(!unixDevice.isEmpty()){
        return false;
    }
    return true;
}

BatchProcessItems::BatchProcessItems()
{

}

BatchProcessItems::~BatchProcessItems()
{

}

void BatchProcessItems::setBatchRemoveParam(const QStringList& uris_to_be_removed, const QHash<QString, FileItem*>& uri_item_hash, QVector<FileItem*> *children)
{
    m_uris_to_be_removed = uris_to_be_removed;
    m_uri_item_hash = uri_item_hash;
    m_children = new QVector<FileItem *>(*children);
}

void BatchProcessItems::slot_removeItems()
{
    // do reset model
    int time0 = QTime::currentTime().msecsSinceStartOfDay();
    QStringList favoriteUris;
    QVector<QString> needHandleLabelUris;
    QVector<FileItem *> itemsToBeDeleted;
    qDebug()<<"execute deletion, deleted count:"<<m_uris_to_be_removed.count()<<",children count,uri item hash count:"<<m_children->size()<<m_uri_item_hash.size();
    for (auto& uri : m_uris_to_be_removed) {
        if(m_uri_item_hash.contains(uri)){
            auto child = m_uri_item_hash.value(uri);
            auto info = child->info();
            if (info && info->isDir())
            {
                favoriteUris.append(uri2FavoriteUri(uri));
            }
            needHandleLabelUris.push_back(uri);
            int i = m_uri_item_hash.remove(uri);
            m_uris_to_be_removed.removeOne(uri);
            m_children->removeOne(child);
            itemsToBeDeleted.append(child);
        }
    }
    BookMarkManager::getInstance()->removeBookMark(favoriteUris);
    Q_EMIT removeItemsFinished(m_children, m_uri_item_hash, needHandleLabelUris);
    for (auto child : itemsToBeDeleted) {
        delete child;
    }
    int time1 = QTime::currentTime().msecsSinceStartOfDay();
    qDebug()<<"execute deletion finished, cost"<<time1 - time0;
}


ExtraInfoRecorder::ExtraInfoRecorder(const QString &uri)
    :m_uri(uri)
{

}

ExtraInfoRecorder::~ExtraInfoRecorder()
{
    /* 改成程序退出时再释放图片，linkto bug#215437 */
    //ThumbnailManager::getInstance()->releaseThumbnail(m_uri);
    EmblemProviderManager::getInstance()->cancelQuery(m_uri);
}

const QString ExtraInfoRecorder::uri()
{
    return m_uri;
}
