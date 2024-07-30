set(BALOO_LIB_SRCS
    term.cpp
    query.cpp
    queryrunnable.cpp
    resultiterator.cpp
    advancedqueryparser.cpp

    file.cpp
    filemonitor.cpp
    taglistjob.cpp

    indexerconfig.cpp
    ../file/fileindexerconfig.cpp
    ../file/storagedevices.cpp
    ../file/regexpcache.cpp
    ../file/fileexcludefilters.cpp
    ../file/propertydata.cpp

    searchstore.cpp
)
ecm_qt_declare_logging_category(BALOO_LIB_SRCS
    HEADER baloodebug.h
    IDENTIFIER BALOO
    CATEGORY_NAME kf.baloo
    OLD_CATEGORY_NAMES org.kde.baloo
    DESCRIPTION "Baloo File debug"
    EXPORT BALOO
)

kconfig_add_kcfg_files(BALOO_LIB_SRCS baloosettings.kcfgc GENERATE_MOC)
add_library(KF6Baloo ${BALOO_LIB_SRCS})

add_library(KF6::Baloo ALIAS KF6Baloo)

target_link_libraries(KF6Baloo
    PUBLIC
    Qt6::Core
    KF6::CoreAddons
    KF6::FileMetaData

    PRIVATE
    KF6::ConfigCore
    Qt6::DBus
    KF6::Solid
    KF6::BalooEngine
    BalooDBusMainInterface
)

set_target_properties(KF6Baloo PROPERTIES
    VERSION ${BALOO_VERSION}
    SOVERSION ${BALOO_SOVERSION}
    EXPORT_NAME Baloo
)
target_include_directories(KF6Baloo INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/Baloo>")

ecm_generate_export_header(KF6Baloo
    EXPORT_FILE_NAME core_export.h
    BASE_NAME BALOO_CORE
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME Baloo
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

ecm_generate_headers(KF6Baloo_CamelCase_HEADERS
    HEADER_NAMES
    Query
    QueryRunnable
    ResultIterator

    File
    FileMonitor
    TagListJob
    IndexerConfig

    PREFIX baloo
    REQUIRED_HEADERS KF6Baloo_HEADERS
)

install(TARGETS KF6Baloo EXPORT KF6BalooTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/core_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/baloosettings.h
    ${KF6Baloo_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Baloo/baloo
    COMPONENT Devel
)

install(FILES
    ${KF6Baloo_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Baloo/Baloo
    COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Baloo_QCH
        NAME Baloo
        BASE_NAME KF6Baloo
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KF6Baloo_HEADERS}
            "${CMAKE_SOURCE_DIR}/docs/distributing.md"
            "${CMAKE_SOURCE_DIR}/docs/development/build-instructions.md"
            "${CMAKE_SOURCE_DIR}/docs/user/searching.md"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            KF6CoreAddons_QCH
            KF6FileMetaData_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            BALOO_CORE_EXPORT
            BALOO_CORE_DEPRECATED
            BALOO_CORE_DEPRECATED_EXPORT
            "BALOO_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KF6Baloo
        LIB_NAME KF6Baloo
        INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/Baloo
        DEPS Qt6Core
        DESCRIPTION "Baloo is a file indexing and searching framework for Linux"
        INSTALL
    )
endif ()

