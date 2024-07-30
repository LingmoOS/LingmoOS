include_directories( ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} )

include(uriencode)
kdoctools_encode_uri(DocBookXML4_DTD_DIR)
set (_custom_dtd_kdex "customization/dtd/kdedbx45.dtd")
configure_file(${_custom_dtd_kdex}.cmake ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex} )
# WARNING: this is due to severe limitations on the windows platform to keep the dtd relocatable
# see the .xsl.cmake files for a more verbose explanation below
if(NOT WIN32)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}
        DESTINATION ${KDE_INSTALL_DATADIR_KF}/kdoctools/customization/dtd )
else()
    get_filename_component(NORMALIZED_DTD_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_DATADIR_KF}/kdoctools/customization/dtd" ABSOLUTE)
    # file(RELATIVE_PATH ...) returns wrong values for paths containing /../ (it doesn't normalize the path)
    # To work around this we make sure the variable passed has all /../ elements
    # removed using get_filename_component(... ABSOLUTE)
    install(CODE
        "file(RELATIVE_PATH DocBookXML4_DTD_DIR \"${NORMALIZED_DTD_INSTALL_PATH}\" \"${DocBookXML4_DTD_DIR}\")
        set(PERL_EXECUTABLE \"${PERL_EXECUTABLE}\")
        include(\"${CMAKE_SOURCE_DIR}/cmake/uriencode.cmake\")
        set(CMAKE_MODULE_PATH \"${CMAKE_MODULE_PATH}\")
        kdoctools_encode_uri(DocBookXML4_DTD_DIR)
        configure_file(\"${CMAKE_CURRENT_SOURCE_DIR}/${_custom_dtd_kdex}.cmake\" \"${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}.install\")"
    )
    get_filename_component(_custom_dtd_kdex_name ${_custom_dtd_kdex} NAME)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_custom_dtd_kdex}.install" DESTINATION ${KDE_INSTALL_DATADIR_KF}/kdoctools/customization/dtd RENAME ${_custom_dtd_kdex_name})
endif()

ecm_qt_declare_logging_category(kdoctoolslog_core_SRCS
    HEADER loggingcategory.h
    IDENTIFIER KDocToolsLog
    CATEGORY_NAME kf.doctools.core
    DEFAULT_SEVERITY Warning
)

# needed by KIO, need to export it
add_library(KF6DocTools SHARED xslt.cpp xslt_kde.cpp ${kdoctoolslog_core_SRCS})
add_library(KF6::DocTools ALIAS KF6DocTools)

set_target_properties(KF6DocTools PROPERTIES
    VERSION     ${KDOCTOOLS_VERSION}
    SOVERSION   ${KDOCTOOLS_SOVERSION}
    EXPORT_NAME DocTools
    POSITION_INDEPENDENT_CODE TRUE
)

ecm_generate_export_header(KF6DocTools
    BASE_NAME KDocTools
    EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/kdoctools_export.h"
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
)

if (NOT MEINPROC_NO_KARCHIVE)
    set (_private_link_karchive "KF6::Archive")
else()
    set (_private_link_karchive "")
endif()

target_link_libraries(KF6DocTools
    PUBLIC
        Qt6::Core
    PRIVATE
        ${LIBXML2_LIBRARIES}
        ${LIBXSLT_LIBRARIES}
        ${LIBXSLT_EXSLT_LIBRARIES}
        ${_private_link_karchive}
)

target_include_directories(KF6DocTools INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KDocTools>")

set(KDocTools_HEADERS
    docbookxslt.h
)

install(FILES
    ${KDocTools_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kdoctools_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDocTools
    COMPONENT Devel
)

install(TARGETS KF6DocTools EXPORT KF6DocToolsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        KF6DocTools_QCH
        NAME KDocTools
        BASE_NAME KF6DocTools
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KDocTools_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KDOCTOOLS_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

########### next target ###############

set_source_files_properties(xslt.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

if(MEINPROC_NO_KARCHIVE)
    add_definitions(-DMEINPROC_NO_KARCHIVE) #we don't have saveToCache when compiling without KArchive, which is used in xslt_kde.cpp
else ()
    set(meinproc_additional_SRCS xslt_kde.cpp)
    set(meinproc_additional_LIBS KF6::Archive)
endif()

add_executable(meinproc6 meinproc.cpp meinproc_common.cpp xslt.cpp ${meinproc_additional_SRCS} ${kdoctoolslog_core_SRCS})
ecm_mark_nongui_executable(meinproc6)
target_link_libraries(meinproc6
    PRIVATE
        Qt6::Core
        ${LIBXML2_LIBRARIES}
        ${LIBXSLT_LIBRARIES}
        ${LIBXSLT_EXSLT_LIBRARIES}
        ${meinproc_additional_LIBS}
)

target_compile_definitions(meinproc6 PRIVATE -DKDOCTOOLS_EXPORT=)
install(TARGETS meinproc6 EXPORT KF6DocToolsToolsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(CMAKE_CROSSCOMPILING AND MEINPROC6_EXECUTABLE)
    add_executable(KF6::meinproc6 IMPORTED GLOBAL)
    set_target_properties(KF6::meinproc6 PROPERTIES IMPORTED_LOCATION ${MEINPROC6_EXECUTABLE})
else()
    add_executable(KF6::meinproc6 ALIAS meinproc6)
endif()

########### customize includes for xslt ###############
# WARNING: this has severe limitations on the windows platform since the .xsl
# & docbook files need to be relocatable. We cannot just replace the paths in
# the .xsl with relative paths because these files are not installed while
# building kdoctools. Instead, we will reconfigure those files at install time
# with relative paths.

file( GLOB _kdeincludexslt_ALL "customization/kde-include*.xsl.cmake" )
foreach( _kdeincludexslt ${_kdeincludexslt_ALL} )
    get_filename_component( _kdeincludexslt_name ${_kdeincludexslt} NAME )
    string( REGEX REPLACE ".cmake$" "" _kdeincludexslt_out "${_kdeincludexslt_name}" )
    set( _kdeincludexslt_binarypath "${CMAKE_CURRENT_BINARY_DIR}/customization/${_kdeincludexslt_out}" )
    configure_file( ${_kdeincludexslt} ${_kdeincludexslt_binarypath} )
    if(NOT WIN32)
        install(FILES ${_kdeincludexslt_binarypath}
            DESTINATION ${KDE_INSTALL_DATADIR_KF}/kdoctools/customization )
    else()
        install(CODE
            "FILE(RELATIVE_PATH DOCBOOKXSL_DIR \${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_DATADIR_KF}/kdoctools/customization ${DOCBOOKXSL_DIR})
            CONFIGURE_FILE( \"${_kdeincludexslt}\" \"${_kdeincludexslt_binarypath}.install\" )")
            install(FILES "${_kdeincludexslt_binarypath}.install" DESTINATION ${KDE_INSTALL_DATADIR_KF}/kdoctools/customization RENAME ${_kdeincludexslt_out})
    endif()
endforeach( _kdeincludexslt ${_kdeincludexslt_ALL} )


########### install files ###############

macro(INSTALL_CUSTOMIZE_DIR _expr _directory)
    FILE(GLOB _file_CUST "${_expr}" )
    foreach(_curentfile ${_file_CUST})
        install(FILES ${_curentfile} DESTINATION ${KDE_INSTALL_DATADIR_KF}/kdoctools/${_directory} )
    endforeach(_curentfile ${_file_CUST})
endmacro(INSTALL_CUSTOMIZE_DIR _expr _directory)

macro(INSTALL_CUSTOMIZE_L_DIR _expr _directory)
    FILE(GLOB _file_CUST "${_expr}" )
    foreach(_curentfile ${_file_CUST})
        # copy the file also under CMAKE_CURRENT_BINARY_DIR - those
        # files are needed to generate the documentation without installing
        # kdoctools (i.e. when kdoctools itself compiled).
        get_filename_component(_currentfile_name ${_curentfile} NAME)
        set(_currentfile_binary_path ${CMAKE_CURRENT_BINARY_DIR}/${_directory}/${_currentfile_name})
        configure_file(${_curentfile} ${_currentfile_binary_path} COPYONLY)

        install(FILES ${_currentfile_binary_path}
            DESTINATION ${KDE_INSTALL_DATADIR_KF}/kdoctools/${_directory} )
    endforeach(_curentfile ${_file_CUST})
endmacro(INSTALL_CUSTOMIZE_L_DIR _expr _directory)



set(expr "customization/*.xsl")
install_customize_l_dir( ${expr} customization/)

set(expr "customization/README")
install_customize_l_dir( ${expr} customization/)

set(expr "customization/catalog.xml")
install_customize_l_dir( ${expr} customization/)

set(customizedir ${KDOCTOOLS_LANGUAGES_LIST} dtd entities xsl)

foreach (_currentcustomizedir ${customizedir})
    set(expr "customization/${_currentcustomizedir}/README")
    install_customize_l_dir( ${expr} customization/${_currentcustomizedir})
    set(expr "customization/${_currentcustomizedir}/*.entities")
    install_customize_l_dir( ${expr} customization/${_currentcustomizedir})
    set(expr "customization/${_currentcustomizedir}/*.xsl")
    install_customize_l_dir( ${expr} customization/${_currentcustomizedir})
    set(expr "customization/${_currentcustomizedir}/entities/*.docbook")
    install_customize_l_dir( ${expr} customization/${_currentcustomizedir}/entities/ )
    set(expr "customization/${_currentcustomizedir}/entities/*.entities")
    install_customize_l_dir( ${expr} customization/${_currentcustomizedir}/entities/ )
    set(expr "customization/${_currentcustomizedir}/*.xml")
    install_customize_l_dir(${expr} customization/${_currentcustomizedir} )
    set(expr "customization/${_currentcustomizedir}/*.dtd")
    install_customize_l_dir(${expr} customization/${_currentcustomizedir}/)
    set(expr "customization/${_currentcustomizedir}/*.elements")
    install_customize_l_dir(${expr} customization/${_currentcustomizedir}/)
endforeach(_currentcustomizedir ${customizedir})

########### l10n xml helper ###############

set( docbookl10nhelper_SRCS docbookl10nhelper.cpp ${kdoctoolslog_core_SRCS})
add_executable( docbookl10nhelper ${docbookl10nhelper_SRCS} )
ecm_mark_nongui_executable( docbookl10nhelper )
target_link_libraries( docbookl10nhelper Qt6::Core )
if(INSTALL_INTERNAL_TOOLS)
    install(TARGETS docbookl10nhelper EXPORT KF6DocToolsToolsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

if(CMAKE_CROSSCOMPILING AND DOCBOOKL10NHELPER_EXECUTABLE)
    add_executable(KF6::docbookl10nhelper IMPORTED GLOBAL)
    set_target_properties(KF6::docbookl10nhelper PROPERTIES IMPORTED_LOCATION ${DOCBOOKL10NHELPER_EXECUTABLE})
else()
    add_executable(KF6::docbookl10nhelper ALIAS docbookl10nhelper)
endif()

add_custom_command( TARGET docbookl10nhelper POST_BUILD
    COMMAND $<TARGET_FILE:KF6::docbookl10nhelper>
    "${DOCBOOKXSL_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/customization/xsl"
    "${CMAKE_CURRENT_BINARY_DIR}/customization/xsl"
)

# all-l10n.xml is generated by docbookl10nhelper
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/customization/xsl/all-l10n.xml
    DESTINATION ${KDE_INSTALL_DATADIR_KF}/kdoctools/customization/xsl/ )

########### checkXML6 helper ###############

set( checkXML6_SRCS checkXML6.cpp ${kdoctoolslog_core_SRCS})
add_executable( checkXML6 ${checkXML6_SRCS} )
ecm_mark_nongui_executable( checkXML6 )
target_link_libraries( checkXML6 Qt6::Core )
install(TARGETS checkXML6 EXPORT KF6DocToolsToolsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(CMAKE_CROSSCOMPILING AND CHECKXML6_EXECUTABLE)
    add_executable(KF6::checkXML6 IMPORTED GLOBAL)
    set_target_properties(KF6::checkXML6 PROPERTIES IMPORTED_LOCATION ${CHECKXML6_EXECUTABLE})
else()
    add_executable(KF6::checkXML6 ALIAS checkXML6)
endif()
