// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGLIBUTILS_H
#define DGLIBUTILS_H

#include <QObject>

enum DGlibUserDirectory
{
    USER_DIRECTORY_DESKTOP,
    USER_DIRECTORY_DOCUMENTS,
    USER_DIRECTORY_DOWNLOAD,
    USER_DIRECTORY_MUSIC,
    USER_DIRECTORY_PICTURES,
    USER_DIRECTORY_PUBLIC_SHARE,
    USER_DIRECTORY_TEMPLATES,
    USER_DIRECTORY_VIDEOS,
    USER_N_DIRECTORIES
};
Q_ENUMS(DGlibUserDirectory);

enum DGlibFormatSizeFlag
{
    FORMAT_SIZE_DEFAULT = 0x0,
    FORMAT_SIZE_LONG_FORMAT = 1 << 0,
    FORMAT_SIZE_IEC_UNITS = 1 << 1,
    FORMAT_SIZE_BITS = 1 << 2
};
Q_DECLARE_FLAGS(DGlibFormatSizeFlags, DGlibFormatSizeFlag)

class DGlibUtils
{
public:
    static QString userSpecialDir(DGlibUserDirectory userDirectory);
    static QStringList systemDataDirs();
    static QString userDataDir();
    static QString tmpDir();
    static QString formatSize(quint64 size, DGlibFormatSizeFlags flags = FORMAT_SIZE_DEFAULT);
};

#endif // DGLIBUTILS_H
