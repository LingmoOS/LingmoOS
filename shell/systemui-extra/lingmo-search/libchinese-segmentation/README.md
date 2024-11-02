# chinese-segmentation

#### 介绍
libchinese-segmentation工程以单例的形式分别提供了中文分词、汉字转拼音和中文繁体简体转换功能。

接口文件分别为:
chinese-segmentation.h
libchinese-segmentation_global.h
common-struct.h

hanzi-to-pinyin.h
pinyin4cpp-common.h

Traditional-to-Simplified.h
安装路径:/usr/include/chinese-seg

#### 使用说明

其中中文分词相关功能由chinese-segmentation.h提供接口，主要包括以下功能函数：

```
   static ChineseSegmentation *getInstance();//全局单例
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

```

汉字转拼音相关功能由hanzi-to-pinyin.h提供接口，主要包括以下功能函数：

```
    static HanZiToPinYin * getInstance();//全局单例

    /**
     * @brief HanZiToPinYin::isMultiTone 判断是否为多音字/词/句
     * @param word 要判断的字/词/句
     * @return bool 不是返回false
     */
    bool isMultiTone(string &word);
    bool isMultiTone(string &&word);
    bool isMultiTone(const string &word);
    bool isMultiTone(const string &&word);

    /**
     * @brief HanZiToPinYin::contains 查询某个字/词/句是否有拼音（是否在数据库包含）
     * @param word 要查询的字/词/句
     * @return bool 数据库不包含返回false
     */
    bool contains(string &word);

    /**
     * @brief HanZiToPinYin::getResults 获取某个字/词/句的拼音
     * @param word 要获取拼音的字/词/句
     * @param results word的拼音列表（有可能多音字），每次调用results会被清空
     * @return int 获取到返回0，否则返回-1
     */
    int getResults(string word, QStringList &results);

    /**
     * @brief setConfig 设置HanZiToPinYin的各项功能，详见pinyin4cpp-common.h
     * @param dataStyle 返回数据风格，默认defult
     * @param segType 是否启用分词，默认启用
     * @param polyphoneType 是否启用多音字，默认不启用
     * @param processType 无拼音数据处理模式，默认defult
     */
    void setConfig(PinyinDataStyle dataStyle,SegType segType,PolyphoneType polyphoneType,ExDataProcessType processType);

```

中文繁体转简体相关功能由Traditional-to-Simplified.h提供接口，主要包括以下功能函数：

```
    static Traditional2Simplified * getInstance();//全局单例
    /**
     * @brief Traditional2Simplified::isMultiTone 判断是否为繁体字，是则返回true
     * @param oneWord 要判断的字
     * @return bool 不是返回false
     */
    bool isTraditional(string &oneWord);

    /**
     * @brief Traditional2Simplified::getResults 转换某个字/词/句的繁体字
     * @param words 要转换为简体中文的字/词/句
     * @return words 的简体中文结果
     */
    string getResults(string words);

```

除此之外工程中提供了测试程序位于chinese-segmentation/test，运行界面如下：
![输入图片说明](https://foruda.gitee.com/images/1682048388802220746/245a2ec3_8021248.png "image.png")

#### 参与贡献

1.  Fork 本仓库
2.  新建分支
3.  提交代码
4.  新建 Pull Request

