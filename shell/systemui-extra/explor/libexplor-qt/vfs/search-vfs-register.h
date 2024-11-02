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

#ifndef SEARCHVFSREGISTER_H
#define SEARCHVFSREGISTER_H

#include "explor-core_global.h"

#include "vfs-plugin-iface.h"

namespace Peony {

class SearchVFSInternalPlugin : public VFSPluginIface
{
public:
    SearchVFSInternalPlugin() {}

    virtual PluginType pluginType() override {return VFSPlugin;}

    virtual const QString name() override {return "search vfs";}
    virtual const QString description() override {return QObject::tr("Default search vfs of explor");}
    virtual const QIcon icon() override {return QIcon();}
    virtual void setEnable(bool enable);
    virtual bool isEnable() {return true;}

    void initVFS() override;
    QString uriScheme() override {return "search://";}
    bool holdInSideBar() override {return false;}
    void *parseUriToVFSFile(const QString &uri) override;
    CustomErrorHandler *customErrorHandler() override {return nullptr;}
};

class PEONYCORESHARED_EXPORT SearchVFSRegister
{
public:
    static void registSearchVFS();

private:
    SearchVFSRegister();
};

}

#endif // SEARCHVFSREGISTER_H
