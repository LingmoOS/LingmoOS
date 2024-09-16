// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsadapter.h"
#include "message.pb.h"
#include "utils/utils.h"
#include "utils/config.h"
#include "common/constant.h"

#include "co/log.h"
#include "co/path.h"

//using namespace deamon_core;

FSAdapter::FSAdapter(QObject *parent)
    : QObject(parent)
{
}

int FSAdapter::getFileEntry(const char *path, FileEntry **entry)
{
    FileEntry *temp = *entry;
    if (!fs::exists(path)) {
        ELOG << "FSAdapter::getFileEntry path not exists: " << path;
        return -1;
    }

    if (fs::isdir(path)) {
        temp->filetype = (FileType::DIR);
    } else {
        temp->filetype = (FileType::FILE_B);
    }

    fastring name = Util::parseFileName(path);
    temp->name = (name.c_str());
    if (name.starts_with('.')) {
        temp->hidden = (true);
    } else {
        temp->hidden = (false);
    }

    temp->size = (fs::fsize(path));
    temp->modified_time = (fs::mtime(path));

    return 0;
}

bool FSAdapter::newFile(const char *path, bool isdir)
{
    fastring fullpath = path::join(DaemonConfig::instance()->getStorageDir(), path);
    if (isdir) {
        fs::mkdir(fullpath, true);
    } else {
        fastring parent = path::dir(fullpath);
        fs::mkdir(parent, true);   // 创建文件保存的根/子目录
        if (!fs::exists(fullpath)) {
            fs::file fx(fullpath, 'm');
            fx.close();
        }
    }
    //    LOG << "new file -> fullpath: " << fullpath;

    return fs::exists(fullpath);
}

bool FSAdapter::newFileByFullPath(const char *fullpath, bool isdir)
{
    if (isdir) {
        fs::mkdir(fullpath, true);
    } else {
        fastring parent = path::dir(fullpath);
        fs::mkdir(parent, true);   // 创建文件保存的根/子目录
        if (!fs::exists(fullpath)) {
            fs::file fx(fullpath, 'm');
            fx.close();
        }
    }
    LOG << "new file -> fullpath: " << fullpath;

    return fs::exists(fullpath);
}

bool FSAdapter::writeBlock(const char *name, int64 seek_len, const char *data, size_t size)
{
    fs::file fx(name, 'm');
    if (!fx.exists()) {
        ELOG << "writeBlock File does not exist: " << name;
        return false;
    }

    size_t wirted_size = 0;
    size_t rem_size = size;
    fx.seek(seek_len);
    do {
        size_t wsize = fx.write(data, rem_size);
        if (wsize <= 0) {
            ELOG << "fx write done: " << rem_size << " => " << wsize;
            break;
        }
        wirted_size += wsize;
        rem_size = size - wsize;
    } while (wirted_size < size);
    fx.close();

    return true;
}

bool FSAdapter::writeBlock(const char *name, int64 seek_len,
                           const char *data, size_t size, const int flags, fs::file **fx)
{
    if (flags & JobTransFileOp::FIlE_CREATE) {
        if ((*fx) != nullptr) {
            // 这里创建文件发现文件描述符存在，错误返回 -1
            ELOG << "file flags is create, but file fx is not nullptr, flags = " << flags;
            (*fx)->close();
            delete (*fx);
            (*fx) = nullptr;
            return false;
        }
        fastring parent = path::dir(name);
        fs::mkdir(parent, true);   // 创建文件保存的根/子目录
        (*fx) = new fs::file(name, 'm');
        if (!(*fx)->exists()) {
            ELOG << " file create error , file = " << name << ", flags = " << flags;
            (*fx)->close();
            delete (*fx);
            (*fx) = nullptr;
            return false;
        }
    }

    if ((*fx) == nullptr) {
        ELOG << "fx is nullptr !!!!!! " << name << " flags = " << flags << " len " << size;
        return false;
    }
    bool write = true;
    if (size != 0) {
        size_t wirted_size = 0;
        size_t rem_size = size;
        (*fx)->seek(seek_len);
        do {
            size_t wsize = (*fx)->write(data, rem_size);
            if (wsize <= 0) {
                write = false;
                ELOG << "fx write done: " << rem_size << " => " << wsize;
                break;
            }
            wirted_size += wsize;
            rem_size = size - wsize;
        } while (wirted_size < size);
    }

    if (flags & JobTransFileOp::FILE_CLOSE || !write) {
        (*fx)->close();
        delete (*fx);
        (*fx) = nullptr;
    }

    return write;
}

bool FSAdapter::reacquirePath(fastring filepath, fastring *newpath)
{
    if (!fs::exists(filepath)) {
        return false;
    }

    std::pair<fastring, fastring> sp = path::split(filepath);
    if (sp.first.empty()) {
        ELOG << "the filepath not start with /";
        return false;
    }

    // 获取一个可用不存在的名字路径
    // /a/b/c/d.txt -> /a/b/c/d(1).txt; /a/b/c/d -> /a/b/c/d(1)
    fastring tmpPath = filepath;
    fastring suffix = path::ext(filepath);
    fastring org = sp.second.replace(suffix, "");
    fastring tmpName;
    int n = 1;
    do {
        tmpName = org + "(" << n << ")" + suffix;
        tmpPath = path::join(sp.first, tmpName);
        n++;
    } while (fs::exists(tmpPath));

    LOG << "reacquire " << filepath << " -> " << tmpPath;

    *newpath = tmpPath;
    return true;
}
