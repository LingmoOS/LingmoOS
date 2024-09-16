// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSADAPTER_H
#define FSADAPTER_H

#include <QObject>

#include "co/fs.h"
#include "common/commonstruct.h"

//namespace deamon_core {

class FSAdapter : public QObject
{
    Q_OBJECT
public:
    explicit FSAdapter(QObject *parent = nullptr);

    static int getFileEntry(const char *path, FileEntry **entry);
    static bool newFile(const char *path, bool isdir);
    static bool newFileByFullPath(const char *fullpath, bool isdir);
    static bool writeBlock(const char *name, int64 seek_len, const char *data, size_t size);
    static bool writeBlock(const char *name, int64 seek_len, const char *data, size_t size,
                           const int flags, fs::file **fx);
    static bool reacquirePath(fastring filepath, fastring *newpath);

signals:

public slots:

private:
    static co::map<int32, fastring> _filemaps_cache;

};

//} // namespace deamon_core

#endif // FSADAPTER_H
