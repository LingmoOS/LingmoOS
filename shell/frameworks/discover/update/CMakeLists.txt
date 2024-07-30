add_executable(lingmo-discover-update main.cpp DiscoverUpdate.cpp)

set_target_properties(lingmo-discover-update PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/lingmo-discover)
target_link_libraries(lingmo-discover-update Discover::Common KF6::CoreAddons KF6::I18n KF6::DBusAddons)

install(TARGETS lingmo-discover-update ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
