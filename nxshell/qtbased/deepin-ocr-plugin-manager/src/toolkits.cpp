#include "toolkits.h"

#include <string.h>
#include <algorithm>

#include <dirent.h>
#include <sys/stat.h>
#include <dlfcn.h>

std::vector<std::string> getSubDirNames(const std::string &rootDir)
{
    std::vector<std::string> result;

    DIR *dir;
    struct dirent *ptr;
    if (!(dir = opendir(rootDir.c_str()))) {
        return result;
    }

    struct stat s_buf;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            stat((rootDir + "/" + ptr->d_name).c_str(), &s_buf);
            if (S_ISDIR(s_buf.st_mode)) {
                result.push_back(ptr->d_name);
            }
        }
    }

    return result;
}

std::vector<std::string> getSubDirFullPaths(const std::string &rootDir)
{
    std::vector<std::string> result;

    auto names = getSubDirNames(rootDir);
    std::transform(names.begin(), names.end(), std::back_inserter(result), [rootDir](const std::string & name) {
        return rootDir + "/" + name;
    });

    return result;
}

std::string getCurrentModuleDir()
{
    Dl_info info;
    int rc;

    rc = dladdr(reinterpret_cast<void *>(getCurrentModuleDir), &info);

    std::string modelFullPath = info.dli_fname;
    size_t lastPos = modelFullPath.find_last_of('/');
    std::string result;
    result.assign(modelFullPath.begin(), modelFullPath.begin() + static_cast<long int>(lastPos));

    return result;
}
