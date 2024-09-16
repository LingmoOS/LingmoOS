#pragma once

#include <string>
#include <vector>

#include <stdio.h>
#define DEEPIN_LOG(...) do { \
    fprintf(stderr, ##__VA_ARGS__); fprintf(stderr, "\n"); } while(0)

//获取rootDir下所有子文件夹的完整路径（不进行递归搜索）
extern std::vector<std::string> getSubDirFullPaths(const std::string &rootDir);

//获取rootDir下所有子文件夹的名字（不进行递归搜索）
std::vector<std::string> getSubDirNames(const std::string &rootDir);

//获取动态库当前目录
std::string getCurrentModuleDir();
