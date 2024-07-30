
if(TARGET Poppler::Qt6)
    add_library(kfilemetadata_popplerextractor MODULE popplerextractor.cpp)

    target_link_libraries(kfilemetadata_popplerextractor
        KF6::FileMetaData
        Poppler::Qt6
    )

    set_target_properties(kfilemetadata_popplerextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_popplerextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

endif()

if(Taglib_FOUND)
    add_library(kfilemetadata_taglibextractor MODULE taglibextractor.cpp ${debug_SRCS})
    target_link_libraries( kfilemetadata_taglibextractor
        KF6::FileMetaData
        Taglib::Taglib
    )

    set_target_properties(kfilemetadata_taglibextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_taglibextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
endif()

if(LibExiv2_FOUND)
    add_library(kfilemetadata_exiv2extractor MODULE exiv2extractor.cpp)
    kde_target_enable_exceptions(kfilemetadata_exiv2extractor PRIVATE)
    target_link_libraries(kfilemetadata_exiv2extractor
        KF6::FileMetaData
        LibExiv2::LibExiv2
    )
    if(${LibExiv2_VERSION} VERSION_GREATER_EQUAL 0.27.4)
        set(EXIV2_HAVE_BMFF_SUPPORT "1")
    endif()
    configure_file(exiv2extractor.json.in exiv2extractor.json)

    set_target_properties(kfilemetadata_exiv2extractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")

    if (${LibExiv2_VERSION} VERSION_LESS 0.28.0)
	# exiv2 0.27.x still uses std::auto_ptr, which is incompatible with C++17
        set_target_properties(kfilemetadata_exiv2extractor PROPERTIES CXX_STANDARD 14)
    endif()

    install(
    TARGETS kfilemetadata_exiv2extractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

endif()

if(AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
    add_library(kfilemetadata_ffmpegextractor MODULE
        ffmpegextractor.cpp
        ../kfilemetadata_debug.cpp
    )
    target_include_directories(kfilemetadata_ffmpegextractor SYSTEM PRIVATE ${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
    target_link_libraries(kfilemetadata_ffmpegextractor
        KF6::FileMetaData
	${AVCODEC_LIBRARIES}
	${AVFORMAT_LIBRARIES}
	${AVUTIL_LIBRARIES}
    )

    set_target_properties(kfilemetadata_ffmpegextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_ffmpegextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

endif()


if(EPUB_FOUND)
    add_library(kfilemetadata_epubextractor MODULE
        epubextractor.cpp
        ../kfilemetadata_debug.cpp
    )
    target_include_directories(kfilemetadata_epubextractor SYSTEM PRIVATE ${EPUB_INCLUDE_DIR})
    target_link_libraries(kfilemetadata_epubextractor
        KF6::FileMetaData
        ${EPUB_LIBRARIES}
    )

    set_target_properties(kfilemetadata_epubextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_epubextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

endif()

#
# Plain Text
#
add_library(kfilemetadata_plaintextextractor MODULE plaintextextractor.cpp)

target_link_libraries( kfilemetadata_plaintextextractor
    KF6::FileMetaData
    KF6::Codecs
)

set_target_properties(kfilemetadata_plaintextextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
install(
TARGETS kfilemetadata_plaintextextractor
DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

#
# PO
#
add_library(kfilemetadata_poextractor MODULE poextractor.cpp)
target_link_libraries( kfilemetadata_poextractor
    KF6::FileMetaData
)

set_target_properties(kfilemetadata_poextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
install(
TARGETS kfilemetadata_poextractor
DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

#
# XML
#
set_package_properties(KF6Archive PROPERTIES PURPOSE "- Required for extracting data from compressed SVGs")
add_library(kfilemetadata_xmlextractor MODULE
   dublincoreextractor.cpp
   xmlextractor.cpp
   ../kfilemetadata_debug.cpp
)
target_link_libraries( kfilemetadata_xmlextractor
    KF6::FileMetaData
    Qt6::Core
    Qt6::Xml
)
if(KF6Archive_FOUND)
    target_link_libraries(kfilemetadata_xmlextractor KF6::Archive)
    target_compile_definitions(kfilemetadata_xmlextractor PRIVATE SVG_XML_COMPRESSED_SUPPORT=1)
endif()

set_target_properties(kfilemetadata_xmlextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
install(
    TARGETS kfilemetadata_xmlextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata
)

#
# Postscript DSC
#
add_library(kfilemetadata_postscriptdscextractor MODULE
   postscriptdscextractor.cpp
   ../kfilemetadata_debug.cpp
)
target_link_libraries( kfilemetadata_postscriptdscextractor
    KF6::FileMetaData
    Qt6::Core
)

set_target_properties(kfilemetadata_postscriptdscextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
install(
    TARGETS kfilemetadata_postscriptdscextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata
)

#
# ODF
#
set_package_properties(KF6Archive PROPERTIES PURPOSE "- Required for ODF extractor")
if(KF6Archive_FOUND)
    add_library(kfilemetadata_odfextractor MODULE odfextractor.cpp)

    target_link_libraries(kfilemetadata_odfextractor
        KF6::FileMetaData
        Qt6::Core
        Qt6::Xml
        KF6::Archive
    )

    set_target_properties(kfilemetadata_odfextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_odfextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
endif()

#
# Office 2007
#
set_package_properties(KF6Archive PROPERTIES PURPOSE "- Required for Office 2007 extractor")
if(KF6Archive_FOUND)
    add_library(kfilemetadata_office2007extractor MODULE
        office2007extractor.cpp
        dublincoreextractor.cpp
        ${debug_SRCS}
    )

    target_link_libraries(kfilemetadata_office2007extractor
        KF6::FileMetaData
        Qt6::Core
        Qt6::Xml
        KF6::Archive
    )

    set_target_properties(kfilemetadata_office2007extractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_office2007extractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
endif()

#
# Office (binary formats)
#

add_library(kfilemetadata_officeextractor MODULE)
target_sources(kfilemetadata_officeextractor
    PRIVATE
        officeextractor.cpp
        ${debug_SRCS}
)

target_link_libraries(kfilemetadata_officeextractor
    KF6::FileMetaData
)

set_target_properties(kfilemetadata_officeextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
install(
TARGETS kfilemetadata_officeextractor
DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

#
# Mobipocket
#
if (QMobipocket6_FOUND)
    add_library(kfilemetadata_mobiextractor MODULE mobiextractor.cpp)
    target_link_libraries(kfilemetadata_mobiextractor
        KF6::FileMetaData
        QMobipocket6
    )

    set_target_properties(kfilemetadata_mobiextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_mobiextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)

endif()

set_package_properties(Qt6Gui PROPERTIES PURPOSE "- Required for AppImage extractor")
if(libappimage_FOUND AND KF6Config_FOUND AND Qt6Gui_FOUND)
    add_library(kfilemetadata_appimageextractor MODULE appimageextractor.cpp )
    target_link_libraries( kfilemetadata_appimageextractor
        KF6::FileMetaData
        KF6::ConfigCore
        Qt6::Xml
        Qt6::Gui
        libappimage
    )

    set_target_properties(kfilemetadata_appimageextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_appimageextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
endif()

set_package_properties(Qt6Gui PROPERTIES PURPOSE "- Required for Qt based PNG extractor")
if(Qt6Gui_FOUND)
    add_library(kfilemetadata_pngextractor MODULE pngextractor.cpp )
    target_link_libraries(kfilemetadata_pngextractor
        KF6::FileMetaData
        Qt6::Gui
    )

    set_target_properties(kfilemetadata_pngextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_pngextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
endif()

#
# FictionBook2
#
set_package_properties(KF6Archive PROPERTIES PURPOSE "- Required for FictionBook2 extractor")
if(KF6Archive_FOUND)
    add_library(kfilemetadata_fb2extractor MODULE fb2extractor.cpp )
    target_link_libraries(kfilemetadata_fb2extractor
        KF6::Archive
        KF6::FileMetaData
        Qt6::Core
    )

    set_target_properties(kfilemetadata_fb2extractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_fb2extractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
endif()

#
# Krita
#
set_package_properties(KF6Archive PROPERTIES PURPOSE "- Required for Krita extractor")
if(KF6Archive_FOUND)
    add_library(kfilemetadata_krita MODULE)
    target_sources(kfilemetadata_krita
        PRIVATE
            kritaextractor.cpp
    )

    target_link_libraries(kfilemetadata_krita
        KF6::FileMetaData
        KF6::Archive
    )

    set_target_properties(kfilemetadata_krita PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
    install(
    TARGETS kfilemetadata_krita
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
endif()


