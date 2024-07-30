# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_BINARY_DIR})

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
)


find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    Auth
)


set(ufw_plugin_helper_SRCS helper.cpp helper.h)
add_executable(kde_ufw_plugin_helper ${ufw_plugin_helper_SRCS})

# set_target_properties(kcm_ufw_helper PROPERTIES OUTPUT_NAME kcm_ufw_helper)
target_link_libraries(kde_ufw_plugin_helper Qt::Core KF6::AuthCore KF6::I18n)

if (IS_ABSOLUTE "${KDE_INSTALL_LIBEXECDIR}")
  set(UFW_PLUGIN_HELPER_PATH ${KDE_INSTALL_LIBEXECDIR}/kde_ufw_plugin_helper.py)
else()
  set(UFW_PLUGIN_HELPER_PATH ${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBEXECDIR}/kde_ufw_plugin_helper.py)
endif()
configure_file(kcm_ufw_helper.py.cmake ${CMAKE_BINARY_DIR}/kde_ufw_plugin_helper.py)
configure_file(ufw_helper_config.h.cmake ${CMAKE_BINARY_DIR}/ufw_helper_config.h)

kauth_install_actions(org.kde.ufw org.kde.ufw.actions)
install(TARGETS kde_ufw_plugin_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kde_ufw_plugin_helper org.kde.ufw root)

# install(TARGETS kcm_ufw_helper DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${CMAKE_BINARY_DIR}/kde_ufw_plugin_helper.py DESTINATION ${KDE_INSTALL_LIBEXECDIR})

# configure_file(org.kde.ufw.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.ufw.service)
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/session/org.kde.ufw.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
install(FILES defaults DESTINATION ${KDE_INSTALL_DATADIR}/kcm_ufw)
