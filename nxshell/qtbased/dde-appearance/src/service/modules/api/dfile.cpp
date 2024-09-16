// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfile.h"
#include "../common/commondefine.h"

#include <unistd.h>
#include <cstring>
#include <string.h> // memset()
#include <glob.h> // glob(), globfree()
#include <sys/stat.h>
#include <sys/types.h>
#include <QFile>
#include <QFileInfo>

DFile::DFile()
{
}
bool DFile::isAbs(QString file)
{
    QFileInfo info(file);
    return info.isAbsolute();
}
bool DFile::isExisted(QString file)
{
    return QFileInfo::exists(file);
}
QString DFile::dir(QString file)
{
    QFileInfo info(file);
    return info.absolutePath();
}
QString DFile::base(QString file)
{
    QString ret =file;
    if (file.indexOf("/")!=-1) {    // 包含路径
        int pos = file.lastIndexOf("/");
        ret = file.mid(file.lastIndexOf("/")+1);
    }
    // 去除后缀
    if (ret.lastIndexOf(".") != -1) {
        ret = ret.mid(0,ret.lastIndexOf("."));
    }
    return ret;
}

std::vector<QString> DFile::glob(const QString& pattern) {
    std::vector<QString> filenames;

    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = ::glob(pattern.toStdString().c_str(), GLOB_TILDE, nullptr, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        return  filenames;
    }

    // collect all the filenames into a std::list<QString>

    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(glob_result.gl_pathv[i]);
    }

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}
/*
int mkdirMulti(const char* path) {
    if(!path) {
        printf("path is NULL!!!\n");
        return -1;
    }
    if(access(path, F_OK) == 0) {
        return 0;
    }
    QString directory;
    const char* currentDir = strrchr(path, '/');
    if(currentDir) {
        //走到这里说明传进来的path是不存在的，我们要获取上一层的目录，再做同样的判断
        directory = QString(path, currentDir-path);
        int ret = mkdirMulti(directory.c_str());
        if(ret == 0) {
            //此处if判断是为了"xxx/xxx/"这种格式的路径做处理的，
            //因为"xxx/xxx/"和"xxx/xxx"对于我们代码来说是两种路径，但其实是同一个路径,mkdir会因为递归调用，
            //分别将两者用mkdir创建一次，第二次的创建会因为第一次已经创建了，导致失败， 就会返回一个-1，但其实第一次已经成功了。
            if((access(path, F_OK) == 0)) {
                return 0;
            } else {
                int mkdir_ret = mkdir(path, S_IRWXU);
                return mkdir_ret;
            }
        } else {
            //情况1：最上层的"/xxx"也不存在的时候，directory会是空的，所以递归mkdirMulti的时候会走到判空中，返回-1，这个时候应该去创建目录一下
            //情况2：mkdirMulti递归创建失败，这种情况比较少，比如没有权限，mkdir失败
            int mkdir_ret = mkdir(path, S_IRWXU);
            return mkdir_ret;
        }
    } else {
        //这种是针对直接传"xxx/xxx"的路径
        int mkdir_ret = mkdir(path, S_IRWXU);
        return mkdir_ret;
    }
}
*/
