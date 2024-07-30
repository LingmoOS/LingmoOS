add_executable(kmimetypefinder kmimetypefinder.cpp)
target_compile_definitions(kmimetypefinder PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
ecm_mark_nongui_executable(kmimetypefinder)

target_link_libraries(kmimetypefinder
   KF6::CoreAddons # KAboutData
   KF6::I18n       # i18n
   Qt::Core
)

install_compat_symlink(kmimetypefinder)
install(TARGETS kmimetypefinder DESTINATION ${KDE_INSTALL_FULL_BINDIR})
