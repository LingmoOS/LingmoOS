/**
 * @brief   Wrapper for Office Open XML (OOXML)
 * @package ooxml
 * @file    ooxml.cpp
 * @author  dmryutov (dmryutov@gmail.com)
 * @date    01.01.2017 -- 18.10.2017
 */
#include <iostream>
#include <zip.h>
#include <string.h>

#include "ooxml.hpp"

namespace ooxml {

void Ooxml::extractFile(const std::string &zipName, const std::string &fileName,
                        pugi::xml_document &tree)
{
    size_t size;
    auto content = getFileContent(zipName, fileName, size);

    //tree.load_string(static_cast<const char*>(content));
    if (content != nullptr) {
        tree.load_buffer(content, size);
        free(content);
    }
}

void Ooxml::extractFile(const std::string &zipName, const std::string &fileName,
                        std::string &buffer)
{
    size_t size;
    auto content = getFileContent(zipName, fileName, size);

    if (content != nullptr) {
        buffer = std::string(static_cast<const char *>(content), size);
        free(content);
    }
}

bool Ooxml::exists(const std::string &zipName, const std::string &fileName)
{
    int errcode = 0;
    auto *archive = zip_open(zipName.c_str(), ZIP_CHECKCONS, &errcode);
    if (!archive)
        return false;

    auto index = zip_name_locate(archive, fileName.c_str(), ZIP_FL_NOCASE);
    zip_close(archive);

    return index != -1;
}

// private:
void *Ooxml::getFileContent(const std::string &zipName, const std::string &fileName, size_t &size)
{
    size = 0;
    int errcode = 0;
    auto *archive = zip_open(zipName.c_str(), ZIP_CHECKCONS, &errcode);
    if (!archive)
        return nullptr;

    zip_stat_t statBuffer;
    if (zip_stat(archive, fileName.c_str(), ZIP_FL_NOCASE, &statBuffer) != 0) {
        zip_close(archive);
        return nullptr;
    }

    auto *zipFile = zip_fopen(archive, fileName.c_str(), ZIP_FL_NOCASE);
    if (!zipFile) {
        zip_close(archive);
        return nullptr;
    }

    char *content = static_cast<char *>(malloc(statBuffer.size));
    memset(content, 0, statBuffer.size);
    if (zip_fread(zipFile, content, statBuffer.size) == -1) {
        zip_close(archive);
        free(content);
        return nullptr;
    }

    size = statBuffer.size;
    zip_fclose(zipFile);
    zip_close(archive);

    return content;
}

}   // End namespace
