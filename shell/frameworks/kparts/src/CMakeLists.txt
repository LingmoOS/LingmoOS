add_library(KF6Parts)
add_library(KF6::Parts ALIAS KF6Parts)

set_target_properties(KF6Parts PROPERTIES
    VERSION     ${KPARTS_VERSION}
    SOVERSION   ${KPARTS_SOVERSION}
    EXPORT_NAME Parts
)

target_sources(KF6Parts PRIVATE
    partbase.cpp
    part.cpp
    partloader.cpp
    openurlarguments.cpp
    readonlypart.cpp
    readwritepart.cpp
    partmanager.cpp
    mainwindow.cpp
    guiactivateevent.cpp
    partactivateevent.cpp
    navigationextension.cpp
    openurlevent.cpp
    statusbarextension.cpp
    fileinfoextension.cpp
    listingfilterextension.cpp
    listingnotificationextension.cpp
)

include(ECMGenerateHeaders)
ecm_generate_headers(KParts_CamelCase_HEADERS
    HEADER_NAMES
        FileInfoExtension
        GUIActivateEvent
        ListingFilterExtension
        ListingNotificationExtension
        MainWindow
        NavigationExtension
        OpenUrlArguments
        OpenUrlEvent
        Part
        PartActivateEvent
        PartBase
        PartLoader
        PartManager
        ReadOnlyPart
        ReadWritePart
        StatusBarExtension
    REQUIRED_HEADERS KParts_HEADERS
    PREFIX KParts
)
install(FILES ${KParts_CamelCase_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KParts/KParts
        COMPONENT Devel)

ecm_qt_declare_logging_category(KF6Parts
    HEADER kparts_logging.h
    IDENTIFIER KPARTSLOG
    CATEGORY_NAME kf.parts
    DESCRIPTION "KParts"
    EXPORT KPARTSLOG
)

ecm_generate_export_header(KF6Parts
    EXPORT_FILE_NAME ${KParts_BINARY_DIR}/kparts/kparts_export.h
    BASE_NAME KParts
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Parts
    PUBLIC "$<BUILD_INTERFACE:${KParts_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KParts>"
)

target_link_libraries(KF6Parts
    PUBLIC
        KF6::KIOCore # KFileItem used in FileInfoExtension API
        KF6::XmlGui # essential to the technology
    PRIVATE
        KF6::Service
        KF6::I18n
        KF6::JobWidgets
        KF6::KIOWidgets
)

install(TARGETS KF6Parts EXPORT KF6PartsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${KParts_BINARY_DIR}/kparts/kparts_export.h
  ${KParts_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KParts/kparts COMPONENT Devel
)

install(FILES kde_terminal_interface.h DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KParts )

ecm_qt_install_logging_categories(
    EXPORT KPARTSLOG
    FILE kparts.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Parts_QCH
        NAME KParts
        BASE_NAME KF6Parts
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KParts_HEADERS}
            kde_terminal_interface.h
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            KF6KIO_QCH
            KF6XmlGui_QCH
        INCLUDE_DIRS
            ${CMAKE_BINARY_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KPARTS_EXPORT
            KPARTS_DEPRECATED
            KPARTS_DEPRECATED_EXPORT
            "KPARTS_DEPRECATED_VERSION(x, y, t)"
            "KPARTS_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KPARTS_DEPRECATED_VERSION(x, y, t)"
            "KPARTS_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

