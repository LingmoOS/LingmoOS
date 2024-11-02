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
#include "pinyin4cpp-trie.h"

Pinyin4cppTrie::Pinyin4cppTrie(string dat_cache_path)
    : StorageBase<char, false, CacheFileHeaderBase>(vector<string>{SINGLE_WORD_PINYIN_PATH, WORDS_PINYIN_PATH}, dat_cache_path)
{
    this->Init();
}

Pinyin4cppTrie::Pinyin4cppTrie(const vector<string> file_paths, string dat_cache_path)
    : StorageBase<char, false, CacheFileHeaderBase>(file_paths, dat_cache_path)
{
    this->Init();
}



bool Pinyin4cppTrie::Contains(string &word) {
    if (this->Find(word) != string())
        return true;
    return false;
}

bool Pinyin4cppTrie::IsMultiTone(const string &word) {
    string result = this->Find(word);
    if (result.find(",") == result.npos)
        return true;
    return false;
}

void Pinyin4cppTrie::LoadSourceFile(const string &dat_cache_file, const string &md5)
{
    CacheFileHeaderBase header;
    assert(sizeof(header.md5_hex) == md5.size());
    memcpy(&header.md5_hex[0], md5.c_str(), md5.size());

    int offset(0), elements_num(0), write_bytes(0), data_trie_size(0);
    string tmp_filepath = string(dat_cache_file) + "_XXXXXX";
    umask(S_IWGRP | S_IWOTH);
    const int fd =mkstemp((char *)tmp_filepath.data());
    assert(fd >= 0);
    fchmod(fd, 0644);

    write_bytes = write(fd, (const char *)&header, sizeof(CacheFileHeaderBase));

    this->LoadSingleWordDict(fd, write_bytes, offset, elements_num);
    this->LoadWordsDict(fd, write_bytes, offset, elements_num);

    write_bytes += write(fd, this->GetDataTrieArray(), this->GetDataTrieTotalSize());

    lseek(fd, sizeof(header.md5_hex), SEEK_SET);
    write(fd, &elements_num, sizeof(int));
    write(fd, &offset, sizeof(int));
    data_trie_size = this->GetDataTrieSize();
    write(fd, &data_trie_size, sizeof(int));

    close(fd);
    assert((size_t)write_bytes == sizeof(CacheFileHeaderBase) + offset + this->GetDataTrieTotalSize());

    tryRename(tmp_filepath, dat_cache_file);
}

string Pinyin4cppTrie::Find(const string &key)
{
    int result = this->ExactMatchSearch(key.c_str(), key.size());
    if (result < 0)
        return string();
    return string(&this->GetElementPtr()[result]);
}

void Pinyin4cppTrie::LoadSingleWordDict(const int &fd, int &write_bytes, int &offset, int &elements_num)
{
    ifstream ifs(SINGLE_WORD_PINYIN_PATH);
    string line;
    vector<string> buf;

    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#") or line.empty()) {
            continue;
        }
        limonp::Split(line, buf, ":");
        if (buf.size() != 3)
            continue;
        this->Update(buf[2].c_str(), buf[2].size(), offset);
        offset += (buf[1].size() + 1);
        elements_num++;
        write_bytes += write(fd, buf[1].c_str(), buf[1].size() + 1);
    }
}

void Pinyin4cppTrie::LoadWordsDict(const int &fd, int &write_bytes, int &offset, int &elements_num)
{
    ifstream ifs(WORDS_PINYIN_PATH);
    string line;
    vector<string> buf;
    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#") or line.empty()) {
            continue;
        }
        limonp::Split(line, buf, ":");
        if (buf.size() != 2)
            continue;
        this->Update(buf[0].c_str(), buf[0].size(), offset);
        offset += (buf[1].size() + 1);
        elements_num++;
        write_bytes += write(fd, buf[1].c_str(), buf[1].size() + 1);
    }
}
