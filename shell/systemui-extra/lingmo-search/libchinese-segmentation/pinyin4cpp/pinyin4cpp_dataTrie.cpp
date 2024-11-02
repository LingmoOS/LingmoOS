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

#include "pinyin4cpp_dataTrie.h"

Pinyin4cppDataTrie::Pinyin4cppDataTrie()
{

}

Pinyin4cppDataTrie::~Pinyin4cppDataTrie()
{
    munmap(m_mmapAddr, m_mmapLength);
    m_mmapAddr = nullptr;
    close(m_mmapFd);
    m_mmapFd = -1;
}

string Pinyin4cppDataTrie::Find(const string &key) const {
//    darts-clone的接口方法
    Darts::DoubleArray::result_pair_type find_result;
    m_DoubleArrayDataTrie.exactMatchSearch(key.c_str(), find_result);
    if ((0 == find_result.length) || (find_result.value < 0) || ((size_t)find_result.value >= m_elementsSize)) {//todo
        return string();
    }
    return string(&m_elementsPtr[find_result.value]);

//  cedarpp的接口方法
//    int result = m_DoubleArrayDataTrie.exactMatchSearch<int>(key.c_str(), key.size());
//    if (result < 0)
//        return string();
//    return string(&m_elementsPtr[result]);

}

bool Pinyin4cppDataTrie::InitBuildDat(map<string, string> &elements, const string &dat_cache_file, const string &md5) {
    BuildDatCache(elements, dat_cache_file, md5);
    return InitAttachDat(dat_cache_file, md5);
}

bool Pinyin4cppDataTrie::InitAttachDat(const string &dat_cache_file, const string &md5) {
    m_mmapFd = open(dat_cache_file.c_str(), O_RDONLY);

    if (m_mmapFd < 0) {
        return false;
    }

    const auto seek_off = lseek(m_mmapFd, 0, SEEK_END);
    assert(seek_off >= 0);

    m_mmapLength = static_cast<size_t>(seek_off);
    m_mmapAddr = reinterpret_cast<char *>(mmap(NULL, m_mmapLength, PROT_READ, MAP_SHARED, m_mmapFd, 0));
    assert(MAP_FAILED != m_mmapAddr);
    assert(m_mmapLength >= sizeof(CacheFileHeader));

    CacheFileHeader & header = *reinterpret_cast<CacheFileHeader*>(m_mmapAddr);
    m_elementsNum = header.elements_num;
    m_elementsSize = header.elements_size;
    assert(sizeof(header.md5_hex) == md5.size());

    if (0 != memcmp(&header.md5_hex[0], md5.c_str(), md5.size())) {
        return false;
    }

    assert(m_mmapLength == sizeof(CacheFileHeader) + header.elements_size  + header.dat_size * m_DoubleArrayDataTrie.unit_size());

    m_elementsPtr = (const char *)(m_mmapAddr + sizeof(CacheFileHeader));
    const char * dat_ptr = m_mmapAddr + sizeof(CacheFileHeader) + header.elements_size;
    m_DoubleArrayDataTrie.set_array((char *)dat_ptr, header.dat_size);
    return true;
}

void Pinyin4cppDataTrie::BuildDatCache(map<string, string> &elements, const string &dat_cache_file, const string &md5) {
    vector<const char*> keys_ptr_vec;
    vector<int> values_vec;
    vector<string> mem_elem_vec;

    keys_ptr_vec.reserve(elements.size());
    values_vec.reserve(elements.size());
    mem_elem_vec.reserve(elements.size());

    CacheFileHeader header;
    assert(sizeof(header.md5_hex) == md5.size());
    memcpy(&header.md5_hex[0], md5.c_str(), md5.size());

    int offset(0);
    for (auto &info:elements) {
        keys_ptr_vec.push_back(info.first.c_str());
        values_vec.push_back(offset);
        offset += (info.second.size() + 1);//+1指字符串后加\0
        assert(info.second.size() > 0);
        mem_elem_vec.push_back(info.second);
    }

    auto const ret = m_DoubleArrayDataTrie.build(keys_ptr_vec.size(), &keys_ptr_vec[0], NULL, &values_vec[0]);
    assert(0 == ret);
    header.elements_num = mem_elem_vec.size();
    header.elements_size = offset;
    header.dat_size = m_DoubleArrayDataTrie.size();

    string tmp_filepath = string(dat_cache_file) + "_XXXXXX";
    umask(S_IWGRP | S_IWOTH);
    const int fd =mkstemp((char *)tmp_filepath.data());
    assert(fd >= 0);
    fchmod(fd, 0644);

    auto write_bytes = write(fd, (const char *)&header, sizeof(header));
    for (size_t i = 0; i < elements.size(); ++i) {
        write_bytes += write(fd, mem_elem_vec[i].c_str(), mem_elem_vec[i].size() + 1);
    }
    write_bytes += write(fd, m_DoubleArrayDataTrie.array(), m_DoubleArrayDataTrie.total_size());

    assert((size_t)write_bytes == sizeof(header) + offset + m_DoubleArrayDataTrie.total_size());
    close(fd);

    const auto rename_ret = rename(tmp_filepath.c_str(), dat_cache_file.c_str());
    assert(0 == rename_ret);
}
