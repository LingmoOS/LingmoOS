INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/pinyin4cpp-trie.h \
    $$PWD/pinyin4cpp_dataTrie.h \
    $$PWD/pinyin4cpp_dictTrie.h

SOURCES += \
    $$PWD/pinyin4cpp-trie.cpp \
    $$PWD/pinyin4cpp_dataTrie.cpp \
    $$PWD/pinyin4cpp_dictTrie.cpp

DISTFILES += \
    pinyin4cpp/dict/wordsPinyin.txt \
    pinyin4cpp/dict/singleWordPinyin.txt
