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
#ifndef STORAGEBASE_CPP
#define STORAGEBASE_CPP

#include "storage-base.h"

template<const bool ordered, typename cache_file_header>
StorageBase<ordered, cache_file_header>::StorageBase(const vector<string> file_paths, string dat_cache_path)
    :m_file_paths(file_paths), m_dat_cache_path(dat_cache_path), m_double_array_data_trie(new cedar::da<int, -1, -2, ordered>)
{
    static_assert(std::is_base_of<CacheFileHeaderBase, header_type>::value, "CacheFileHeader class not derived from CacheFileHeaderBase!");
}

template<const bool ordered, typename cache_file_header>
void StorageBase<ordered, cache_file_header>::Init()
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

template<const bool ordered, typename cache_file_header>
string StorageBase<ordered, cache_file_header>::Find(const string &key)
{
    int result = m_double_array_data_trie->template exactMatchSearch<int>(key.c_str(), key.size());
    if (result < 0)
        return {};
    return string(&m_elements_ptr[result]);
}

template<const bool ordered, typename cache_file_header>
bool StorageBase<ordered, cache_file_header>::Contains(string &word)
{
    if (this->Find(word) != string())
        return true;
    return false;
}

template<const bool ordered, typename cache_file_header>
bool StorageBase<ordered, cache_file_header>::IsMultiTone(const string &word)
{
    string result = this->Find(word);
    if (result.find(",") == result.npos)
        return true;
    return false;
}

template<const bool ordered, typename cache_file_header>
int StorageBase<ordered, cache_file_header>::GetTotalDictSize() const
{
    return m_total_dict_size;
}

template<const bool ordered, typename cache_file_header>
StorageBase<ordered, cache_file_header>::~StorageBase()
{
    munmap(m_mmap_addr, m_mmap_length);
    m_mmap_addr = nullptr;
    close(m_mmap_fd);
    m_mmap_fd = -1;

    if (m_double_array_data_trie)
        delete m_double_array_data_trie;
    m_double_array_data_trie = nullptr;
}

template<const bool ordered, typename cache_file_header>
cedar::da<int, -1, -2, ordered> *StorageBase<ordered, cache_file_header>::GetDoubleArrayDataTrie()
{
    return m_double_array_data_trie;
}

template<const bool ordered, typename cache_file_header>
const void *StorageBase<ordered, cache_file_header>::GetDataTrieArray()
{
    return m_double_array_data_trie->array();
}

template<const bool ordered, typename cache_file_header>
int StorageBase<ordered, cache_file_header>::GetDataTrieSize()
{
    return m_double_array_data_trie->size();
}

template<const bool ordered, typename cache_file_header>
int StorageBase<ordered, cache_file_header>::GetDataTrieTotalSize()
{
    return m_double_array_data_trie->total_size();
}

template<const bool ordered, typename cache_file_header>
cache_file_header *StorageBase<ordered, cache_file_header>::GetCacheFileHeaderPtr()
{
    return reinterpret_cast<header_type*>(m_mmap_addr);
}


template<const bool ordered, typename cache_file_header>
bool StorageBase<ordered, cache_file_header>::InitAttachDat(const string &dat_cache_file, const string &md5)
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
            or m_mmap_length != sizeof(header_type) + header.elements_size  + header.dat_size * m_double_array_data_trie->unit_size()) {
        munmap(m_mmap_addr, m_mmap_length);
        m_mmap_addr = nullptr;
        close(m_mmap_fd);
        m_mmap_fd = -1;
        return false;
    }

    m_elements_ptr = (const char *)(m_mmap_addr + sizeof(header_type));
    const char * dat_ptr = m_mmap_addr + sizeof(header_type) + header.elements_size;
    this->m_double_array_data_trie->set_array((char *)dat_ptr, header.dat_size);
    return true;
}

string CalcFileListMD5(const vector<string> &files_list, int &file_size_sum) {
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
#endif
