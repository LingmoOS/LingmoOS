# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_plymouth\")

SET(PLYMOUTH_THEMES_DIR "/usr/share/plymouth/themes/" CACHE STRING "Where Plymouth themes are installed")
SET(PLYMOUTH_CONFIG_PATH "/etc/plymouth/plymouthd.conf" CACHE STRING "where the main plymouth config file is")
configure_file (config-kcm.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcm.h )

kcmutils_add_qml_kcm(kcm_plymouth SOURCES kcm.cpp kcm.h)

target_link_libraries(kcm_plymouth PRIVATE
  KF6::CoreAddons
  KF6::AuthCore
  KF6::I18n
  KF6::KCMUtils
  KF6::NewStuffCore
  KF6::ConfigCore
  KF6::KIOCore
)

install(FILES plymouth.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

#installer tool for knewstuff
add_executable(kplymouththemeinstaller
    kplymouththemeinstaller.cpp
)

target_link_libraries(kplymouththemeinstaller
    KF6::I18n
    KF6::AuthCore
    KF6::CoreAddons
    KF6::Archive
    KF6::ConfigCore
)
install(TARGETS kplymouththemeinstaller ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

#polkit stuff
add_executable(plymouthhelper helper.cpp helper.h)
target_link_libraries(plymouthhelper
    KF6::Archive
    KF6::AuthCore
    KF6::ConfigCore
    KF6::I18n
)
install(TARGETS plymouthhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(plymouthhelper org.kde.kcontrol.kcmplymouth root)
kauth_install_actions(org.kde.kcontrol.kcmplymouth kcmplymouth_actions.actions)
