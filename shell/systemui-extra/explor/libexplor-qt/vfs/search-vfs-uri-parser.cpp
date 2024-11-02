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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "search-vfs-uri-parser.h"
#include "file-utils.h"

#include <QStringList>
#include <QDebug>
using namespace Peony;

SearchVFSUriParser::SearchVFSUriParser()
{

}

const QString SearchVFSUriParser:: parseSearchKey(const QString &uri, const QString &key, const bool &search_file_name,
        const bool &search_content, const QString &extend_key, const bool &recursive)
{
    //When searching the computer directory, switch to search under the root directory. fix bug97220
    QString tmp = uri;
    if(uri.startsWith("computer:///")){
        tmp = "file:///";
    }
    QString search_str = "search:///search_uris="+tmp;
    if (search_file_name)
        search_str += "&name_regexp="+key;
    if (search_content)
        search_str += "&content_regexp="+key;
    //multiple name key search
    if (extend_key != "")
        search_str += "&extend_regexp="+extend_key;
    else if (! search_file_name && ! search_content && extend_key=="")
    {
        qWarning()<<"Search content or file name at least one be true!";
        //use default search file name
        search_str += "&name_regexp="+key;
    }

    if (recursive)
        return QString(search_str+"&recursive=1");

    return QString(search_str+"&recursive=0");
}

const QString SearchVFSUriParser::getSearchUriNameRegexp(const QString &searchUri)
{
    auto string = searchUri;
    string.remove("search:///");
    auto list = string.split("&");
    qDebug() << "list length:" <<list.length();
    QString ret = "";
    for (auto arg : list) {
        if (arg.startsWith("name_regexp=")) {
            qDebug()<<"name_regexp: "<<arg;
            auto tmp = arg.remove("name_regexp=");
            if (ret == "")
                ret = tmp;
            else
                ret += "," + tmp;
        }

        if (arg.startsWith("extend_regexp"))
        {
            qDebug()<<"extend_regexp: "<<arg;
            auto tmp = arg.remove("extend_regexp=");
            if (ret == "")
                ret = tmp;
            else
                ret += "," + tmp;
        }
    }
    return ret;
}

const QString SearchVFSUriParser::getSearchUriTargetDirectory(const QString &searchUri)
{
    auto string = searchUri;
    string.remove("search:///");
    auto list = string.split("&");
    for (auto arg : list) {
        if (arg.startsWith("search_uris=")) {
            qDebug()<<"arg:"<<arg;
            auto tmp = arg.remove("search_uris=");
            auto uris = tmp.split(",");
            if (uris.count() == 1) {
                //FIXME: replace BLOCKING api in ui thread.
                //When searching the computer directory, switch to search under the root directory. fix bug97220
                auto displayName = FileUtils::getFileDisplayName(tmp);
                if(displayName.isEmpty()){
                    displayName = QObject::tr("Computer");
                }
                return displayName;
            }
            tmp = nullptr;
            QStringList names;
            for (auto uri : uris) {
                //FIXME: replace BLOCKING api in ui thread.
                auto displayName = FileUtils::getFileDisplayName(uri);
                if (tmp == nullptr)
                    tmp = displayName;
                else
                    tmp = tmp + ", " + displayName;
            }
            return tmp;
        }
    }
    return nullptr;
}

const QString SearchVFSUriParser::addSearchKey(const QString &uri, const bool &search_engine)
{
    QString search_str = uri;
    if (search_engine) {
        search_str += "&search_engine=1";
    } else {
        search_str += "&search_engine=0";
    }
    return search_str;
}
