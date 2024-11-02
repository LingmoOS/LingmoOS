/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef Traditional2SimplifiedTrie_H
#define Traditional2SimplifiedTrie_H

#include "storage-base.hpp"

const char * const  TRADITIONAL_CHINESE_SIMPLIFIED_DICT_PATH = DICT_INSTALL_PATH"/TraditionalChineseSimplifiedDict.txt";

class Traditional2SimplifiedTrie : public StorageBase<char, false, CacheFileHeaderBase>
{
public:
    Traditional2SimplifiedTrie(string dat_cache_path = "");
    Traditional2SimplifiedTrie(const vector<string> file_paths, string dat_cache_path = "");
    void LoadSourceFile(const string &dat_cache_file, const string &md5) override;
    string Find(const string &key);
    bool IsTraditional(const string &word);

private:
    void LoadDict(const int &fd, int &write_bytes, int &offset, int &elements_num);
};

#endif // Traditional2SimplifiedTrie_H
