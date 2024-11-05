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

#include "side-bar-favorite-item.h"
#include "side-bar-model.h"
#include "file-utils.h"

#include "file-info.h"
#include "file-info-job.h"

#include "bookmark-manager.h"

#include <QStandardPaths>
#include <QUrl>
#include <QDir>

using namespace Peony;

bool kydroidInstall = false;
QString kydroidPath = "kydroid:///";
static const char* localFileSystemPath = "file://";

SideBarFavoriteItem::SideBarFavoriteItem(QString uri,SideBarFavoriteItem *parentItem,
                                         SideBarModel *model, QObject *parent) :
    SideBarAbstractItem (model, parent),m_parent(parentItem)

{
    m_uri = uri;
    m_is_root_child = m_parent == nullptr;
    if (m_is_root_child) {
        initChildren();
        return;
    }

    //FIXME: replace BLOCKING api in ui thread.
    GFile* file = g_file_new_for_uri(m_uri.toUtf8().constData());
    if (nullptr != file) {
        GFileInfo* fileInfo = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
        if (nullptr != fileInfo) {
            const char* displayName = g_file_info_get_attribute_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
            if (nullptr != displayName) {
                m_displayName = displayName;
            }
            g_object_unref(fileInfo);
        }
        g_object_unref(file);
    }

    qDebug() << "SideBarFavoriteItem m_displayName:"<<m_displayName <<m_uri;
    if (m_displayName.isEmpty() || "" == m_displayName) {
        m_displayName = FileUtils::getFileDisplayName(m_uri);
    }

    m_iconName = FileUtils::getFileIconName(m_uri);

    m_info = FileInfo::fromUri(m_uri);
    auto infoJob = new FileInfoJob(m_info);
    infoJob->setAutoDelete();
    connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=](){
        Q_EMIT this->queryInfoFinished();
    });
    infoJob->queryAsync();

    if (m_uri == "trash:///") {
        m_displayName = tr("Trash");
        m_iconName = "user-trash-symbolic";
    } else if (m_uri == "recent:///") {
        m_displayName = tr("Recent");
        m_iconName = "document-open-recent-symbolic";
    }
}

void SideBarFavoriteItem::initChildren()
{
    m_uri = "favorite:///";
    m_displayName = tr("Quick access");

    QString desktopUri = localFileSystemPath + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
//    QString videoUri = localFileSystemPath + QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
//    QString pictureUri = localFileSystemPath + QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
//    QString downloadUri = localFileSystemPath + QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
//    QString musicUri = localFileSystemPath + QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
//    QString docUri = localFileSystemPath + QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    auto recentItem = new SideBarFavoriteItem("recent:///", this, m_model);
    auto desktopItem = new SideBarFavoriteItem(desktopUri, this, m_model);
    auto trashItem = new SideBarFavoriteItem("trash:///", this, m_model);
//    auto videoItem = new SideBarFavoriteItem(videoUri, this, m_model);
//    auto pictureItem = new SideBarFavoriteItem(pictureUri, this, m_model);
//    auto downloadItem = new SideBarFavoriteItem(downloadUri, this, m_model);
//    auto musicItem = new SideBarFavoriteItem(musicUri, this, m_model);
//    auto docItem = new SideBarFavoriteItem(docUri, this, m_model);

    m_children->append(recentItem);
    m_children->append(desktopItem);
    m_children->append(trashItem);
    //m_children->append(docItem);
    //m_children->append(musicItem);
    //m_children->append(downloadItem);
    //m_children->append(pictureItem);
    //m_children->append(videoItem);


    if (FileUtils::isFileExsit("file:///data/usershare")) {
        m_children->append(new SideBarFavoriteItem("favorite:///data/usershare?schema=file", this, m_model));
    }

    // check kydroid is install
    if (QFile::exists("/usr/lib/libkmre.so")){
        // new version has no kmre plugin, use /usr/lib/libkmre.so and path /var/lib/kmre/data/kmre-1000-lingmo/KmreData
        QString MobileDataPath = FileUtils::getMobieDataPath();
        if (MobileDataPath.length() > 0)
        {
            kydroidInstall = true;
            auto kmreItem = new SideBarFavoriteItem(MobileDataPath, this, m_model);
            kmreItem->setProperty("isKmre", true);
            m_children->append(kmreItem);
        }
    }else if (FileUtils::isFileExsit("file:///var/lib/kydroid") || FileUtils::isFileExsit("file:///var/lib/kmre")) {
        GVfs* vfs = g_vfs_get_default();
        if (vfs) {
            const gchar* const* schemas = g_vfs_get_supported_uri_schemes (vfs);
            if (schemas) {
                int i = 0;
                for (; schemas[i] != NULL; ++i) {
                    if (0 == strcmp(schemas[i], "kydroid")) {
                        kydroidInstall = true;
                        kydroidPath = "kydroid:///";
                        break;
                    }
                    else if(0 == strcmp(schemas[i], "kmre"))
                    {
                        kydroidInstall = true;
                        kydroidPath = "kmre:///";
                        break;
                    }
                }
            }
        }
        if (kydroidInstall)
            m_children->append(new SideBarFavoriteItem(kydroidPath, this, m_model));
    }

    m_model->insertRows(0, m_children->count(), firstColumnIndex());
    //TODO: support custom bookmarks.
    auto bookmark = BookMarkManager::getInstance();
    if (bookmark->isLoaded()) {
        syncBookMark();
    } else {
        connect(bookmark, &BookMarkManager::urisLoaded, this, [=]() {
            syncBookMark();
            disconnect(bookmark, &BookMarkManager::urisLoaded, this, nullptr);
        });
    }
}

SideBarAbstractItem::Type SideBarFavoriteItem::type() {
    return SideBarAbstractItem::FavoriteItem;
}

QString SideBarFavoriteItem::uri()
{
    return m_uri;
}

QString SideBarFavoriteItem::displayName()
{
    //fix bug#125040
    if (property("isKmre").toBool())
        return tr("KmreData");

    if (!m_info)
        return m_displayName;
    if (m_info->displayName().isEmpty())
        return m_displayName;
    return m_info.get()->displayName();
}

QString SideBarFavoriteItem::iconName()
{
    //fix bug#125040
    if (property("isKmre").toBool())
        return "folder-phone";

    if (!m_info)
        return m_iconName;
    if (m_info->iconName().isEmpty()) {
        return m_iconName;
    }
    /* 设计要求图标统一，文件夹图标名称为folder；而从底层读取的文件夹名称为inode-directory，因此替换一下 */
    auto iconName = m_info.get()->iconName();
    if(iconName == "inode-directory")
        iconName = "folder";
    return iconName;
}

bool SideBarFavoriteItem::hasChildren()
{
    return m_is_root_child;
}

QModelIndex SideBarFavoriteItem::firstColumnIndex()
{
    //TODO: bind with model
    return m_model->firstColumnIndex(this);
}

QModelIndex SideBarFavoriteItem::lastColumnIndex()
{
    //TODO: bind with model
    return m_model->lastColumnIndex(this);
}

void SideBarFavoriteItem::syncBookMark()
{
    qDebug()<<"sync book mark=================="<<this->displayName();
    auto bookmark = BookMarkManager::getInstance();
    auto uris = bookmark->getCurrentUris();
    for (auto uri : uris) {
        auto item = new SideBarFavoriteItem(uri, this, m_model);
        *m_children<<item;
        m_model->insertRows(m_children->count() - 1, 1, this->firstColumnIndex());
    }
    connect(bookmark, &BookMarkManager::bookMarkAdded, this, [=](const QString &uri, bool successed) {
        if (successed) {
            auto item = new SideBarFavoriteItem(FileUtils::urlDecode(uri), this, m_model);
            *m_children<<item;
            m_model->insertRows(m_children->count() - 1, 1, this->firstColumnIndex());
        }
    });
    connect(bookmark, &BookMarkManager::bookMarkRemoved, this, [=] (const QString &uri, bool successed) {
        QString delUri = getTargetUri(uri);
        if (successed) {
            for (auto item : *m_children) {
                QString itemUri = getTargetUri(item->uri());
                if (delUri == itemUri) {
                    m_model->removeRow(m_children->indexOf(item), this->firstColumnIndex());
                    m_children->removeOne(item);
                }
            }
        }
    });
}

QString SideBarFavoriteItem::getTargetUri(const QString &uri)
{
    QString sguri = uri;
    if (uri.startsWith("favorite://")) {
        g_autoptr(GFile) turi = g_file_new_for_uri(FileUtils::urlEncode(uri).toUtf8().constData());
        if (turi) {
            g_autoptr(GFileInfo) turiInfo = g_file_query_info(turi, "standard::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
            if (turiInfo) {
                g_autofree char* tturi = g_file_info_get_attribute_as_string(turiInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
                if (tturi) {
                    sguri = tturi;
                }
            }
        }
    }

    return FileUtils::urlDecode(sguri);
}
