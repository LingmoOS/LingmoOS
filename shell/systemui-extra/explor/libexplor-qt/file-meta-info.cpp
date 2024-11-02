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

#include "file-meta-info.h"
#include "file-info-manager.h"

#include <QDebug>

using namespace Peony;

std::shared_ptr<FileMetaInfo> FileMetaInfo::fromGFileInfo(const QString &uri, GFileInfo *g_info)
{
    return std::make_shared<FileMetaInfo>(uri, g_info);
}

std::shared_ptr<FileMetaInfo> FileMetaInfo::fromUri(const QString &uri)
{
    auto mgr = FileInfoManager::getInstance();
    auto info = mgr->findFileInfoByUri(uri);
    if (info)
        return mgr->findFileInfoByUri(uri)->m_meta_info;
    return nullptr;
}

std::shared_ptr<FileMetaInfo> FileMetaInfo::dupFromUri(const QString &uri)
{
    auto mgr = FileInfoManager::getInstance();
    auto info = mgr->findFileInfoByUri(uri);
    if (info) {
        //fixme: metainfo不安全, 在主线程中可能被修改
        auto metaInfo = mgr->findFileInfoByUri(uri)->m_meta_info;
        if (metaInfo) {
            QMutexLocker l(&(metaInfo->m_mutex));
            auto dupInfo = std::make_shared<FileMetaInfo>(metaInfo.get());
            return dupInfo;
        }
    }
    return nullptr;
}

FileMetaInfo::FileMetaInfo(const QString &uri, GFileInfo *g_info)
{
    m_uri = uri;
    if (g_info) {
        char **metainfo_attributes = g_file_info_list_attributes(g_info, "metadata");
        if (metainfo_attributes) {
            for (int i = 0; metainfo_attributes[i] != nullptr; i++) {
                auto type = g_file_info_get_attribute_type(g_info, metainfo_attributes[i]);
                switch (type) {
                case G_FILE_ATTRIBUTE_TYPE_STRING: {
                    char *string = g_file_info_get_attribute_as_string(g_info, metainfo_attributes[i]);
                    if (string) {
                        auto var = QVariant(string);
                        this->setMetaInfoVariant(metainfo_attributes[i], var, false);
                        //qDebug()<<"======"<<m_uri<<metainfo_attributes[i]<<var.toString();
                        g_free(string);
                    }
                    break;
                }
                case G_FILE_ATTRIBUTE_TYPE_STRINGV: {
                    gchar **stringv = g_file_info_get_attribute_stringv(g_info, metainfo_attributes[i]);
                    if (stringv) {
                        QStringList list;
                        gchar **tmp = stringv;
                        while (*tmp) {
                            list<<*tmp;
                            tmp++;
                        }
                        auto var = QVariant(list);
                        this->setMetaInfoVariant(metainfo_attributes[i], var, false);
                    }
                }
                default:
                    break;
                }
            }
            g_strfreev(metainfo_attributes);
        }
    }
}

FileMetaInfo::FileMetaInfo(const FileMetaInfo &other)
{
    m_uri = other.m_uri;
    m_meta_hash = other.m_meta_hash;
}

FileMetaInfo::FileMetaInfo(FileMetaInfo *other)
{
    m_uri = other->m_uri;
    m_meta_hash = other->m_meta_hash;
}

FileMetaInfo::~FileMetaInfo()
{
}

void FileMetaInfo::setMetaInfoInt(const QString &key, int value)
{
    setMetaInfoVariant(key, QString::number(value));
}

void FileMetaInfo::setMetaInfoString(const QString &key, const QString &value)
{
    setMetaInfoVariant(key, value);;
}

void FileMetaInfo::setMetaInfoStringList(const QString &key, const QStringList &value)
{
    QString string = value.join('\n');
    setMetaInfoVariant(key, string);
}

void FileMetaInfo::setMetaInfoVariant(const QString &key, const QVariant &value, bool syncToFile)
{
//    if (!m_mutex.tryLock(300)) {
//        return;
//    }

    QString realKey = key;
    if (!key.startsWith("metadata::"))
        realKey = "metadata::" + key;

    if (syncToFile) {
        GFile *file = g_file_new_for_uri(m_uri.toUtf8().constData());
        GError *err = nullptr;
        g_file_set_attribute(file, realKey.toLatin1().data(), G_FILE_ATTRIBUTE_TYPE_STRING,
                             (gpointer)value.toString().toUtf8().constData(),
                             G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &err);

        if (err) {
            qDebug()<<err->message;
            g_error_free(err);
        } else {
            m_mutex.lock();
            m_meta_hash.remove(realKey);
            m_meta_hash.insert(realKey, value);
            m_mutex.unlock();
        }
        g_object_unref(file);
    } else {
        m_mutex.lock();
        m_meta_hash.remove(realKey);
        m_meta_hash.insert(realKey, value);
        m_mutex.unlock();
    }
//    m_mutex.unlock();
}

const QVariant FileMetaInfo::getMetaInfoVariant(const QString &key)
{
    QString realKey = key;
    if (!key.startsWith("metadata::"))
        realKey = "metadata::" + key;

    QMutexLocker l(&m_mutex);
    if (m_meta_hash.contains(realKey) && m_meta_hash.value(realKey).isValid()){
        return m_meta_hash.value(realKey);
    }
    //FIXME: should i use gio query meta here?
    return QVariant();
}

void FileMetaInfo::setMetaInfoStringListV1(const QString &key, const QStringList &value)
{
    QString realKey = key;
    if (!key.startsWith("metadata::"))
        realKey = "metadata::" + key;

    QString tmp = value.join("\n");
    gchar **stringv = g_strsplit(tmp.toUtf8().constData(), "\n", -1);

    g_autoptr (GFile) file = g_file_new_for_uri(this->m_uri.toUtf8().constData());
    GError *err = nullptr;
    g_file_set_attribute(file, realKey.toUtf8().constData(), G_FILE_ATTRIBUTE_TYPE_STRINGV, gpointer(stringv), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &err);

    g_strfreev(stringv);

    if (err) {
        qWarning() << err->message;
        g_error_free(err);
    } else {
        m_mutex.lock();
        m_meta_hash.remove(realKey);
        m_meta_hash.insert(realKey, value);
        m_mutex.unlock();
    }
}

const QStringList FileMetaInfo::getMetaInfoStringListV1(const QString &key)
{
    return getMetaInfoVariant(key).toStringList();
}

const QString FileMetaInfo::getMetaInfoString(const QString &key)
{
    return getMetaInfoVariant(key).toString();
}

const QStringList FileMetaInfo::getMetaInfoStringList(const QString &key)
{
    return getMetaInfoVariant(key).toString().split('\n');
}

int FileMetaInfo::getMetaInfoInt(const QString &key)
{
    return getMetaInfoVariant(key).toString().toInt();
}

void FileMetaInfo::removeMetaInfo(const QString &key)
{
//    if (!m_mutex.tryLock(300)) {
//        return;
//    }

    QString realKey = key;
    if (!key.startsWith("metadata::"))
        realKey = "metadata::" + key;
    m_mutex.lock();
    m_meta_hash.remove(realKey);
    GFile *file = g_file_new_for_uri(m_uri.toUtf8().constData());
    m_mutex.unlock();
    g_file_set_attribute(file,
                         realKey.toUtf8().constData(),
                         G_FILE_ATTRIBUTE_TYPE_INVALID,
                         nullptr,
                         G_FILE_QUERY_INFO_NONE,
                         nullptr,
                         nullptr);
    g_object_unref(file);
//    m_mutex.unlock();
}
