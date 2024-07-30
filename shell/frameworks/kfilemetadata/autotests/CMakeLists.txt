find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

find_package(Python3 COMPONENTS Interpreter)
set_package_properties(Python3 PROPERTIES DESCRIPTION "Python Interpreter"
                       URL "https://www.python.org" TYPE OPTIONAL
                       PURPOSE "Python interpreter is required for testing external extractors and writers")

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

configure_file(indexerextractortestsconfig.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/indexerextractortestsconfig.h @ONLY)


set(KfileMetaDataAutotest_SRCS)
ecm_qt_declare_logging_category(KfileMetaDataAutotest_SRCS
    HEADER kfilemetadata_debug.h
    IDENTIFIER KFILEMETADATA_LOG
    CATEGORY_NAME kf.filemetadata
)


#
# Test case coverage
#
ecm_add_test(extractorcoveragetest.cpp
    TEST_NAME "extractorcoveragetest"
    LINK_LIBRARIES Qt6::Test KF6::FileMetaData
)

#
# Full text extraction test
#
set(text_COMPAT_CODEC)
ecm_add_test(
    indexerextractortests.cpp
    ../src/extractors/plaintextextractor.cpp
    TEST_NAME "indexextractortest"
    LINK_LIBRARIES Qt6::Test KF6::FileMetaData ${text_COMPAT_CODEC} KF6::Codecs
)

#
# Office
#
if(KF6Archive_FOUND)
    ecm_add_test(odfextractortest.cpp ../src/extractors/odfextractor.cpp
        TEST_NAME "odfextractortest"
        LINK_LIBRARIES Qt6::Test Qt6::Xml KF6::FileMetaData KF6::Archive
    )
endif()

if(KF6Archive_FOUND)
    ecm_add_test(office2007extractortest.cpp
        ../src/extractors/office2007extractor.cpp
        ../src/extractors/dublincoreextractor.cpp
        ../src/kfilemetadata_debug.cpp
        TEST_NAME "officeextractortest"
        LINK_LIBRARIES Qt6::Test Qt6::Xml KF6::FileMetaData KF6::Archive
    )
endif()

#
# Poppler
#
if(Poppler_Qt6_FOUND)
    ecm_add_test(popplerextractortest.cpp ../src/extractors/popplerextractor.cpp
        TEST_NAME "popplerextractortest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData Poppler::Qt6
    )
endif()

#
# EPub
#
if(EPUB_FOUND)
    include_directories(${EPUB_INCLUDE_DIR})

    ecm_add_test(
        epubextractortest.cpp
	../src/extractors/epubextractor.cpp
	../src/kfilemetadata_debug.cpp
        TEST_NAME "epubextractortest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData ${EPUB_LIBRARIES}
    )
endif()

#
# Mobi
#
if (QMobipocket_FOUND)
    ecm_add_test(mobiextractortest.cpp ../src/extractors/mobiextractor.cpp
        TEST_NAME "mobiextractortest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData qmobipocket
    )
endif()

#
# FictionBook2
#
if(KF6Archive_FOUND)
    ecm_add_test(
        fb2extractortest.cpp
        ../src/extractors/fb2extractor.cpp
        ../src/kfilemetadata_debug.cpp
        TEST_NAME "fb2extractortest"
        LINK_LIBRARIES Qt6::Test KF6::Archive KF6::FileMetaData
    )
endif()

#
# Property Info
#
add_executable(propertyinfotest_bin propertyinfotest.cpp)
target_link_libraries(propertyinfotest_bin
    Qt6::Test
    KF6::FileMetaData
)
ecm_mark_as_test(propertyinfotest_bin)
ecm_mark_nongui_executable(propertyinfotest_bin)
# Run the benchmark with just 1 iteration, so we known it works
add_test(NAME propertyinfotest_en COMMAND propertyinfotest_bin "-iterations" "1")
add_test(NAME propertyinfotest_localized COMMAND propertyinfotest_bin "--localized" "-iterations" "1")

#
# Property Info
#
add_executable(typeinfotest_bin typeinfotest.cpp)
target_link_libraries(typeinfotest_bin
    Qt6::Test
    KF6::FileMetaData
)
ecm_mark_as_test(typeinfotest_bin)
ecm_mark_nongui_executable(typeinfotest_bin)
# Run the benchmark with just 1 iteration, so we known it works
add_test(NAME typeinfotest_en COMMAND typeinfotest_bin "-iterations" "1")
add_test(NAME typeinfotest_localized COMMAND typeinfotest_bin "--localized" "-iterations" "1")

#
# Exiv2
#
if(LibExiv2_FOUND)
    kde_enable_exceptions()
    ecm_add_test(exiv2extractortest.cpp ../src/extractors/exiv2extractor.cpp
        TEST_NAME "exiv2extractortest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData LibExiv2::LibExiv2
    )
    if (${LibExiv2_VERSION} VERSION_LESS 0.28.0)
	# exiv2 0.27.x still uses std::auto_ptr, which is incompatible with C++17
        set_target_properties(kfilemetadata_exiv2extractor PROPERTIES CXX_STANDARD 14)
    endif()
    target_include_directories(exiv2extractortest PRIVATE ${CMAKE_BINARY_DIR}/src/extractors)
endif()

#
# FFMPEG
#
if(FFMPEG_FOUND)
    kde_enable_exceptions()
    ecm_add_test(
        ffmpegextractortest.cpp
        ../src/extractors/ffmpegextractor.cpp
        ../src/kfilemetadata_debug.cpp
        TEST_NAME "ffmpegextractortest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData ${AVCODEC_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVUTIL_LIBRARIES}
    )
    target_include_directories(ffmpegextractortest SYSTEM PRIVATE ${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
endif()

#
# TagLib
#
if(Taglib_FOUND)
    kde_enable_exceptions()
    ecm_add_test(taglibextractortest.cpp ../src/extractors/taglibextractor.cpp ${KfileMetaDataAutotest_SRCS}
        TEST_NAME "taglibextractortest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData Taglib::Taglib
    )
endif()


if(TARGET Python3::Interpreter AND NOT WIN32)
    # Test relies on working she-bang, which does non exist on Windows
    configure_file(samplefiles/testexternalextractor/main.py samplefiles/testexternalextractor/main.py)
    configure_file(samplefiles/testexternalextractor/manifest.json samplefiles/testexternalextractor/manifest.json COPYONLY)

    ecm_add_test(externalextractortest.cpp $<$<BOOL:${BUILD_SHARED_LIBS}>:../src/externalextractor.cpp> ${KfileMetaDataAutotest_SRCS}
            TEST_NAME "externalextractortest"
            LINK_LIBRARIES Qt6::Test KF6::FileMetaData KF6::I18n
    )
endif()

#
# Collection
#
ecm_add_test(extractorcollectiontest.cpp
        TEST_NAME "extractorcollectiontest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData
)

if(Taglib_FOUND)
    ecm_add_test(writercollectiontest.cpp
            TEST_NAME "writercollectiontest"
            LINK_LIBRARIES Qt6::Test KF6::FileMetaData
    )
endif()

#
# XML
#
ecm_add_test(
    xmlextractortest.cpp
    ../src/extractors/xmlextractor.cpp
    ../src/extractors/dublincoreextractor.cpp
    ../src/kfilemetadata_debug.cpp
    TEST_NAME "xmlextractortest"
    LINK_LIBRARIES Qt6::Test Qt6::Xml KF6::FileMetaData
)
if(KF6Archive_FOUND)
    target_link_libraries(xmlextractortest KF6::Archive)
    target_compile_definitions(xmlextractortest PRIVATE SVG_XML_COMPRESSED_SUPPORT=1)
endif()

#
# Postscript DSC
#
ecm_add_test(
    postscriptdscextractortest.cpp
    ../src/extractors/postscriptdscextractor.cpp
    ../src/kfilemetadata_debug.cpp
    TEST_NAME "postscriptdscextractortest"
    LINK_LIBRARIES Qt6::Test KF6::FileMetaData
)

#
# AppImage
#
if(libappimage_FOUND AND KF6Config_FOUND AND Qt6Gui_FOUND)
    ecm_add_test(appimageextractortest.cpp ../src/extractors/appimageextractor.cpp
        TEST_NAME "appimageextractortest"
        LINK_LIBRARIES
            Qt6::Test
            KF6::FileMetaData
            KF6::ConfigCore
            Qt6::Xml
            Qt6::Gui
            libappimage
    )
endif()

#
# PNG
#
if(Qt6Gui_FOUND)
    ecm_add_test(pngextractortest.cpp ../src/extractors/pngextractor.cpp
        TEST_NAME "pngextractortest"
        LINK_LIBRARIES
            Qt6::Test
            KF6::FileMetaData
            Qt6::Gui
    )
endif()

#
# Krita
#
if(KF6Archive_FOUND)
    ecm_add_test(kritaextractortest.cpp ../src/extractors/kritaextractor.cpp
        TEST_NAME "kritaextractortest"
        LINK_LIBRARIES
        KF6::Archive
        KF6::FileMetaData
        Qt6::Test
        )
endif()

################
# Writer tests #
################

#
# UserMetaData
#
if(UNIX)
    kde_enable_exceptions()
    ecm_add_test(usermetadatawritertest.cpp ../src/usermetadata.cpp
        TEST_NAME "usermetadatawritertest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData
    )
endif()

#
# TagLib
#
if(Taglib_FOUND)
    kde_enable_exceptions()
    ecm_add_test(taglibwritertest.cpp ../src/writers/taglibwriter.cpp ${KfileMetaDataAutotest_SRCS}
        TEST_NAME "taglibwritertest"
        LINK_LIBRARIES Qt6::Test KF6::FileMetaData Taglib::Taglib
    )
endif()

if(TARGET Python3::Interpreter AND NOT WIN32)
    configure_file(samplefiles/testexternalwriter/main.py samplefiles/testexternalwriter/main.py)
    configure_file(samplefiles/testexternalwriter/manifest.json samplefiles/testexternalwriter/manifest.json COPYONLY)

    ecm_add_test(externalwritertest.cpp $<$<BOOL:${BUILD_SHARED_LIBS}>:../src/externalwriter.cpp> ${KfileMetaDataAutotest_SRCS}
            TEST_NAME "externalwritertest"
            LINK_LIBRARIES Qt6::Test KF6::FileMetaData KF6::I18n
    )
endif()
