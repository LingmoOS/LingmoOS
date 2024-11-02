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
#include <fstream>
#include <iostream>
#include "Md5.hpp"
#include "StringUtil.hpp"
//#define USE_DARTS
#ifdef USE_DARTS
#include "../storage-base/darts-clone/darts.h"
#include <cassert>
#else
#include "../storage-base/cedar/cedar.h"
#endif
using namespace std;

inline string CalcFileListMD5(const vector<string> &files_list, int & file_size_sum)
{
    limonp::MD5 md5;
    file_size_sum = 0;

    for (auto const & local_path : files_list) {
        const int fd = open(local_path.c_str(), O_RDONLY);
        if (fd < 0){
            continue;
        }
        auto const len = lseek(fd, 0, SEEK_END);
        if (len > 0) {
            void * addr = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
            assert(MAP_FAILED != addr);

            md5.Update((unsigned char *) addr, len);
            file_size_sum += len;

            munmap(addr, len);
        }
        close(fd);
    }

    md5.Final();
    return string(md5.digestChars);
}

inline bool isFileExist(const string filePath) {
    ifstream infile(filePath);
    return infile.good();
}

inline void tryRename(string tmpName, string name) {
    if (0 != rename(tmpName.c_str(), name.c_str())) {
        if (isFileExist(name)) {
            remove(tmpName.c_str());
        }
    }
}

struct CacheFileHeaderBase { //todo 字节对齐
    char     md5_hex[32] = {};
    uint32_t elements_num = 0;
    uint32_t elements_size = 0;
    uint32_t dat_size = 0;
};

template<typename element_ptr_type, const bool ordered = false, typename cache_file_header = CacheFileHeaderBase>
class StorageBase
{
public:
    typedef cache_file_header header_type;
#ifdef USE_DARTS
    typedef typename Darts::DoubleArray::result_pair_type result_pair_type;
    StorageBase(const vector<string> file_paths, string dat_cache_path = "")
        :m_file_paths(file_paths), m_dat_cache_path(dat_cache_path), m_double_array_data_trie(new Darts::DoubleArray)
    {
        static_assert(std::is_base_of<CacheFileHeaderBase, header_type>::value, "CacheFileHeader class not derived from CacheFileHeaderBase!");
    }
#else
    typedef typename cedar::da<int, -1, -2, ordered>::result_pair_type result_pair_type;
    StorageBase(const vector<string> file_paths, string dat_cache_path = "")
        :m_file_paths(file_paths), m_dat_cache_path(dat_cache_path)/*, m_double_array_data_trie(new cedar::da<int, -1, -2, ordered>)*/
    {
        static_assert(std::is_base_of<CacheFileHeaderBase, header_type>::value, "CacheFileHeader class not derived from CacheFileHeaderBase!");
    }
#endif
    virtual void Init()
    {
        int file_size_sum = 0;
        const string md5 = CalcFileListMD5(m_file_paths, file_size_sum);
        m_total_dict_size = file_size_sum;

        if (m_dat_cache_path.empty()) {
            m_dat_cache_path = "/tmp/" + md5 + ".dat_";//未指定词库数据文件存储位置的默认存储在tmp目录下
        }
         m_dat_cache_path += VERSION;
        if (InitAttachDat(m_dat_cache_path, md5)) {
            return;
        }

        LoadSourceFile(m_dat_cache_path, md5);//构建DATrie，写入dat文件

        bool build_ret = InitAttachDat(m_dat_cache_path, md5);

        assert(build_ret);
    }

    virtual void LoadSourceFile(const string &dat_cache_file, const string &md5) = 0;

    virtual ~StorageBase()
    {
        munmap(m_mmap_addr, m_mmap_length);
        m_mmap_addr = nullptr;
        close(m_mmap_fd);
        m_mmap_fd = -1;
    }
#ifndef USE_DARTS
    inline int Update(const char* key, size_t len, int val)
    {
        return m_double_array_data_trie.update(key, len, val);
    }
#endif
    inline size_t CommonPrefixSearch(const char* key, result_pair_type* result, size_t result_len) const
    {
        return m_double_array_data_trie.commonPrefixSearch(key, result, result_len);
    }

    inline int ExactMatchSearch(const char* key, size_t len) const
    {
        return m_double_array_data_trie.template exactMatchSearch<int>(key, len);
    }

    inline const void * GetDataTrieArray()
    {
        return m_double_array_data_trie.array();
    }

    inline int GetDataTrieSize()
    {
        return m_double_array_data_trie.size();
    }

    inline int GetDataTrieTotalSize()
    {
        return m_double_array_data_trie.total_size();
    }

    inline cache_file_header * GetCacheFileHeaderPtr() const
    {
        return reinterpret_cast<header_type*>(m_mmap_addr);
    }

    inline const element_ptr_type * GetElementPtr() const
    {
        return m_elements_ptr;
    }

private:
    StorageBase();
    StorageBase(const StorageBase&);
    StorageBase& operator = (const StorageBase&);

    bool InitAttachDat(const string &dat_cache_file, const string &md5)
    {
        m_mmap_fd = open(dat_cache_file.c_str(), O_RDONLY);

        if (m_mmap_fd < 0) {
            return false;
        }

        const auto seek_off = lseek(m_mmap_fd, 0, SEEK_END);
        if (seek_off < 0){
            close(m_mmap_fd);
            m_mmap_fd = -1;
            return false;
        };

        m_mmap_length = seek_off;
        m_mmap_addr = reinterpret_cast<char *>(mmap(NULL, m_mmap_length, PROT_READ, MAP_SHARED, m_mmap_fd, 0));
        if (m_mmap_addr == MAP_FAILED) {
            close(m_mmap_fd);
            m_mmap_fd = -1;
            return false;
        }
        if (m_mmap_length < sizeof(header_type)) {
            munmap(m_mmap_addr, m_mmap_length);
            m_mmap_addr = nullptr;
            close(m_mmap_fd);
            m_mmap_fd = -1;
            return false;
        }
        header_type & header = *reinterpret_cast<header_type*>(m_mmap_addr);

        if (0 != memcmp(&header.md5_hex[0], md5.c_str(), md5.size())
                or m_mmap_length != sizeof(header_type) + header.elements_size  + header.dat_size * m_double_array_data_trie.unit_size()) {
            munmap(m_mmap_addr, m_mmap_length);
            m_mmap_addr = nullptr;
            close(m_mmap_fd);
            m_mmap_fd = -1;
            return false;
        }

        m_elements_ptr = (const element_ptr_type *)(m_mmap_addr + sizeof(header_type));
        const char * dat_ptr = m_mmap_addr + sizeof(header_type) + header.elements_size;
        this->m_double_array_data_trie.set_array((char *)dat_ptr, header.dat_size);
        return true;
    }

    vector<string> m_file_paths;
    string m_dat_cache_path;

#ifdef USE_DARTS
    Darts::DoubleArray m_double_array_data_trie;
#else
    cedar::da<int, -1, -2, ordered> m_double_array_data_trie;
#endif

    const element_ptr_type * m_elements_ptr = nullptr;
    int    m_mmap_fd = -1;
    size_t    m_mmap_length = 0;
    char * m_mmap_addr = nullptr;
    int    m_total_dict_size = 0;

};

#endif // STORAGEBASE_H
