########### next target ###############

set(kwin_killer_helper_SRCS killer.cpp)

add_executable(kwin_killer_helper ${kwin_killer_helper_SRCS})

set_target_properties(kwin_killer_helper PROPERTIES
    OUTPUT_NAME deepin-kwin_killer_helper
)

target_link_libraries(kwin_killer_helper
    KF5::AuthCore
    KF5::I18n
    KF5::WidgetsAddons
    Qt${QT_MAJOR_VERSION}::Widgets
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(kwin_killer_helper Qt${QT_MAJOR_VERSION}::X11Extras)
else()
    target_link_libraries(kwin_killer_helper Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()

install(TARGETS kwin_killer_helper DESTINATION ${KDE_INSTALL_LIBEXECDIR})
