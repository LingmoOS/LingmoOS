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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#include "details-properties-page-factory.h"
#include "details-properties-page.h"
#include "file-info.h"

using namespace Peony;

static DetailsPropertiesPageFactory *global_instance = nullptr;

DetailsPropertiesPageFactory *DetailsPropertiesPageFactory::getInstance()
{
    if (!global_instance)
        global_instance = new DetailsPropertiesPageFactory;
    return global_instance;
}

DetailsPropertiesPageFactory::DetailsPropertiesPageFactory()
{

}

DetailsPropertiesPageFactory::~DetailsPropertiesPageFactory()
{

}

bool DetailsPropertiesPageFactory::supportUris(const QStringList &uris)
{
    //FIXME: 需要明确支持范围
    //FIXME: Need to clarify the scope of support
    //只支持文件和应用
    //Only supports files and applications
    if (uris.count() != 1)
        return false;

    QString uri = uris.first();

    if (uri.startsWith("computer://") || uri.startsWith("recent://") || uri.startsWith("trash://"))
        return false;

    //FIXME: 替换ui线程中的阻塞API
    //FIXME: replace BLOCKING api in ui thread.
    auto fileInfo = FileInfo::fromUri(uri);
    FileInfoJob *job = new FileInfoJob(fileInfo);
    job->setAutoDelete(true);
    job->querySync();

    //排除文件夹和卷
    //Exclude folders and volumes
    if (fileInfo.get()->isDir() || fileInfo.get()->isVolume())
        return false;

    return true;
}

PropertiesWindowTabIface *DetailsPropertiesPageFactory::createTabPage(const QStringList &uris)
{
    return new DetailsPropertiesPage(uris.first());
}

void DetailsPropertiesPageFactory::closeFactory()
{
    this->deleteLater();
}
