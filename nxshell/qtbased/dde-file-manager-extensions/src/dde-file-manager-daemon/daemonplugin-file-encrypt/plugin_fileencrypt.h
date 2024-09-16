// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEENCRYPTPLUGIN_H
#define FILEENCRYPTPLUGIN_H

#include "daemonplugin_file_encrypt_global.h"

#include <dfm-framework/dpf.h>

FILE_ENCRYPT_BEGIN_NS
class DiskEncryptDBus;
class DAEMONPLUGIN_FILE_ENCRYPT_EXPORT FileEncryptPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "plugin_fileencrypt.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<DiskEncryptDBus> mng;
};

FILE_ENCRYPT_END_NS

#endif   // FILEENCRYPTPLUGIN_H
