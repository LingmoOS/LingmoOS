/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "share-emblem-provider.h"
#include <QDebug>
#include <gio/gio.h>

using namespace Peony;

static ShareEmblemProvider *global_instance = nullptr;

ShareEmblemProvider *ShareEmblemProvider::getInstance()
{
    if(!global_instance)
        global_instance = new ShareEmblemProvider;
    return global_instance;
}

const QString ShareEmblemProvider::emblemKey()
{
    return SHARE_EMBLEMS;
}

QStringList ShareEmblemProvider::getFileEmblemIcons(const QString &uri)
{
    QStringList lists;
    if (uri.startsWith("file:///")) {
        auto fileInfo = FileXattrInfo::fromUri(uri);
        if(fileInfo && !fileInfo->getXattrInfoString(SHARE_EMBLEMS).isEmpty()){
            lists.append(fileInfo->getXattrInfoString(SHARE_EMBLEMS));
        }
    }
    return lists;
}

ShareEmblemProvider::ShareEmblemProvider(QObject *parent)
{
    Q_UNUSED(parent);
}

std::shared_ptr<FileXattrInfo> FileXattrInfo::fromUri(const QString &uri)
{
    return std::make_shared<FileXattrInfo>(uri);
}

FileXattrInfo::FileXattrInfo(const QString &uri)
{
    m_uri = uri;
    g_autoptr(GFile) file = g_file_new_for_uri(m_uri.toUtf8().constData());
    if (file) {
        g_autoptr(GFileInfo) fileInfo = g_file_query_info(file, "xattr::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
        if (fileInfo) {
            char **xattrinfo_attributes = g_file_info_list_attributes(fileInfo, "xattr");
            if(xattrinfo_attributes){
                for (int i = 0; xattrinfo_attributes[i] != nullptr; i++) {
                    auto type = g_file_info_get_attribute_type(fileInfo, xattrinfo_attributes[i]);
                    switch (type) {
                    case G_FILE_ATTRIBUTE_TYPE_STRING: {
                        char *string = g_file_info_get_attribute_as_string(fileInfo, xattrinfo_attributes[i]);
                        if (string) {
                            auto var = QVariant(string);
                            this->setXattrInfoString(xattrinfo_attributes[i], var.toString(), false);
                            //qDebug()<<"xattrinfo_attributes======"<<m_uri<<xattrinfo_attributes[i]<<var.toString();
                            g_free(string);
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                g_strfreev(xattrinfo_attributes);
            }
        }
    }
}

void FileXattrInfo::setXattrInfoString(const QString &key, const QString &value, bool syncToFile)
{
    QString realKey = key;
    if (!key.startsWith("xattr::"))
        realKey = "xattr::" + key;

    if(syncToFile) {
        g_autoptr (GFile) file = g_file_new_for_uri(this->m_uri.toUtf8().constData());
        GError *err = nullptr;
        g_file_set_attribute(file, realKey.toLatin1().data(), G_FILE_ATTRIBUTE_TYPE_STRING,
                             (gpointer)value.toUtf8().data(),
                             G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &err);

        if (err) {
            qWarning() << err->message;
            g_error_free(err);
        } else {
            m_hash.remove(realKey);
            m_hash.insert(realKey, value);
        }
    } else {
           m_hash.remove(realKey);
           m_hash.insert(realKey, value);
    }
    qDebug() << "setXattrInfoString" << m_hash;
}

const QString FileXattrInfo::getXattrInfoString(const QString &key)
{
    QString realKey = key;
    if (!key.startsWith("xattr::"))
        realKey = "xattr::" + key;
    if (m_hash.value(realKey).isValid())
        return m_hash.value(realKey).toString();
    //FIXME: should i use gio query meta here?
    return QString();
}

void FileXattrInfo::removeXattrInfo(const QString &key)
{
    //FIXME: Failed to set invalid type
    this->setXattrInfoString(key, " ", true);
}
