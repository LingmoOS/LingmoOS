add_library(KF5ConfigGui)
add_library(KF5::ConfigGui ALIAS KF5ConfigGui)

ecm_create_qm_loader(KF5ConfigGui kconfig5_qt)

target_sources(KF5ConfigGui PRIVATE
    kconfiggui.cpp
    kconfiggroupgui.cpp
    kconfigloader.cpp
    kconfigskeleton.cpp
    kstandardshortcut.cpp
    kstandardshortcutwatcher.cpp
    kwindowconfig.cpp
    kwindowstatesaver.cpp
)

ecm_qt_declare_logging_category(KF5ConfigGui
    HEADER kconfig_gui_log_settings.h
    IDENTIFIER KCONFIG_GUI_LOG
    CATEGORY_NAME kf.config.gui
    DESCRIPTION "KConfig Gui"
    EXPORT KCONFIG
)

ecm_generate_export_header(KF5ConfigGui
    BASE_NAME KConfigGui
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.11 5.39 5.71 5.82
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5ConfigGui
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfig;${KDE_INSTALL_INCLUDEDIR_KF}/KConfigGui>")

target_link_libraries(KF5ConfigGui PUBLIC Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Xml KF5::ConfigCore)

set_target_properties(KF5ConfigGui PROPERTIES VERSION   ${KCONFIG_VERSION}
                                              SOVERSION ${KCONFIG_SOVERSION}
                                              EXPORT_NAME ConfigGui
)

ecm_generate_headers(KConfigGui_HEADERS
  HEADER_NAMES
  KConfigGui
  KConfigLoader
  KConfigSkeleton
  KStandardShortcut
  KStandardShortcutWatcher
  KWindowConfig
  KWindowStateSaver

   REQUIRED_HEADERS KConfigGui_HEADERS
)

if (NOT BUILD_SHARED_LIBS)
    target_sources(KF5ConfigGui PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kconfigguistaticinitializer.cpp>
        $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfigGui/kconfigguistaticinitializer.cpp>
    )
endif()

install(TARGETS KF5ConfigGui EXPORT KF5ConfigTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfiggui_export.h
  ${KConfigGui_HEADERS}
  kconfigguistaticinitializer.cpp
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigGui COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KConfigGui_APIDOX_SRCS ${KConfigGui_HEADERS} PARENT_SCOPE)
set(KConfigGui_APIDOX_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KConfigGui LIB_NAME KF5ConfigGui DEPS "gui xml KConfigCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigGui)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
