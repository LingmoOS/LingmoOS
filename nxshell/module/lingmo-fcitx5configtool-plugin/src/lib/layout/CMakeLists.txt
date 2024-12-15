add_library(X11Import UNKNOWN IMPORTED)
set_target_properties(X11Import PROPERTIES
    IMPORTED_LOCATION "${X11_X11_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${X11_X11_INCLUDE_PATH}")

add_library(X11XkblibImport INTERFACE IMPORTED)
set_target_properties(X11XkblibImport PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${X11_Xkblib_INCLUDE_PATH}")

set(PLUGIN_NAME "layoutlib${QT_MAJOR_VERSION}")
add_library(${PLUGIN_NAME} STATIC keyboardlayoutwidget.cpp)
set_target_properties(${PLUGIN_NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(${PLUGIN_NAME} PUBLIC Fcitx5::Utils Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets PRIVATE X11Import X11XkblibImport PkgConfig::XkbFile)
target_include_directories(${PLUGIN_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

set(kbd_layout_viewer_SOURCES
  main.cpp
  )

set(EXECUTABLE_NAME "kbd-layout-viewer${QT_MAJOR_VERSION}")
add_executable(${EXECUTABLE_NAME} ${kbd_layout_viewer_SOURCES})

target_link_libraries(${EXECUTABLE_NAME} Fcitx5::Utils Qt${QT_MAJOR_VERSION}::Widgets layoutlib${QT_MAJOR_VERSION})

install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${KDE_INSTALL_BINDIR})
fcitx5_translate_desktop_file(
  kbd-layout-viewer6.desktop.in
  kbd-layout-viewer${QT_MAJOR_VERSION}.desktop PO_DIRECTORY ${PROJECT_SOURCE_DIR}/po/fcitx5-configtool)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kbd-layout-viewer${QT_MAJOR_VERSION}.desktop"
  DESTINATION ${KDE_INSTALL_APPDIR})
