/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhuoan Li <lizhuoan@kylinos.cn>
 *
 */

#ifdef LINGMO_UDF_BURN

#include "file-operation-helper.h"
#include <libkyudfburn/datacdrom.h>
#include <libkyudfburn/udfreadwrite.h>
#include "gio/gio.h"
#include <QDir>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

using namespace Peony;

using namespace UdfBurn;

FileOperationHelper::FileOperationHelper(const QString destDir, QObject *parent) : QObject(parent)
{
    matchingUnixDevice(destDir);
}

FileOperationHelper::~FileOperationHelper()
{

}

QString FileOperationHelper::getDiscType()
{
    return m_disc_media_type;
}

bool FileOperationHelper::isUnixCDDevice()
{
    if (!m_unix_device.isEmpty() && m_unix_device.startsWith("/dev/sr")) {
        return true;
    } else {
        return false;
    }
}

void FileOperationHelper::judgeSpecialDiscOperation()
{
    //获取光盘文件格式和光盘类型
    QString dbusPath = "/org/freedesktop/UDisks2/block_devices/";
    dbusPath.append(m_unix_device.split("/").back());
    QDBusInterface iface("org.freedesktop.UDisks2",dbusPath,"org.freedesktop.DBus.Properties",QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UDisks2.Block", "IdType");
    if (reply.isValid()) {
        m_disc_system_type = reply.value().toString();
        qDebug() << "disc system type is:" << m_disc_system_type;
    }
    if (!m_disc_system_type.isEmpty()) {
        DataCDROM *cdrom = new DataCDROM(m_unix_device);
        m_disc_media_type = cdrom->getDiscType();
        qDebug() << "disc media type is:" << m_disc_media_type;
        delete cdrom;
    }
}

QString FileOperationHelper::matchingUnixDevice(QString uri)
{
    QString destPath = getDestName(uri);
    destPath = uri.remove(destPath);
    GFile *file = g_file_new_for_uri(destPath.toUtf8().constData());
    if (!file) {
        qDebug()<< "File creation Failure";
        g_object_unref(file);
        return m_unix_device;
    }
    GError *err = nullptr;
    GMount *mount = g_file_find_enclosing_mount(file, nullptr, &err);
    if (err) {
        qDebug()<< err->code <<err->message;
        g_error_free(err);
        g_object_unref(file);
        return m_unix_device;
    }
    g_object_unref(file);
    if (mount) {
        GVolume *volume = g_mount_get_volume(mount);
        g_autofree gchar *device = g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        m_unix_device = device;
    }
    return m_unix_device;
}

QString FileOperationHelper::dealDVDReduce()
{
    if (0 == m_disc_media_type.compare("DVD-R")
              || 0 == m_disc_media_type.compare("CD-R")
              || 0 == m_disc_media_type.compare("DVD+R")
              || 0 == m_disc_media_type.compare("DVD+R DL")) {
        QString destDir = QDir::homePath() + "/.cache";
        QDir dir = QDir(destDir);
        if (!dir.exists("LingmoTransitBurner")) {
            dir.mkdir("LingmoTransitBurner");
        }
        destDir.append("/LingmoTransitBurner").prepend("file://");
        return destDir;
    } else {
        return "";
    }
}

bool FileOperationHelper::discWriteOperation(const QStringList &sourUrisList, const QString &destUri)
{
    if (!m_disc_media_type.isNull() && !sourUrisList.isEmpty()) {
        if (m_disc_media_type.contains("DVD-RW") || m_disc_media_type.contains("CD-RW")) {
            bool right;
            char* errinfo = (char*)malloc(1024);
            memset(errinfo, 0, 1024);
            QStringList list;
            GFile* destGfile = g_file_new_for_uri(destUri.toUtf8().constData());
            QString destPath = g_file_get_path(destGfile);
            if (nullptr != destGfile) {
                g_object_unref(destGfile);
            }
            for(QString sourUri : sourUrisList) {
                GFile* sourGfile = g_file_new_for_uri(sourUri.toUtf8().constData());
                sourUri = g_file_get_path(sourGfile);
                sourUri = destPath + "/" +sourUri.split("/").back();
                list << sourUri;
                if (nullptr != sourGfile) {
                    g_object_unref(sourGfile);
                }
            }
            QString destName = getDestName(destPath);
            //创建UdfReadWrite类
            std::shared_ptr<UdfReadWrite> udfwrite = std::make_shared<UdfReadWrite>(m_unix_device,nullptr);
            //开启udfclient的初始化
            right = udfwrite->startUdfClient(&errinfo);
            if (!right) {
                if (udfwrite->isDiscRunning()) {
                    m_disc_error_msg = errinfo;
                } else {
                    m_disc_error_msg = tr("Burn failed");
                }
                qDebug() << "udf clint error message: " << errinfo;
                for (QString filePath : list) {
                    QFile file(filePath);
                    if (file.exists()) {
                        file.remove();
                    }
                }               
                udfwrite->closeUdfClient();
                free(errinfo);
                return false;
            }
            //写单个数据
            //right = udfwrite->writeSingleData(&errinfo, m_source_uris);
            qDebug() << "udf write list:" << list << "dest name" << destName;
            right = udfwrite->writeMultiData(&errinfo, list, destName);
            if (!right) {
                m_disc_error_msg = tr("Burn failed");
                qDebug() << "udf write error message: " << errinfo;
                for (QString filePath : list) {
                    QFile file(filePath);
                    if (file.exists()) {
                        file.remove();
                    }
                }
                udfwrite->closeUdfClient();
                free(errinfo);
                return false;
            }
            //同步数据及关闭udfclient
            udfwrite->closeUdfClient();
            free(errinfo);

        }
    }
    return true;
}

void FileOperationHelper::discRenameOperation(const QString &oldName,const QString &newName)
{
    if (!m_disc_media_type.isNull()) {
        if (m_disc_media_type.contains("DVD-RW") || m_disc_media_type.contains("CD-RW")) {
            bool right;
            char* errinfo = (char*)malloc(1024);
            memset(errinfo, 0, 1024);
            //创建UdfReadWrite类
            std::shared_ptr<UdfReadWrite> udfwrite = std::make_shared<UdfReadWrite>(m_unix_device,nullptr);
            //开启udfclient的初始化
            right = udfwrite->startUdfClient(&errinfo);
            if(!right) {
                qDebug() << "udf clint error message: " << errinfo;
                udfwrite->closeUdfClient();
                free(errinfo);
                return;
            }
            //重命名数据
            right = udfwrite->changeFileName(oldName, newName, &errinfo);

            if(!right) {
               qDebug() << "udf rename error message: " << errinfo;
            }
            //同步数据及关闭udfclient
            udfwrite->closeUdfClient();
            free(errinfo);
        }
    }
}

void FileOperationHelper::discDeleteOperation(const QStringList &srcUris)
{
    if (!m_disc_media_type.isNull()) {
        if (m_disc_media_type.contains("DVD-RW") || m_disc_media_type.contains("CD-RW")) {
            bool right;
            char* errinfo = (char*)malloc(1024);
            memset(errinfo, 0, 1024);
            QStringList list;
            for (QString srcUri : srcUris) {
                GFile* sourGfile = g_file_new_for_uri(srcUri.toUtf8().constData());
                srcUri = g_file_get_path(sourGfile);
                srcUri = getDestName(srcUri);
                list << srcUri;
                if (nullptr != sourGfile) {
                    g_object_unref(sourGfile);
                }
            }
            //创建UdfReadWrite类
            std::shared_ptr<UdfReadWrite> udfwrite = std::make_shared<UdfReadWrite>(m_unix_device,nullptr);
            //开启udfclient的初始化
            right = udfwrite->startUdfClient(&errinfo);
            if(!right) {
                qDebug() << "udf clint error message: " << errinfo;
                free(errinfo);
                udfwrite->closeUdfClient();
                return;
            }
            //删除数据
            right = udfwrite->removeFile(list, &errinfo);
            if(!right) {
                qDebug() << "udf clint error message: " << errinfo;
            }
            //同步数据及关闭udfclient
            udfwrite->closeUdfClient();
            free(errinfo);
        }
    }
}

QString FileOperationHelper::getDestName(const QString &destUri)
{
    QString destName = destUri;
    QStringList list;
    if (!destName.startsWith("/")) {
        list = destName.remove(0,14).split("/");
    } else {
        list = destName.remove(0,7).split("/");
    }
    if (2 == list.count()) {
        destName = QString("");
    } else {
        list.removeAt(1);
        list.pop_front();
        destName = list.join("/");
    }
    return destName;
}

QString FileOperationHelper::getDiscError()
{
    return m_disc_error_msg;
}

#endif
