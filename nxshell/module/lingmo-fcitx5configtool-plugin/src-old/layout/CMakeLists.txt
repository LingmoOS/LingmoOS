add_library(X11Import UNKNOWN IMPORTED)
set_target_properties(X11Import PROPERTIES
    IMPORTED_LOCATION "${X11_X11_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${X11_X11_INCLUDE_PATH}")

add_library(X11XkblibImport INTERFACE IMPORTED)
set_target_properties(X11XkblibImport PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${X11_Xkblib_INCLUDE_PATH}")

add_library(layoutlib STATIC keyboardlayoutwidget.cpp)
set_target_properties(layoutlib PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(layoutlib PUBLIC Fcitx5::Utils Qt5::Widgets PRIVATE Qt5::X11Extras X11Import X11XkblibImport PkgConfig::XkbFile)
target_include_directories(layoutlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

set(kbd_layout_viewer_SOURCES
  main.cpp
  )

add_executable(kbd-layout-viewer5 ${kbd_layout_viewer_SOURCES})

target_link_libraries(kbd-layout-viewer5 Fcitx5::Utils Qt5::Widgets Qt5::X11Extras layoutlib)

install(TARGETS kbd-layout-viewer5 DESTINATION ${KDE_INSTALL_BINDIR})
fcitx5_translate_desktop_file(
  kbd-layout-viewer5.desktop.in
  kbd-layout-viewer5.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/fcitx5-configtool)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kbd-layout-viewer5.desktop"
  DESTINATION ${KDE_INSTALL_APPDIR})
