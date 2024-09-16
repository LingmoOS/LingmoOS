// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin_fileencrypt.h"
#include "dbus/diskencryptdbus.h"

FILE_ENCRYPT_USE_NS

void FileEncryptPlugin::initialize()
{
}

bool FileEncryptPlugin::start()
{
    mng.reset(new DiskEncryptDBus(this));
    return true;
}
