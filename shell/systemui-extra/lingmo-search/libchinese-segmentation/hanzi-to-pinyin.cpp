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

#include <mutex>
#include <cctype>
#include "hanzi-to-pinyin.h"
#include "hanzi-to-pinyin-private.h"
#include "chinese-segmentation.h"
#include "cppjieba/Unicode.hpp"

HanZiToPinYin * HanZiToPinYin::g_pinYinManager = nullptr;
std::once_flag g_singleFlag;

bool HanZiToPinYinPrivate::contains(string &word)
{
    return m_pinYinTrie.Contains(word);
}

int HanZiToPinYinPrivate::getResults(string &word, QStringList &results)
{
    results.clear();

    string directResult = m_pinYinTrie.Find(word);

    if (directResult == string()) {
        if (m_segType == SegType::NoSegmentation) {//无分词、无结果直接返回-1
            return -1;
        } else {//无结果、启用分词
            vector<string> segResults = ChineseSegmentation::getInstance()->callMixSegmentCutStr(word);
            string data;
            for (string &info : segResults) {
                if (info == string()) {
                    continue;
                }
                data = m_pinYinTrie.Find(info);
                if (data == string()) {//分词后无结果
                    if (cppjieba::IsSingleWord(info)) {//单个字符
                        if (m_exDataProcessType == ExDataProcessType::Default) {//原数据返回
                            results.append(QString().fromStdString(info));
                        } else if (m_exDataProcessType == ExDataProcessType::Delete) {//忽略
                            continue;
                        }
                    } else {//多个字符
                        string oneWord;
                        cppjieba::RuneStrArray runeArray;
                        cppjieba::DecodeRunesInString(info, runeArray);
                        for (auto i = runeArray.begin(); i != runeArray.end(); ++i) {
                            oneWord = cppjieba::GetStringFromRunes(info, i, i);
                            data = m_pinYinTrie.Find(oneWord);
                            if (data == string()) {//单字无结果则按设置返回
                                if (m_exDataProcessType == ExDataProcessType::Default) {//原数据返回
                                    results.append(QString().fromStdString(oneWord));
                                } else if (m_exDataProcessType == ExDataProcessType::Delete) {//忽略
                                    continue;
                                }
                            }
                            if (m_polyphoneType == PolyphoneType::Enable) {//启用多音字
                                results.append(QString().fromStdString(data));
                            } else if (m_polyphoneType == PolyphoneType::Disable) {//不启用多音字
                                if (limonp::IsInStr(data, ',')) {
                                    results.append(QString().fromStdString(data.substr(0, data.find_first_of(",", 0))));
                                } else {
                                    results.append(QString().fromStdString(data));
                                }
                            }
                        }
                    }
                } else {//分词后有结果
                    if (cppjieba::IsSingleWord(info)) {//单个字符
                        if (m_polyphoneType == PolyphoneType::Enable) {//启用多音字
                            results.append(QString().fromStdString(data));
                        } else if (m_polyphoneType == PolyphoneType::Disable) {//不启用多音字
                            if (limonp::IsInStr(data, ',')) {
                                results.append(QString().fromStdString(data.substr(0, data.find_first_of(",", 0))));
                            } else {
                                results.append(QString().fromStdString(data));
                            }
                        }
                    } else {//多个字符
                        vector<string> dataVec = limonp::Split(data, "/");
                        if (dataVec.size() == 1) {//无多音词
                            vector<string> dataVec = limonp::Split(data, ",");
                            for (auto &oneResult : dataVec) {
                                results.append(QString().fromStdString(oneResult));
                            }
                        } else {
                            if (m_polyphoneType == PolyphoneType::Enable) {//启用多音字
                                int wordSize = limonp::Split(dataVec[0], ",").size();
                                for (int i = 0; i < wordSize; ++i) {
                                    QStringList oneResult;
                                    for (size_t j = 0; j < dataVec.size(); ++j) {
                                        oneResult.append(QString().fromStdString(limonp::Split(dataVec[j], ",")[i]));
                                    }
                                    results.append(oneResult.join('/'));
                                }
                            } else if (m_polyphoneType == PolyphoneType::Disable) {//不启用多音字
                                vector<string> tmp = limonp::Split(dataVec[0], ",");
                                for (auto &oneResult : tmp) {
                                    results.append(QString().fromStdString(oneResult));
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {//可以直接查到结果
        if (cppjieba::IsSingleWord(word)) {//单个字符
            if (m_polyphoneType == PolyphoneType::Enable) {//启用多音字
                results.append(QString().fromStdString(directResult));
            } else if (m_polyphoneType == PolyphoneType::Disable) {//不启用多音字
                if (limonp::IsInStr(directResult, ',')) {
                    results.append(QString().fromStdString(directResult.substr(0, directResult.find_first_of(",", 0))));
                } else {
                    results.append(QString().fromStdString(directResult));
                }
            }
        } else {//多个字符
            vector<string> dataVec = limonp::Split(directResult, "/");
            if (dataVec.size() == 1) {//无多音词
                vector<string> dataVec = limonp::Split(directResult, ",");
                for (auto &oneResult : dataVec) {
                    results.append(QString().fromStdString(oneResult));
                }
            } else {
                if (m_polyphoneType == PolyphoneType::Enable) {//启用多音字
                    int wordSize = limonp::Split(dataVec[0], ",").size();
                    for (int i = 0; i < wordSize; ++i) {
                        QStringList oneResult;
                        for (size_t j = 0; j < dataVec.size(); ++j) {
                            oneResult.append(QString().fromStdString(limonp::Split(dataVec[j], ",")[i]));
                        }
                        results.append(oneResult.join('/'));
                    }
                } else if (m_polyphoneType == PolyphoneType::Disable) {//不启用多音字
                    vector<string> tmp = limonp::Split(dataVec[0], ",");
                    for (auto &oneResult : tmp) {
                        results.append(QString().fromStdString(oneResult));
                    }
                }
            }
        }
    }
    convertDataStyle(results);
    return 0;//todo
}

void HanZiToPinYinPrivate::setConfig(PinyinDataStyle dataStyle, SegType segType, PolyphoneType polyphoneType, ExDataProcessType processType)
{
    m_pinyinDataStyle = dataStyle;
    m_segType = segType;
    m_polyphoneType = polyphoneType;
    m_exDataProcessType = processType;
}

void HanZiToPinYinPrivate::convertDataStyle(QStringList &results)
{
    QString value;
    if (m_pinyinDataStyle == PinyinDataStyle::Default) {
        for (QString &info : results) {
            if(info == ",") {
                continue;
            }
            //if info's length was been changed, there's someting wrong while traverse the chars of info
            for (const QChar &c : info) {
                if (!isalpha(c.toLatin1())) {
                    value = PhoneticSymbol.value(c);
                    if (!value.isEmpty()) {
                        info.replace(c, value.at(0));
                    }
                }
            }

            QStringList tmpList = info.split(',', QString::SkipEmptyParts); //去重(保持原顺序)
            QStringList tmpValue;
            for (auto &str : tmpList) {
                if (!tmpValue.contains(str)) {
                    tmpValue.push_back(str);
                }
            }
            info = tmpValue.join(",");
        }
    } else if (m_pinyinDataStyle == PinyinDataStyle::Tone) {
        //无需处理
    } else if (m_pinyinDataStyle == PinyinDataStyle::Tone2) {
        for (QString &info : results) {
            for (int i = 0; i < info.size();) {
                auto c = info.at(i);
                if (!isalpha(c.toLatin1())) {
                    value = PhoneticSymbol.value(c);
                    if (!value.isEmpty()) {
                        info.replace(c, PhoneticSymbol.value(c));
                        i += PhoneticSymbol.value(c).size();
                        continue;
                    }
                }
                i++;
            }
        }
    } else if (m_pinyinDataStyle == PinyinDataStyle::Tone3) {
        for (QString &info : results) {
            if(info == "/") {
                continue;
            }
            bool isPolyphoneWords(false);
            if (info.contains("/")) {
                isPolyphoneWords = true;
                info.replace("/", ",");
            }

            for (int i = 0; i < info.size();) {
                auto c = info.at(i);
                if (!isalpha(c.toLatin1())) {
                    value = PhoneticSymbol.value(c);
                    if (!value.isEmpty()) {
                        info.replace(i, 1, value.at(0));
                        //多音词模式
                        if (info.contains(",")) {
                            int pos = info.indexOf(',', i);
                            if (isPolyphoneWords) {
                                info.replace(",", "/");
                            }
                            //最后一个读音时
                            if (pos == -1) {
                                info.append(value.at(1));
                                break;
                            }
                            info.insert(pos, value.at(1));
                            i = pos + 1;    //insert导致','的位置加一，将i行进到','的位置
                            i++;
                            continue;
                        } else {
                            info.append(value.at(1));
                            break;
                        }
                    }
                }
                i++;
            }

        }
    } else if (m_pinyinDataStyle == PinyinDataStyle::FirstLetter) {
        for (QString &info : results) {
            if(info == "," or info == "/") {
                continue;
            }

            bool isPolyphoneWords(false);
            if (info.contains("/")) {
                isPolyphoneWords = true;
                info.replace("/", ",");
            }

            for (int i = 0; i < info.size();i++) {
                auto c = info.at(i);
                if (!isalpha(c.toLatin1())) {
                    value = PhoneticSymbol.value(c);
                    if (!value.isEmpty()) {
                        info.replace(c, value.at(0));
                    }
                }
            }

            QStringList tmpList = info.split(',', QString::SkipEmptyParts); //去重(保持原顺序)
            QStringList tmpValue;
            for (auto &str : tmpList) {
                if (!tmpValue.contains(str)) {
                    tmpValue.push_back(str.at(0));
                }
            }
            if (isPolyphoneWords) {
                info = tmpValue.join("/");
            } else {
                info = tmpValue.join(",");
            }
        }
    } else if (m_pinyinDataStyle == PinyinDataStyle::English) {
        //暂不支持
    }
}

HanZiToPinYinPrivate::HanZiToPinYinPrivate(HanZiToPinYin *parent) : q(parent)
{
    //const char * const  SINGLE_WORD_PINYIN_PATH = "/usr/share/lingmo-search/res/dict/singleWordPinyin.txt";
    //const char * const  WORDS_PINYIN_PATH = "/usr/share/lingmo-search/res/dict/wordsPinyin.txt";
    //m_pinYinTrie = new Pinyin4cppDictTrie(SINGLE_WORD_PINYIN_PATH, WORDS_PINYIN_PATH);
    //m_pinYinTrie = new Pinyin4cppTrie;
}

HanZiToPinYinPrivate::~HanZiToPinYinPrivate()
{
//    if (m_pinYinTrie){
//        delete m_pinYinTrie;
//        m_pinYinTrie = nullptr;
//    }
}

HanZiToPinYin * HanZiToPinYin::getInstance()
{
    call_once(g_singleFlag, []() {
        g_pinYinManager = new HanZiToPinYin;
    });
    return g_pinYinManager;
}

bool HanZiToPinYin::contains(string &word)
{
    return d->contains(word);
}

bool HanZiToPinYin::isMultiTone(string &word)
{
    return d->isMultiTone(word);
}

bool HanZiToPinYin::isMultiTone(string &&word)
{
    return d->isMultiTone(word);
}

bool HanZiToPinYin::isMultiTone(const string &word)
{
    return d->isMultiTone(word);
}

bool HanZiToPinYin::isMultiTone(const string &&word)
{
    return d->isMultiTone(word);
}

int HanZiToPinYin::getResults(string word, QStringList &results)
{
    return d->getResults(word, results);
}

void HanZiToPinYin::setConfig(PinyinDataStyle dataStyle, SegType segType, PolyphoneType polyphoneType, ExDataProcessType processType)
{
    d->setConfig(dataStyle, segType, polyphoneType, processType);
}

HanZiToPinYin::HanZiToPinYin() : d(new HanZiToPinYinPrivate)
{
}
