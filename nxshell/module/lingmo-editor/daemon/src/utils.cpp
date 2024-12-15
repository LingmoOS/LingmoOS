// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <QDir>
#include <QFileInfo>

bool Utils::fileExists(QString path) {
    QFileInfo check_file(path);
    
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

bool Utils::fileIsWritable(QString path)
{
    QFileDevice::Permissions permissions = QFile(path).permissions();
    
    return permissions & QFileDevice::WriteUser;
}
