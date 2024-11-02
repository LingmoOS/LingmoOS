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
#include "idf-trie.h"

IdfTrie::IdfTrie(const vector<string> file_paths, string dat_cache_path)
    : StorageBase<double, false, IdfCacheFileHeader>(file_paths, dat_cache_path)
{
    this->Init();
}

IdfTrie::IdfTrie(string file_path, string dat_cache_path)
: StorageBase<double, false, IdfCacheFileHeader>(vector<string>{file_path}, dat_cache_path)
{
    this->Init();
}

void IdfTrie::LoadSourceFile(const string &dat_cache_file, const string &md5)
{
    IdfCacheFileHeader header;
    assert(sizeof(header.md5_hex) == md5.size());
    memcpy(&header.md5_hex[0], md5.c_str(), md5.size());

    int offset(0), elements_num(0), write_bytes(0), data_trie_size(0);
    double idf_sum(0), idf_average(0), tmp(0);
    string tmp_filepath = string(dat_cache_file) + "_XXXXXX";
    umask(S_IWGRP | S_IWOTH);
    const int fd =mkstemp((char *)tmp_filepath.data());
    assert(fd >= 0);
    fchmod(fd, 0644);

    write_bytes = write(fd, (const char *)&header, sizeof(IdfCacheFileHeader));

    ifstream ifs(IDF_DICT_PATH);
    string line;
    vector<string> buf;

    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#") or line.empty()) {
            continue;
        }
        limonp::Split(line, buf, " ");
        if (buf.size() != 2)
            continue;
        this->Update(buf[0].c_str(), buf[0].size(), elements_num);
        offset += sizeof(double);
        elements_num++;
        tmp = atof(buf[1].c_str());
        write_bytes += write(fd, &tmp, sizeof(double));
        idf_sum += tmp;
    }
    idf_average = idf_sum / elements_num;
    write_bytes += write(fd, this->GetDataTrieArray(), this->GetDataTrieTotalSize());

    lseek(fd, sizeof(header.md5_hex), SEEK_SET);
    write(fd, &elements_num, sizeof(int));
    write(fd, &offset, sizeof(int));
    data_trie_size = this->GetDataTrieSize();
    write(fd, &data_trie_size, sizeof(int));
    write(fd, &idf_average, sizeof(double));

    close(fd);
    assert((size_t)write_bytes == sizeof(IdfCacheFileHeader) + offset + this->GetDataTrieTotalSize());

    tryRename(tmp_filepath, dat_cache_file);
}

double IdfTrie::Find(const string &key) const
{
    int result = this->ExactMatchSearch(key.c_str(), key.size());
    if (result < 0)
        return -1;
    return this->GetElementPtr()[result];
}

double IdfTrie::GetIdfAverage() const
{
    return this->GetCacheFileHeaderPtr()->idf_average;
}

