include_directories(../)
include_directories(${POPPLER_INCLUDE_DIRS})
include_directories(${UCHARDET_INCLUDE_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})

if(AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND AND SWSCALE_FOUND)
    add_library(lingmofilemetadata_ffmpegextractor MODULE
            ffmpeg-extractor.cpp
            )
    target_include_directories(lingmofilemetadata_ffmpegextractor SYSTEM PRIVATE ${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIRS})
    target_link_libraries(lingmofilemetadata_ffmpegextractor
            lingmo-file-metadata
            ${AVCODEC_LIBRARIES}
            ${AVFORMAT_LIBRARIES}
            ${AVUTIL_LIBRARIES}
            ${SWSCALE_LIBRARIES}
            )
    set_target_properties(lingmofilemetadata_ffmpegextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
    install(
            TARGETS lingmofilemetadata_ffmpegextractor
            DESTINATION "${PLUGIN_INSTALL_DIR}")

endif()

#
#office (binary)
#

add_library(lingmofilemetadata_officeextractor MODULE office-extractor.cpp binary-parser.cpp)
target_link_libraries(lingmofilemetadata_officeextractor
        lingmo-file-metadata)
set_target_properties(lingmofilemetadata_officeextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_officeextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#office2007
#

add_library(lingmofilemetadata_office2007extractor MODULE office2007-extractor.cpp)
target_link_libraries(lingmofilemetadata_office2007extractor
        lingmo-file-metadata
        quazip5)
set_target_properties(lingmofilemetadata_office2007extractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_office2007extractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#text
#

add_library(lingmofilemetadata_textextractor MODULE text-extractor.cpp)
target_link_libraries(lingmofilemetadata_textextractor
        lingmo-file-metadata
        ${UCHARDET_LIBRARIES}
        )
set_target_properties(lingmofilemetadata_textextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_textextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#pdf
#

add_library(lingmofilemetadata_pdfextractor MODULE pdf-extractor.cpp)
target_link_libraries(lingmofilemetadata_pdfextractor
        lingmo-file-metadata
        ${POPPLER_LIBRARIES}
        )
set_target_properties(lingmofilemetadata_pdfextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_pdfextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#uof
#

add_library(lingmofilemetadata_uofextractor MODULE uof-extractor.cpp)
target_link_libraries(lingmofilemetadata_uofextractor
        lingmo-file-metadata
        quazip5)
set_target_properties(lingmofilemetadata_uofextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_uofextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#ofd
#

add_library(lingmofilemetadata_ofdextractor MODULE ofd-extractor.cpp)
target_link_libraries(lingmofilemetadata_ofdextractor
        lingmo-file-metadata
        quazip5)
set_target_properties(lingmofilemetadata_ofdextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_ofdextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#png
#
add_library(lingmofilemetadata_pngextractor MODULE png-extractor.cpp)
target_link_libraries(lingmofilemetadata_pngextractor
        lingmo-file-metadata)
set_target_properties(lingmofilemetadata_pngextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_pngextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#taglib
#

    add_library(lingmofilemetadata_taglibextractor MODULE taglib-extractor.cpp)
    target_link_libraries( lingmofilemetadata_taglibextractor
            lingmo-file-metadata
            ${TAGLIB_LIBRARIES}
    )

    set_target_properties(lingmofilemetadata_taglibextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
    install(
            TARGETS lingmofilemetadata_taglibextractor
            DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#png
#
add_library(lingmofilemetadata_imageextractor MODULE image-extractor.cpp)
target_link_libraries(lingmofilemetadata_imageextractor
        lingmo-file-metadata)
set_target_properties(lingmofilemetadata_imageextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lingmofilemetadata")
install(
        TARGETS lingmofilemetadata_imageextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")