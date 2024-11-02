INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/DictTrie.hpp \
    $$PWD/IdfTrie.hpp \
    $$PWD/PinYinTrie.hpp \
    $$PWD/FullSegment.hpp \
    $$PWD/HMMModel.hpp \
    $$PWD/HMMSegment.hpp \
    $$PWD/Jieba.hpp \
    $$PWD/KeywordExtractor.hpp \
    $$PWD/MPSegment.hpp \
    $$PWD/MixSegment.hpp \
    $$PWD/PosTagger.hpp \
    $$PWD/PreFilter.hpp \
    $$PWD/QuerySegment.hpp \
    $$PWD/SegmentBase.hpp \
    $$PWD/SegmentTagged.hpp \
    $$PWD/TextRankExtractor.hpp \
#    $$PWD/Trie.hpp \
    $$PWD/Unicode.hpp \
    $$PWD/DatTrie.hpp \
    $$PWD/idf-trie/idf-trie.h \
    $$PWD/segment-trie/segment-trie.h

DISTFILES += \
    dict/README.md \
    dict/hmm_model.utf8 \
    dict/idf.utf8 \
    dict/jieba.dict.utf8 \
    dict/pos_dict/char_state_tab.utf8 \
    dict/pos_dict/prob_emit.utf8 \
    dict/pos_dict/prob_start.utf8 \
    dict/pos_dict/prob_trans.utf8 \
    dict/stop_words.utf8 \
    dict/user.dict.utf8
    #dict/pinyinWithoutTone.txt \

include(limonp/limonp.pri)

SOURCES += \
    $$PWD/idf-trie/idf-trie.cpp \
    $$PWD/segment-trie/segment-trie.cpp
