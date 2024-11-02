/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 */
#ifndef CHINESESEGMENTATION_H
#define CHINESESEGMENTATION_H

#include <QString>
#include "libchinese-segmentation_global.h"
#include "common-struct.h"

class ChineseSegmentationPrivate;
class CHINESESEGMENTATION_EXPORT ChineseSegmentation {
public:
    static ChineseSegmentation *getInstance();

    /**
     * @brief ChineseSegmentation::callSegment
     * 调用extractor进行关键词提取，先使用Mix方式初步分词，再使用Idf词典进行关键词提取，只包含两字以上关键词
     *
     * @param sentence 要提取关键词的句子
     * @return vector<KeyWord> 存放提取后关键词的信息的容器
     */
    vector<KeyWord> callSegment(const string &sentence);
    vector<KeyWord> callSegment(QString &sentence);

    /**
     * @brief ChineseSegmentation::callMixSegmentCutStr
     * 使用Mix方法进行分词，即先使用最大概率法MP初步分词，再用隐式马尔科夫模型HMM进一步分词，可以准确切出词典已有词和未登录词，结果比较准确
     *
     * @param sentence 要分词的句子
     * @return vector<string> 只存放分词后每个词的内容的容器
     */
    vector<string> callMixSegmentCutStr(const string& sentence);

    /**
     * @brief ChineseSegmentation::callMixSegmentCutWord
     * 和callMixSegmentCutStr功能相同
     * @param sentence 要分词的句子
     * @return vector<Word> 存放分词后每个词所有信息的容器
     */
    vector<Word> callMixSegmentCutWord(const string& str);

    /**
     * @brief ChineseSegmentation::lookUpTagOfWord
     * 查询word的词性
     * @param word 要查询词性的词
     * @return string word的词性
     */
    string lookUpTagOfWord(const string& word);

    /**
     * @brief ChineseSegmentation::getTagOfWordsInSentence
     * 使用Mix分词后获取每个词的词性
     * @param sentence 要分词的句子
     * @return vector<pair<string, string>> 分词后的每个词的内容(firsr)和其对应的词性(second)
     */
    vector<pair<string, string>> getTagOfWordsInSentence(const string &sentence);

    /**
     * @brief ChineseSegmentation::callFullSegment
     * 使用Full进行分词，Full会切出字典里所有的词。
     * @param sentence 要分词的句子
     * @return vector<Word> 存放分词后每个词所有信息的容器
     */
    vector<Word> callFullSegment(const string& sentence);

    /**
     * @brief ChineseSegmentation::callQuerySegment
     * 使用Query进行分词，即先使用Mix，对于长词再用Full，结果最精确，但词的数量也最大
     * @param sentence 要分词的句子
     * @return vector<Word> 存放分词后每个词所有信息的容器
     */
    vector<Word> callQuerySegment(const string& sentence);

    /**
     * @brief ChineseSegmentation::callHMMSegment
     * 使用隐式马尔科夫模型HMM进行分词
     * @param sentence 要分词的句子
     * @return vector<Word> 存放分词后每个词所有信息的容器
     */
    vector<Word> callHMMSegment(const string& sentence);

    /**
     * @brief ChineseSegmentation::callMPSegment
     * 使用最大概率法MP进行分词
     * @param sentence 要分词的句子
     * @return vector<Word> 存放分词后每个词所有信息的容器
     */
    vector<Word> callMPSegment(const string& sentence);

private:
    explicit ChineseSegmentation();
    ~ChineseSegmentation() = default;
    ChineseSegmentation(const ChineseSegmentation&) = delete;
    ChineseSegmentation& operator =(const ChineseSegmentation&) = delete;

private:
    ChineseSegmentationPrivate *d = nullptr;
};

#endif // CHINESESEGMENTATION_H
