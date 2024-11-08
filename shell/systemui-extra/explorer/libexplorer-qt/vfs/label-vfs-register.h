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
 * Authors: Ling Yang <yangling@kylinos.cn>
 *
 */

#ifndef LABELVFSREGISTER_H
#define LABELVFSREGISTER_H

#include "explorer-core_global.h"
#include "vfs-plugin-iface.h"

namespace Peony {

class LabelVFSInternalPlugin : public VFSPluginIface
{
public:
    LabelVFSInternalPlugin () {}

    virtual PluginType pluginType () override {return VFSPlugin;}

    virtual void setEnable (bool enable);
    virtual bool isEnable () {return true;}
    virtual const QIcon icon () override {return QIcon();}
    virtual const QString name () override {return "favorite vfs";}
    virtual const QString description () override {return QObject::tr("Default favorite vfs of explorer");}

    void initVFS () override;
    bool holdInSideBar () override {return false;}
    QString uriScheme () override {return "labels://";}
    void* parseUriToVFSFile (const QString &uri) override;
    CustomErrorHandler *customErrorHandler() override {return nullptr;}
};

class PEONYCORESHARED_EXPORT LabelVFSRegister
{
public:
    static void registLabelVFS ();

private:
    LabelVFSRegister ();
};

}
#endif // LABELVFSREGISTER_H
