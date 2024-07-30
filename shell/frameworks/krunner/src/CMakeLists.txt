# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF6Runner SHARED)
add_library(KF6::Runner ALIAS KF6Runner)

set_target_properties(KF6Runner PROPERTIES
    VERSION     ${KRUNNER_VERSION}
    SOVERSION   ${KRUNNER_SOVERSION}
    EXPORT_NAME Runner
)

target_sources(KF6Runner PRIVATE
    abstractrunner.cpp
    abstractrunner.h
    abstractrunnertest.h
    dbusrunner.cpp
    dbusrunner_p.h
    dbusutils_p.h
    querymatch.cpp
    querymatch.h
    runnercontext.cpp
    runnercontext.h
    runnermanager.cpp
    runnermanager.h
    runnersyntax.cpp
    runnersyntax.h
    action.h
    action.cpp

    model/runnerresultsmodel.cpp
    model/runnerresultsmodel_p.h
    model/resultsmodel.cpp
    model/resultsmodel.h
)
ecm_qt_declare_logging_category(KF6Runner
    HEADER krunner_debug.h
    IDENTIFIER KRUNNER
    CATEGORY_NAME kf.runner
    OLD_CATEGORY_NAMES org.kde.krunner
    DESCRIPTION "KRunner"
    EXPORT KRUNNER
)
set_property(SOURCE "data/org.kde.krunner1.xml" PROPERTY INCLUDE dbusutils_p.h)

ecm_generate_export_header(KF6Runner
    BASE_NAME KRunner
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

set(KRunner_BUILD_INCLUDE_DIRS ${KRunner_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(KF6Runner INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KRunner>")
target_include_directories(KF6Runner PUBLIC "$<BUILD_INTERFACE:${KRunner_BUILD_INCLUDE_DIRS}>")


target_link_libraries(KF6Runner
    PUBLIC
        Qt6::Core
        KF6::CoreAddons # KPluginFactory
    PRIVATE
        Qt6::DBus
        Qt6::Gui
        KF6::ConfigCore
        KF6::I18n
        KF6::ItemModels
)
ecm_generate_headers(KRunner_CamelCase_HEADERS
  HEADER_NAMES
    AbstractRunner
    Action
    RunnerContext
    RunnerManager
    RunnerSyntax
    QueryMatch
    AbstractRunnerTest

    PREFIX KRunner
    REQUIRED_HEADERS KRunner_HEADERS
)
ecm_generate_headers(KRunner_CamelCase_HEADERS
  HEADER_NAMES ResultsModel
    PREFIX KRunner
    REQUIRED_HEADERS KRunner_HEADERS
    RELATIVE model
)

# Install files

install(TARGETS KF6Runner
        EXPORT KF6RunnerTargets
        ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${KRunner_CamelCase_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KRunner/KRunner
        COMPONENT Devel)

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/krunner_export.h
            ${KRunner_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KRunner/krunner
        COMPONENT Devel)

ecm_qt_install_logging_categories(
    EXPORT KRUNNER
    FILE krunner.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)


if(BUILD_QCH)
    ecm_add_qch(
        KF6Runner_QCH
        NAME KRunner
        BASE_NAME KF6Runner
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KRunner_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            KF6CoreAddons_QCH
        INCLUDE_DIRS
            ${KRunner_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            KRUNNER_EXPORT
            KRUNNER_DEPRECATED
            KRUNNER_DEPRECATED_EXPORT
            "KRUNNER_DEPRECATED_VERSION(x, y, t)"
            "KRUNNER_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KRUNNER_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

install(FILES
   "data/org.kde.krunner1.xml"
   DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
   RENAME kf6_org.kde.krunner1.xml)
