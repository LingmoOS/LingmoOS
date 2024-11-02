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
#ifndef IdfTrie_H
#define IdfTrie_H

#include "storage-base.hpp"

const char * const  IDF_DICT_PATH = DICT_INSTALL_PATH"/idf.utf8";

struct IdfCacheFileHeader : CacheFileHeaderBase
{
    double idf_average = 0;
};

class IdfTrie : public StorageBase<double, false, IdfCacheFileHeader>
{
public:
    IdfTrie(const vector<string> file_paths, string dat_cache_path);
    IdfTrie(string file_path, string dat_cache_path);
    void LoadSourceFile(const string &dat_cache_file, const string &md5) override;
    double Find(const string &key) const;
    double GetIdfAverage() const;

private:

};

#endif // IdfTrie_H
