set(TRAYICON_SOURCES
  TrayIcon.h
  TrayIcon.cpp
  IconProvider.h
  IconProvider.cpp
  qml.qrc
)

add_library(trayicon SHARED
  ${TRAYICON_SOURCES}
)

target_link_libraries(trayicon PRIVATE
  dimcore
  Qt6::Core
  Qt6::Gui
  Qt6::Quick
  Qt6::Widgets
  $<$<BOOL:${Dtk6Core_FOUND}>:Dtk6::Core>
  $<$<BOOL:${Dtk6Gui_FOUND}>:Dtk6::Gui>
)

set(TRANSLATE_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/translations" CACHE STRING "Install dir for deepin-im translate files")
set(TRANSLATE_DIR "${CMAKE_INSTALL_PREFIX}/${TRANSLATE_INSTALL_DIR}")
target_compile_definitions(trayicon PRIVATE TRANSLATE_DIR="${TRANSLATE_DIR}")

install(TARGETS trayicon
  LIBRARY DESTINATION ${DIM_ADDON_DIR}
)

install(FILES trayicon.conf
  DESTINATION ${DIM_ADDON_INFO_DIR}
)

file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION ${TRANSLATE_INSTALL_DIR})
