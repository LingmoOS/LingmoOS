add_executable(kconf_update kconf_update.cpp)
add_executable(KF6::kconf_update ALIAS kconf_update)

target_compile_definitions(kconf_update PRIVATE -DCMAKE_INSTALL_FULL_LIBDIR="${CMAKE_INSTALL_FULL_LIBDIR}")
ecm_qt_declare_logging_category(kconf_update
    HEADER kconf_update_debug.h
    IDENTIFIER KCONF_UPDATE_LOG
    CATEGORY_NAME kf.config.kconf_update
    OLD_CATEGORY_NAMES kf5.kconfig.update
    DESCRIPTION "kconf_update"
    EXPORT KCONFIG
)

target_link_libraries(kconf_update Qt6::Core KF6::ConfigCore)
include(ECMMarkNonGuiExecutable)
ecm_mark_nongui_executable(kconf_update)

# Although this is mostly an internal binary (hence installing it in
# KF6_LIBEXEC_INSTALL_DIR), it is used by kded, and so we export its location
install(TARGETS kconf_update EXPORT KF6ConfigCompilerTargets DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})
