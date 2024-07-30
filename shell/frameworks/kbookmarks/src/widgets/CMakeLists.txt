add_library(KF6BookmarksWidgets)
add_library(KF6::BookmarksWidgets ALIAS KF6BookmarksWidgets)

set_target_properties(KF6BookmarksWidgets PROPERTIES
    VERSION     ${KBOOKMARKS_VERSION}
    SOVERSION   ${KBOOKMARKS_SOVERSION}
    EXPORT_NAME BookmarksWidgets
)

ecm_create_qm_loader(KF6BookmarksWidgets kbookmarkswidgets6_qt)

target_sources(KF6BookmarksWidgets PRIVATE
    kbookmarkactionmenu.cpp
    kbookmarkactionmenu.h
    kbookmarkcontextmenu.cpp
    kbookmarkcontextmenu.h
    kbookmarkdialog.cpp
    kbookmarkdialog.h
    kbookmarkdialog_p.h
    kbookmarkmenu.cpp
    kbookmarkmenu.h
    kbookmarkmenu_p.h
)

ecm_qt_declare_logging_category(KF6BookmarksWidgets
    HEADER kbookmarkswidgets_debug.h
    IDENTIFIER KBOOKMARKSWIDGETS_LOG
    CATEGORY_NAME kf.bookmarks.widgets
    DESCRIPTION "KBookmarks Widgets"
    EXPORT KBOOKMARKSWIDGETS
)

ecm_generate_export_header(KF6BookmarksWidgets
    BASE_NAME KBookmarksWidgets
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER kbookmarks_version.h
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6BookmarksWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KBookmarksWidgets>")

target_link_libraries(KF6BookmarksWidgets PUBLIC Qt6::Widgets KF6::WidgetsAddons)
target_link_libraries(KF6BookmarksWidgets PRIVATE
    KF6::ConfigWidgets # for KStandardAction
    KF6::Bookmarks
)

ecm_generate_headers(KBookmarksWidgets_HEADERS
  HEADER_NAMES
  KBookmarkActionMenu
  KBookmarkContextMenu
  KBookmarkMenu
  KBookmarkDialog

  REQUIRED_HEADERS KBookmarksWidgets_HEADERS
)

install(TARGETS KF6BookmarksWidgets EXPORT KF6BookmarksTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kbookmarkswidgets_export.h
  ${KBookmarksWidgets_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KBookmarksWidgets COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KBOOKMARKSWIDGETS
    FILE kbookmarkswidgets.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6BookmarksWidgets_QCH
        NAME KBookmarksWidgets
        BASE_NAME KF6BookmarksWidgets
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KBookmarksWidgets_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Widgets_QCH
            Qt6Xml_QCH
            KF6WidgetsAddons_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KBOOKMARKSWIDGETS_EXPORT
            KBOOKMARKSWIDGETS_DEPRECATED
            KBOOKMARKSWIDGETS_DEPRECATED_EXPORT
            "KBOOKMARKS_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
