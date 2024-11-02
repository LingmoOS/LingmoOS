find_package(QT NAMES Qt6 Qt5 COMPONENTS Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

#
# ffmpeg test
#

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

configure_file(indexerextractortestsconfig.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/indexerextractortestsconfig.h @ONLY)
include_directories(../src)

add_executable(ffmpegExtractorTest
        ffmpeg-extractortest.cpp
        ../src/extractors/ffmpeg-extractor.cpp)
target_include_directories(ffmpegExtractorTest SYSTEM PRIVATE ${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIRS})
target_link_libraries(ffmpegExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        ${AVCODEC_LIBRARIES}
        ${AVFORMAT_LIBRARIES}
        ${AVUTIL_LIBRARIES}
        ${SWSCALE_LIBRARIES}
        )
add_test(ffmpegExtractorTest ${CMAKE_BINARY_DIR}/autotests/ffmpegExtractorTest)

#
# office(binary) test
#

add_executable(OfficeExtractorTest
        ../src/extractors/office-extractor.cpp
        ../src/extractors/binary-parser.cpp
        office-extractortest.cpp)
target_link_libraries(OfficeExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        )
add_test(OfficeExtractortest ${CMAKE_BINARY_DIR}/autotests/OfficeExtractorTest)

#
# office(2007) test
#

add_executable(Office2007ExtractorTest
        ../src/extractors/office2007-extractor.cpp
        office2007-extractortest.cpp)
target_link_libraries(Office2007ExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        libquazip5.so
        )
add_test(Office2007Extractortest ${CMAKE_BINARY_DIR}/autotests/OfficeExtractorTest)

#
# pdf test
#

add_executable(PdfExtractorTest
        ../src/extractors/pdf-extractor.cpp
        pdf-extractortest.cpp)
target_include_directories(PdfExtractorTest SYSTEM PRIVATE ${POPPLER_INCLUDE_DIRS})
target_link_libraries(PdfExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        ${POPPLER_LIBRARIES}
        )
add_test(PdfExtractortest ${CMAKE_BINARY_DIR}/autotests/PdfExtractorTest)

#
# text test
#

add_executable(TextExtractorTest
        ../src/extractors/text-extractor.cpp
        text-extractortest.cpp)
target_include_directories(TextExtractorTest SYSTEM PRIVATE ${UCHARDET_INCLUDE_DIRS})
target_link_libraries(TextExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        ${UCHARDET_LIBRARIES}
        )
add_test(TextExtractortest ${CMAKE_BINARY_DIR}/autotests/TextExtractorTest)

#
# ofd test
#

add_executable(OfdExtractorTest
        ../src/extractors/ofd-extractor.cpp
        ofd-extractortest.cpp)
target_link_libraries(OfdExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        quazip5
        )
add_test(OfdExtractorTest ${CMAKE_BINARY_DIR}/autotests/OfdExtractorTest)

#
# uof test
#

add_executable(UofExtractorTest
        ../src/extractors/uof-extractor.cpp
        uof-extractortest.cpp)
target_link_libraries(UofExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        quazip5
        )
add_test(UofExtractorTest ${CMAKE_BINARY_DIR}/autotests/UofExtractorTest)

#
# png test
#

add_executable(PngExtractorTest
        ../src/extractors/png-extractor.cpp
        png-extractortest.cpp)
target_link_libraries(PngExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        tesseract
        leptonica
)
add_test(PngExtractortest ${CMAKE_BINARY_DIR}/autotests/PngExtractorTest)

#
# taglib test
#

add_executable(TaglibExtractorTest
        ../src/extractors/taglib-extractor.cpp
        taglib-extractortest.cpp)
target_include_directories(TaglibExtractorTest SYSTEM PRIVATE ${TAGLIB_INCLUDE_DIRS})
target_link_libraries(TaglibExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        ${TAGLIB_LIBRARIES}
)
add_test(TaglibExtractorTest ${CMAKE_BINARY_DIR}/autotests/TaglibExtractorTest)

#
# image test
#

add_executable(ImageExtractorTest
        ../src/extractors/image-extractor.cpp
        image-extractortest.cpp)
target_include_directories(ImageExtractorTest SYSTEM PRIVATE ${IMAGE_INCLUDE_DIRS})
target_link_libraries(ImageExtractorTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        lingmo-file-metadata
        ${IMAGE_LIBRARIES}
)
add_test(ImageExtractorTest ${CMAKE_BINARY_DIR}/autotests/ImageExtractorTest)