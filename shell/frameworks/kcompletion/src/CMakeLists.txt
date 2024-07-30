add_library(KF6Completion)
add_library(KF6::Completion ALIAS KF6Completion)

set_target_properties(KF6Completion PROPERTIES
    VERSION     ${KCOMPLETION_VERSION}
    SOVERSION   ${KCOMPLETION_SOVERSION}
    EXPORT_NAME Completion
)

ecm_create_qm_loader(KF6Completion kcompletion6_qt)

target_sources(KF6Completion PRIVATE
    kcombobox.cpp
    kcombobox.h
    kcombobox_p.h
    kcompletionbase.cpp
    kcompletionbase.h
    kcompletionbox.cpp
    kcompletionbox.h
    kcompletion.cpp
    kcompletion.h
    kcompletionmatches.cpp
    kcompletionmatches.h
    kcompletion_p.h
    kemailvalidator.cpp
    kemailvalidator.h
    khistorycombobox.cpp
    khistorycombobox.h
    klineedit.cpp
    klineedit.h
    klineedit_p.h
    ksortablelist.h
    kzoneallocator.cpp
    kzoneallocator_p.h
)

ecm_qt_declare_logging_category(KF6Completion
    HEADER kcompletion_debug.h
    IDENTIFIER KCOMPLETION_LOG
    CATEGORY_NAME kf.completion
    DESCRIPTION "KCompletion"
    EXPORT KCOMPLETION
)

ecm_generate_export_header(KF6Completion
    BASE_NAME KCompletion
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Completion INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCompletion>")

target_link_libraries(KF6Completion PUBLIC Qt6::Widgets
                                    PRIVATE KF6::ConfigCore      # KConfigGroup, used in many places
                                            KF6::ConfigGui       # KStandardShortcut
                                            KF6::WidgetsAddons   # KCursor
                                            KF6::Codecs # KEmailAddress
)

ecm_generate_headers(KCompletion_HEADERS
  HEADER_NAMES
  KComboBox
  KCompletion
  KCompletionBase
  KCompletionBox
  KEmailValidator
  KLineEdit
  KHistoryComboBox
  KSortableList
  KCompletionMatches

  REQUIRED_HEADERS KCompletion_HEADERS
)

install(TARGETS KF6Completion EXPORT KF6CompletionTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_export.h
  ${KCompletion_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCompletion COMPONENT Devel
)

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF6Completion_QCH
        NAME KCompletion
        BASE_NAME KF6Completion
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCompletion_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KCOMPLETION_EXPORT
            KCOMPLETION_DEPRECATED
            "KCOMPLETION_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

ecm_qt_install_logging_categories(
    EXPORT KCOMPLETION
    FILE kcompletion.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
