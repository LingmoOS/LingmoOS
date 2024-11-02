/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: jixiaoxu <jixiaoxu@kylinos.cn>
 *
 */
#ifndef STORAGEBASE_H
#define STORAGEBASE_H
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "Md5.hpp"
#include "StringUtil.hpp"
#include "cedar.h"
using namespace std;

struct CacheFileHeaderBase { //todo 字节对齐
    char     md5_hex[32] = {};
    uint32_t elements_num = 0;
    uint32_t elements_size = 0;
    uint32_t dat_size = 0;
};

template<const bool ordered = false, typename cache_file_header = CacheFileHeaderBase>
class StorageBase
{
public:
    typedef cache_file_header header_type;

    StorageBase(const vector<string> file_paths, string dat_cache_path = "");

    virtual void Init();

    virtual string Find(const string &key);

    virtual bool Contains(string &word);

    virtual bool IsMultiTone(const string &word);

    virtual int GetTotalDictSize() const;

    virtual void LoadSourceFile(const string &dat_cache_file, const string &md5) = 0;

    virtual ~StorageBase();

    cedar::da<int, -1, -2, ordered> * GetDoubleArrayDataTrie();
    const void * GetDataTrieArray();
    int GetDataTrieSize();
    int GetDataTrieTotalSize();

    cache_file_header * GetCacheFileHeaderPtr();

private:
    StorageBase();
    StorageBase(const StorageBase&);
    StorageBase& operator = (const StorageBase&);

    bool InitAttachDat(const string &dat_cache_file, const string &md5);

    vector<string> m_file_paths;
    string m_dat_cache_path;

    cedar::da<int, -1, -2, ordered> * m_double_array_data_trie = nullptr;

    const char * m_elements_ptr = nullptr;

    int    m_mmap_fd = -1;
    int    m_mmap_length = 0;
    char * m_mmap_addr = nullptr;

    int    m_total_dict_size = 0;

};

inline string CalcFileListMD5(const vector<string> &files_list, int & file_size_sum);
#include "storage-base.cpp"
#endif // STORAGEBASE_H
