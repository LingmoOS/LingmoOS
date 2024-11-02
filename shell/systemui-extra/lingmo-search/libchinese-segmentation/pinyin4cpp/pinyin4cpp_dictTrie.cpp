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

#include "pinyin4cpp_dictTrie.h"
#include "malloc.h"

Pinyin4cppDictTrie::Pinyin4cppDictTrie(const string &single_word_dict_path, const string &words_dict_paths, const string &dat_cache_path) {
    Init(single_word_dict_path, words_dict_paths, dat_cache_path);
}

string Pinyin4cppDictTrie::Find(const string &word) const {
    return m_DataTrie.Find(word);
}

bool Pinyin4cppDictTrie::Contains(string &word) {
    if (m_DataTrie.Find(word) != string())
        return true;
    return false;
}

bool Pinyin4cppDictTrie::IsMultiTone(const string &word) {
    string result = m_DataTrie.Find(word);
    if (result.find(",") == result.npos)
        return true;
    return false;
}

size_t Pinyin4cppDictTrie::GetTotalDictSize() const {
    return m_TotalDictSize_;
}

void Pinyin4cppDictTrie::Init(const string &single_word_dict_path, const string &words_dict_paths, string dat_cache_path) {
    const auto dict_list = single_word_dict_path + "|" + words_dict_paths;
    size_t file_size_sum = 0;
    const string md5 = CalcFileListMD5(dict_list, file_size_sum);
    m_TotalDictSize_ = file_size_sum;

    if (dat_cache_path.empty()) {
        dat_cache_path = "/tmp/" + md5 + ".dat_cache";//未指定词库数据文件存储位置的默认存储在tmp目录下
    }
    qDebug() << "#####Pinyin Dict path:" << dat_cache_path.c_str();
    if (m_DataTrie.InitAttachDat(dat_cache_path, md5)) {
        return;
    }

    LoadSingleWordDict(single_word_dict_path);
    LoadWordsDict(words_dict_paths);
    bool build_ret = m_DataTrie.InitBuildDat(m_StaticNodeInfos, dat_cache_path, md5);
    assert(build_ret);
    m_StaticNodeInfos.clear();
    malloc_trim(0);
}

void Pinyin4cppDictTrie::LoadSingleWordDict(const string &filePath) {
    ifstream ifs(filePath.c_str());
    string line;
    vector<string> buf;

    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#")) {
            continue;
        }
        limonp::Split(line, buf, ":");
        assert(buf.size() == SINGLE_WORD_DICT_COLUMN_NUM);
        if (m_StaticNodeInfos.find(buf[2]) != m_StaticNodeInfos.end()) {
            vector<string> tmp;
            bool isfind(false);
            limonp::Split(m_StaticNodeInfos[buf[2]], tmp, ",");
            for (auto &onePinyin:tmp) {
                if (onePinyin == buf[1]) {
                    isfind = true;
                    break;
                }
            }
            if (!isfind) {
                m_StaticNodeInfos[buf[2]] += ("," + buf[2]);
            }
        } else {
            m_StaticNodeInfos[buf[2]] = buf[1];
        }
    }
}

void Pinyin4cppDictTrie::LoadWordsDict(const string &filePath) {
    ifstream ifs(filePath.c_str());
    string line;
    vector<string> buf;
    for (; getline(ifs, line);) {
        if (limonp::StartsWith(line, "#")) {
            continue;
        }
        limonp::Split(line, buf, ":");
        assert(buf.size() == WORDS_DICT_COLUMN_NUM);
        if (m_StaticNodeInfos.find(buf[0]) != m_StaticNodeInfos.end()) {
            vector<string> tmp;
            bool isfind(false);
            limonp::Split(m_StaticNodeInfos[buf[0]], tmp, "/");
            for (auto &onePinyin:tmp) {
                if (onePinyin == buf[1]) {
                    isfind = true;
                    break;
                }
            }
            if (!isfind) {
                m_StaticNodeInfos[buf[0]] += ("/" + buf[1]);
            }
        } else {
            m_StaticNodeInfos[buf[0]] = buf[1];
        }
    }
}

string CalcFileListMD5(const string &files_list, size_t &file_size_sum) {
    limonp::MD5 md5;

    const auto files = limonp::Split(files_list, "|;");
    file_size_sum = 0;

    for (auto const & local_path : files) {
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
