/*
 *
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
 */
#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#include <cmath>
#include <limits>
#include "limonp/StringUtil.hpp"
#include "limonp/Logging.hpp"
#include "Unicode.hpp"
#include "DatTrie.hpp"
#include <QDebug>
namespace cppjieba {

using namespace limonp;

const size_t PINYIN_COLUMN_NUM = 2;

class PinYinTrie {
public:
    enum UserWordWeightOption {
        WordWeightMin,
        WordWeightMedian,
        WordWeightMax,
    }; // enum UserWordWeightOption

    PinYinTrie(const string& dict_path, const string & dat_cache_path = "",
             UserWordWeightOption user_word_weight_opt = WordWeightMedian) {
        Init(dict_path, dat_cache_path, user_word_weight_opt);
    }

    ~PinYinTrie() {}

    int getMultiTonResults(string word, QStringList &results) {
        if (qmap_chinese2pinyin.contains(QString::fromStdString(word))) {
            for (auto i:qmap_chinese2pinyin[QString::fromStdString(word)])
                results.push_back(i);
            return 0;
        }
        return -1;
    }

    int getSingleTonResult(string word, QString &result) {
        const PinYinMemElem * tmp = dat_.PinYinFind(word);
        if (tmp) {
            result = QString::fromStdString(tmp->GetTag());
            return 0;
        }
        return -1;
    }

    bool contains(string &word) {
        if (qmap_chinese2pinyin.contains(QString::fromStdString(word))
                or !dat_.PinYinFind(word))
            return true;
//        if (map_chinese2pinyin.contains(word)
//                or !dat_.PinYinFind(word))
//            return true;
        return false;
    }

    bool isMultiTone(const string &word) {
        if (qmap_chinese2pinyin.contains(QString::fromStdString(word)))
            return true;
//        if (map_chinese2pinyin.contains(word))
//            return true;
        return false;
    }

    size_t GetTotalDictSize() const {
        return total_dict_size_;
    }

private:
    void Init(const string& dict_path, string dat_cache_path,
              UserWordWeightOption user_word_weight_opt) {
        size_t file_size_sum = 0;
        vector<PinYinElement> node_infos;
        const string md5 = CalcFileListMD5(dict_path, file_size_sum);
        total_dict_size_ = file_size_sum;

        if (dat_cache_path.empty()) {
            //未指定词库数据文件存储位置的默认存储在tmp目录下--jxx20200519
            dat_cache_path = /*dict_path*/"/tmp/" + md5 + "." + to_string(user_word_weight_opt) +  ".dat_cache";
        }
        QString path = QString::fromStdString(dat_cache_path);
        qDebug() << "#########PinYin path:" << path << file_size_sum;
        if (dat_.InitPinYinAttachDat(dat_cache_path, md5)) {
            //多音字仍需遍历文件信息
            LoadDefaultPinYin(node_infos, dict_path, true);
            return;
        }

        LoadDefaultPinYin(node_infos, dict_path, false);
        double min_weight = 0;
        dat_.SetMinWeight(min_weight);

        const auto build_ret = dat_.InitBuildDat(node_infos, dat_cache_path, md5);
        assert(build_ret);
        vector<PinYinElement>().swap(node_infos);
    }

    void LoadDefaultPinYin(vector<PinYinElement> &node_infos, const string& filePath, bool multiFlag) {
        ifstream ifs(filePath.c_str());
        if(not ifs.is_open()){
            return ;
        }
        XCHECK(ifs.is_open()) << "open " << filePath << " failed.";
        string line;
        vector<string> buf;
        size_t lineno = 0;

        for (; getline(ifs, line); lineno++) {
            if (line.empty()) {
                XLOG(ERROR) << "lineno: " << lineno << " empty. skipped.";
                continue;
            }
            Split(line, buf, " ");
            if (buf.size() == PINYIN_COLUMN_NUM) {
                if (multiFlag) {//非多音字
                    continue;
                }
                PinYinElement node_info;
                node_info.word = buf[1];
                node_info.tag = buf[0];
                node_infos.push_back(node_info);
            } else {//多音字
                QString content = QString::fromUtf8(line.c_str());
                qmap_chinese2pinyin[content.split(" ").last().trimmed()] = content.split(" ");
                qmap_chinese2pinyin[content.split(" ").last().trimmed()].pop_back();
                /*
                 //std map string list
                 list<string> tmpList;
                 for(int i = 0; i < buf.size() - 1; ++i){
                    tmpList.push_back(buf[i]);
                 }
                 map[buf[buf.size() - 1]] = tmpList;
                */
            }
        }
    }

private:
    QMap<QString, QStringList> qmap_chinese2pinyin;
    //map<string, list<string>> map_chinese2pinyin;
    size_t total_dict_size_ = 0;
    DatTrie dat_;
};
}

