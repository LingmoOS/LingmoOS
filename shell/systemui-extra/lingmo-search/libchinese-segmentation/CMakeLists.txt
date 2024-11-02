cmake_minimum_required(VERSION 3.14)
project(chinese-segmentation LANGUAGES CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 1)
set(VERSION_MICRO 0)
set(CHINESE_SEGMENTATION_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

set(HEADERS
        chinese-segmentation.h
        common-struct.h
        hanzi-to-pinyin.h
        Traditional-to-Simplified.h
        pinyin4cpp-common.h
        libchinese-segmentation_global.h)

set(CHINESE_SEGMENTATION_SRC
        Traditional-Chinese-Simplified-conversion/Traditional2Simplified_trie.cpp
        Traditional-Chinese-Simplified-conversion/Traditional2Simplified_trie.h
        Traditional-to-Simplified.cpp
        Traditional-to-Simplified-private.h
        chinese-segmentation.cpp
        chinese-segmentation-private.h
        cppjieba/DatTrie.hpp
        cppjieba/DictTrie.hpp
        cppjieba/FullSegment.hpp
        cppjieba/HMMModel.hpp
        cppjieba/HMMSegment.hpp
        cppjieba/IdfTrie.hpp
        cppjieba/Jieba.hpp
        cppjieba/KeywordExtractor.hpp
        cppjieba/MPSegment.hpp
        cppjieba/MixSegment.hpp
        cppjieba/PinYinTrie.hpp
        cppjieba/PosTagger.hpp
        cppjieba/PreFilter.hpp
        cppjieba/QuerySegment.hpp
        cppjieba/SegmentBase.hpp
        cppjieba/SegmentTagged.hpp
        cppjieba/TextRankExtractor.hpp
        cppjieba/Unicode.hpp
        cppjieba/idf-trie/idf-trie.cpp cppjieba/idf-trie/idf-trie.h
        cppjieba/limonp/ArgvContext.hpp
        cppjieba/limonp/BlockingQueue.hpp
        cppjieba/limonp/BoundedBlockingQueue.hpp
        cppjieba/limonp/BoundedQueue.hpp
        cppjieba/limonp/Closure.hpp
        cppjieba/limonp/Colors.hpp
        cppjieba/limonp/Condition.hpp
        cppjieba/limonp/Config.hpp
        cppjieba/limonp/FileLock.hpp
        cppjieba/limonp/ForcePublic.hpp
        cppjieba/limonp/LocalVector.hpp
        cppjieba/limonp/Logging.hpp
        cppjieba/limonp/Md5.hpp
        cppjieba/limonp/MutexLock.hpp
        cppjieba/limonp/NonCopyable.hpp
        cppjieba/limonp/StdExtension.hpp
        cppjieba/limonp/StringUtil.hpp
        cppjieba/limonp/Thread.hpp
        cppjieba/limonp/ThreadPool.hpp
        cppjieba/segment-trie/segment-trie.cpp
        cppjieba/segment-trie/segment-trie.h
        hanzi-to-pinyin.cpp
        hanzi-to-pinyin-private.h
        pinyin4cpp/pinyin4cpp-trie.cpp
        pinyin4cpp/pinyin4cpp-trie.h
        pinyin4cpp/pinyin4cpp_dataTrie.cpp
        pinyin4cpp/pinyin4cpp_dataTrie.h
        pinyin4cpp/pinyin4cpp_dictTrie.cpp
        pinyin4cpp/pinyin4cpp_dictTrie.h
        storage-base/cedar/cedar.h
        storage-base/cedar/cedarpp.h
        storage-base/darts-clone/darts.h
        storage-base/storage-base.cpp
        storage-base/storage-base.h
        storage-base/storage-base.hpp)

add_library(chinese-segmentation SHARED
        ${CHINESE_SEGMENTATION_SRC}
        ${HEADERS}
        )

include_directories(chinese-segmentation
        storage-base/cedar
        storage-base
        cppjieba
        cppjieba/limonp
        pinyin4cpp
        Traditional-Chinese-Simplified-conversion
        )

target_link_libraries(chinese-segmentation PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/chinese-segmentation")
set(HEADERS_INSTALL_DIR "/usr/include/chinese-segmentation")
set(PC_INSTALL_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")
set(DICT_INSTALL_PATH "/usr/share/chinese-segmentation/res/dict")
add_compile_definitions(
        VERSION="${CHINESE_SEGMENTATION_VERSION}"
        DICT_INSTALL_PATH="${DICT_INSTALL_PATH}"
)
target_include_directories(chinese-segmentation INTERFACE  $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/chinese-segmentation-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/chinese-segmentation-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR})

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/chinese-segmentation-config-version.cmake
        VERSION ${CHINESE_SEGMENTATION_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/chinese-segmentation.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/chinese-segmentation.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR})

set_target_properties(chinese-segmentation PROPERTIES
        VERSION ${CHINESE_SEGMENTATION_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME chinese-segmentation
        )
install(TARGETS chinese-segmentation
        EXPORT chinese-segmentation
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        )
install(EXPORT chinese-segmentation
        FILE chinese-segmentation-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/chinese-segmentation.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/chinese-segmentation-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/chinese-segmentation-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
set(DICT_FILES
        dict/hmm_model.utf8
        dict/idf.utf8
        dict/jieba.dict.utf8
        dict/stop_words.utf8
        dict/user.dict.utf8
        dict/pinyinWithoutTone.txt
        dict/pos_dict/char_state_tab.utf8
        dict/pos_dict/prob_emit.utf8
        dict/pos_dict/prob_start.utf8
        dict/pos_dict/prob_trans.utf8
        pinyin4cpp/dict/singleWordPinyin.txt
        pinyin4cpp/dict/wordsPinyin.txt
        Traditional-Chinese-Simplified-conversion/dict/TraditionalChineseSimplifiedDict.txt
        )
install(FILES ${DICT_FILES} DESTINATION ${DICT_INSTALL_PATH})

if (BUILD_TEST)
    add_subdirectory(test)
endif ()



