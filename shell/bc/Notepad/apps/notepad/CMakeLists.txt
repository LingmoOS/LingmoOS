# Notepad application

find_package(KF5DBusAddons ${KF5_DEP_VERSION} REQUIRED)

add_executable(notepad main.cpp  data/notepad.qrc)
target_link_libraries(notepad PRIVATE kateprivate PUBLIC KF5::DBusAddons)

# ensure right translations domain, we use one kate.po(t) for lib + apps
target_compile_definitions(notepad PRIVATE TRANSLATION_DOMAIN="kate")

# See https://cmake.org/cmake/help/v3.15/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
if(APPLE)
  set_property(
    TARGET notepad
    PROPERTY MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/data/MacOSXBundleInfo.plist.in
  )

  # These are substituted by CMake into plist.in.
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.lingmo.Notepad")
  set(MACOSX_BUNDLE_BUNDLE_NAME "Notepad")
  set(MACOSX_BUNDLE_DISPLAY_NAME "Notepad")
  set(MACOSX_BUNDLE_INFO_STRING "Notepad - Text Editor")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "Notepad ${RELEASE_SERVICE_VERSION}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${RELEASE_SERVICE_VERSION}")
  set(MACOSX_BUNDLE_COPYRIGHT "2000-2022 The KWrite Authors")
endif()

install(TARGETS notepad ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.lingmo.notepad.desktop
  DESTINATION ${KDE_INSTALL_APPDIR}
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.lingmo.notepad.appdata.xml
  DESTINATION ${KDE_INSTALL_METAINFODIR}
)
