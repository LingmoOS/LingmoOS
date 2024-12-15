// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfile.h"

#include <unistd.h>
#include <cstring>
#include <string.h> // memset()
#include <glob.h> // glob(), globfree()

DFile::DFile()
{
}
bool DFile::isAbs(std::string file)
{
    char resolved_path[MAX_FILEPATH_LEN];
    if (realpath(file.c_str(), resolved_path)) {
        std::string filePath(resolved_path);
        if (filePath == file)
            return true;
    }
    return false;
}
bool DFile::isExisted(std::string file)
{
    return !access(file.c_str(), F_OK);
}
std::string DFile::dir(std::string file)
{
    std::string ret;
    if (isAbs(file)) {
        size_t pos = file.find_last_of("/");
        if (pos != std::string::npos) {
            ret.assign(file, 0, pos + 1); // 包含结尾斜杠/
        }
    }
    return ret;
}
std::string DFile::base(std::string file)
{
    std::string ret;
    if (strstr(file.c_str(), "/")) {    // 包含路径
        size_t pos = file.find_last_of("/");
        if (pos != std::string::npos) {
            ret.assign(file, pos + 1, file.size() - pos);   // 去除路径
        }
    }
    size_t pos = file.find_last_of(".");    // 去除后缀
    if (pos != std::string::npos) {
        ret.assign(file, 0, pos + 1);
    }
    return ret;
}

std::vector<std::string> DFile::glob(const std::string& pattern) {
    std::vector<std::string> filenames;

    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = ::glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        return  filenames;
    }

    // collect all the filenames into a std::list<std::string>

    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(glob_result.gl_pathv[i]);
    }

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}
