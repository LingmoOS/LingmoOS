set(SDDM_CONFIG_FILE                 "/etc/sddm.conf"                                    CACHE PATH      "Path of the sddm config file")
set(SDDM_CONFIG_DIR                  "/etc/sddm.conf.d"                                  CACHE PATH      "Path of the sddm config directory")
set(SDDM_SYSTEM_CONFIG_DIR           "${CMAKE_INSTALL_PREFIX}/lib/sddm/sddm.conf.d"      CACHE PATH      "Path of the system sddm config directory")
set(XSESSIONS_DIR                    "${CMAKE_INSTALL_PREFIX}/share/xsessions"           CACHE PATH      "Path of the xsessions")
set(WAYLAND_SESSIONS_DIR             "${CMAKE_INSTALL_PREFIX}/share/wayland-sessions"    CACHE PATH      "Path of the wayland sessions")

configure_file(config.h.in config.h IMMEDIATE @ONLY)

kcmutils_add_qml_kcm(kcm_sddm)

target_sources(kcm_sddm PRIVATE
    sddmkcm.cpp sddmkcm.h
    themesmodel.cpp themesmodel.h
    thememetadata.cpp thememetadata.h
    usersmodel.cpp usersmodel.h
    sessionmodel.cpp sessionmodel.h
    sddmdata.cpp sddmdata.h
    sddmsettingsbase.cpp sddmsettingsbase.h
)
kconfig_add_kcfg_files(kcm_sddm sddmsettings.kcfgc GENERATE_MOC)

target_compile_definitions(kcm_sddm PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kcm_sddm PRIVATE
    KF6::I18n
    KF6::AuthCore
    KF6::KCMUtils
    KF6::KIOGui
    KF6::Service
)
