

configure_file(config-kwindowsystem.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h )

ecm_create_qm_loader(kwindowsystem_QM_LOADER kwindowsystem5_qt)

set(kwindowsystem_SRCS
    kkeyserver.cpp
    kstartupinfo.cpp
    kusertimestamp.cpp
    kwindoweffects.cpp
    kwindoweffects_dummy.cpp
    kwindowinfo.cpp
    kwindowsystem.cpp
    platforms/wayland/kwindowsystem.cpp
    pluginwrapper.cpp
    kwindowsystemplugininterface.cpp
    ${kwindowsystem_QM_LOADER}
    )
ecm_qt_declare_logging_category(kwindowsystem_SRCS HEADER kwindowsystem_debug.h IDENTIFIER LOG_KWINDOWSYSTEM CATEGORY_NAME org.kde.kwindowsystem DEFAULT_SEVERITY Warning)
ecm_qt_declare_logging_category(kwindowsystem_SRCS HEADER kwindowsystem_xcb_debug.h IDENTIFIER LOG_KKEYSERVER_X11 CATEGORY_NAME org.kde.kwindowsystem.keyserver.x11 DEFAULT_SEVERITY Warning)

if (KWINDOWSYSTEM_HAVE_X11)
  set(kwindowsystem_SRCS ${kwindowsystem_SRCS}
    platforms/xcb/kselectionowner.cpp
    platforms/xcb/kselectionwatcher.cpp
    platforms/xcb/kxerrorhandler.cpp
    platforms/xcb/kxutils.cpp
  )
endif()

set(platformLinkLibraries)

if (KWINDOWSYSTEM_HAVE_X11)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platforms/xcb)
   if(NOT X11_Xrender_LIB)
      message(FATAL_ERROR "The XRender library could not be found. Please install the development package for it.")
   endif()
   if(NOT X11_Xfixes_LIB)
      message(FATAL_ERROR "The XFixes library could not be found. Please install the development package for it.")
   endif()
   set(platformLinkLibraries Qt5::X11Extras ${X11_LIBRARIES} ${X11_Xfixes_LIB} ${X11_Xrender_LIB} ${XCB_XCB_LIBRARY} ${XCB_KEYSYMS_LIBRARY})
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} platforms/xcb/kkeyserver.cpp
                                                platforms/xcb/kxmessages.cpp
                                                platforms/xcb/netwm.cpp )
endif ()

if (APPLE)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platforms/osx)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} platforms/osx/kkeyserver.cpp
#                                                 kwindowsystem_mac.cpp
# FIXME: adjust kwindowinfo_mac to inherit from KWindowInfoPrivate
#                                                 kwindowinfo_mac.cpp
)
  set(platformLinkLibraries "-framework CoreFoundation -framework Carbon")
endif ()

if (WIN32)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} platforms/windows/kkeyserver.cpp
#                                                 kwindowsystem_win.cpp
# FIXME: adjust kwindowinfo_win to inherit from KWindowInfoPrivate
#                                                 kwindowinfo_win.cpp
)
   set(platformLinkLibraries
      Qt5::WinExtras # QtWin::fromHICON(), QtWin::toHICON()
      ws2_32 # gethostname()
   )
endif ()

add_library(KF5WindowSystem ${kwindowsystem_SRCS})
generate_export_header(KF5WindowSystem BASE_NAME KWindowSystem)
add_library(KF5::WindowSystem ALIAS KF5WindowSystem)

target_include_directories(KF5WindowSystem INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KWindowSystem>")

target_link_libraries(KF5WindowSystem
    PUBLIC Qt5::Widgets
    PRIVATE ${platformLinkLibraries}
)

if(KWINDOWSYSTEM_HAVE_X11)
    # we install kkeyserver_x11.h which needs the X11 headers available
    # if we don't add the include path here code that includes kkeyserver.h will fail
    # to compile unless X11 is installed in /usr/include
    target_include_directories(KF5WindowSystem PUBLIC ${XCB_XCB_INCLUDE_DIR} ${X11_X11_INCLUDE_PATH})
endif()

set_target_properties(KF5WindowSystem PROPERTIES VERSION   ${KWINDOWSYSTEM_VERSION_STRING}
                                                 SOVERSION ${KWINDOWSYSTEM_SOVERSION}
                                                 EXPORT_NAME WindowSystem
)

ecm_generate_headers(KWindowSystem_HEADERS
  HEADER_NAMES
  KKeyServer
  KStartupInfo
  KUserTimestamp
  KWindowEffects
  KWindowInfo
  KWindowSystem

  REQUIRED_HEADERS KWindowSystem_HEADERS
)

install(TARGETS KF5WindowSystem EXPORT KF5WindowSystemTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  # FIXME: It seems odd to install this.
  ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_export.h
  ${KWindowSystem_HEADERS}
  netwm_def.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KWindowSystem COMPONENT Devel
)

install(
    FILES
        kwindoweffects_p.h
        kwindowinfo_p.h
        kwindowsystem_p.h
        kwindowsystemplugininterface_p.h
    DESTINATION
        ${KDE_INSTALL_INCLUDEDIR_KF5}/KWindowSystem/private
    COMPONENT
        Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5WindowSystem_QCH
        NAME KWindowSystem
        BASE_NAME KF5WindowSystem
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCE_DIRS
            # using dir for now, to cover any platform dependant code
            # TODO: should only use public headers, to cover only public API
            ${CMAKE_CURRENT_SOURCE_DIR}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Widgets_QCH
        BLANK_MACROS
            KWINDOWSYSTEM_EXPORT
            KWINDOWSYSTEM_DEPRECATED
            KWINDOWSYSTEM_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

add_subdirectory(platforms)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KWindowSystem LIB_NAME KF5WindowSystem DEPS "gui" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KWindowSystem)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
