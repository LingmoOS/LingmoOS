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
#include "chinese-segmentation.h"
#include "chinese-segmentation-private.h"

ChineseSegmentationPrivate::ChineseSegmentationPrivate(ChineseSegmentation *parent) : q(parent)
{
    //const char * const DICT_PATH = "/usr/share/lingmo-search/res/dict/jieba.dict.utf8";
    const char * const  HMM_PATH = DICT_INSTALL_PATH"/hmm_model.utf8";
    //const char * const USER_DICT_PATH = "/usr/share/lingmo-search/res/dict/user.dict.utf8";
    //const char * const  IDF_PATH = "/usr/share/lingmo-search/res/dict/idf.utf8";
    const char * const  STOP_WORD_PATH = DICT_INSTALL_PATH"/stop_words.utf8";
    m_jieba = new cppjieba::Jieba(DICT_PATH,
                                  HMM_PATH,
                                  USER_DICT_PATH,
                                  IDF_DICT_PATH,
                                  STOP_WORD_PATH,
                                  "");
}

ChineseSegmentationPrivate::~ChineseSegmentationPrivate() {
    if(m_jieba)
        delete m_jieba;
    m_jieba = nullptr;
}

vector<KeyWord> ChineseSegmentationPrivate::callSegment(const string &sentence) {
    const size_t topk = -1;
    vector<KeyWord> keywordres;
    ChineseSegmentationPrivate::m_jieba->extractor.Extract(sentence, keywordres, topk);

    return keywordres;

}

vector<KeyWord> ChineseSegmentationPrivate::callSegment(QString &sentence) {
    //'\xEF\xBC\x8C' is "，" "\xE3\x80\x82" is "。"  use three " " to replace ,to ensure the offset info.
    sentence = sentence.replace("\t", " ").replace("\xEF\xBC\x8C", "   ").replace("\xE3\x80\x82", "   ");
    const size_t topk = -1;
    vector<KeyWord> keywordres;
    ChineseSegmentationPrivate::m_jieba->extractor.Extract(sentence.left(20480000).toStdString(), keywordres, topk);

    return keywordres;

}

vector<string> ChineseSegmentationPrivate::callMixSegmentCutStr(const string &sentence)
{
    vector<string> keywordres;
    ChineseSegmentationPrivate::m_jieba->Cut(sentence, keywordres);
    return keywordres;
}

vector<Word> ChineseSegmentationPrivate::callMixSegmentCutWord(const string &sentence)
{
    vector<Word> keywordres;
    ChineseSegmentationPrivate::m_jieba->Cut(sentence, keywordres);
    return keywordres;
}

string ChineseSegmentationPrivate::lookUpTagOfWord(const string &word)
{
    return ChineseSegmentationPrivate::m_jieba->LookupTag(word);
}

vector<pair<string, string>> ChineseSegmentationPrivate::getTagOfWordsInSentence(const string &sentence)
{
     vector<pair<string, string>> words;
     ChineseSegmentationPrivate::m_jieba->Tag(sentence, words);
     return words;
}

vector<Word> ChineseSegmentationPrivate::callFullSegment(const string &sentence)
{
    vector<Word> keywordres;
    ChineseSegmentationPrivate::m_jieba->CutAll(sentence, keywordres);
    return keywordres;
}

vector<Word> ChineseSegmentationPrivate::callQuerySegment(const string &sentence)
{
    vector<Word> keywordres;
    ChineseSegmentationPrivate::m_jieba->CutForSearch(sentence, keywordres);
    return keywordres;
}

vector<Word> ChineseSegmentationPrivate::callHMMSegment(const string &sentence)
{
    vector<Word> keywordres;
    ChineseSegmentationPrivate::m_jieba->CutHMM(sentence, keywordres);
    return keywordres;
}

vector<Word> ChineseSegmentationPrivate::callMPSegment(const string &sentence)
{
    size_t maxWordLen = 512;
    vector<Word> keywordres;
    ChineseSegmentationPrivate::m_jieba->CutSmall(sentence, keywordres, maxWordLen);
    return keywordres;
}

ChineseSegmentation *ChineseSegmentation::getInstance()
{
    static ChineseSegmentation *global_instance_chinese_segmentation = new ChineseSegmentation;
    return global_instance_chinese_segmentation;
}

vector<KeyWord> ChineseSegmentation::callSegment(const string &sentence)
{
    return d->callSegment(sentence);
}

vector<KeyWord> ChineseSegmentation::callSegment(QString &sentence)
{
    return d->callSegment(sentence);
}

vector<string> ChineseSegmentation::callMixSegmentCutStr(const string &sentence)
{
    return d->callMixSegmentCutStr(sentence);
}

vector<Word> ChineseSegmentation::callMixSegmentCutWord(const string &str)
{
    return d->callMixSegmentCutWord(str);
}

string ChineseSegmentation::lookUpTagOfWord(const string &word)
{
    return d->lookUpTagOfWord(word);
}

vector<pair<string, string> > ChineseSegmentation::getTagOfWordsInSentence(const string &sentence)
{
    return d->getTagOfWordsInSentence(sentence);
}

vector<Word> ChineseSegmentation::callFullSegment(const string &sentence)
{
    return d->callFullSegment(sentence);
}

vector<Word> ChineseSegmentation::callQuerySegment(const string &sentence)
{
    return d->callQuerySegment(sentence);
}

vector<Word> ChineseSegmentation::callHMMSegment(const string &sentence)
{
    return d->callHMMSegment(sentence);
}

vector<Word> ChineseSegmentation::callMPSegment(const string &sentence)
{
    return d->callMPSegment(sentence);
}

ChineseSegmentation::ChineseSegmentation() : d(new ChineseSegmentationPrivate)
{
}
