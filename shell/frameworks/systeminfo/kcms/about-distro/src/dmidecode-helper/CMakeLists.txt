# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

find_package(dmidecode)
set_package_properties(dmidecode PROPERTIES TYPE RUNTIME)

add_executable(kinfocenter-dmidecode-helper helper.cpp)
target_link_libraries(kinfocenter-dmidecode-helper
    Qt::Widgets
    KF6::AuthCore
    KF6::I18n
)

kauth_install_actions(org.kde.kinfocenter.dmidecode org.kde.kinfocenter.dmidecode.actions)
kauth_install_helper_files(kinfocenter-dmidecode-helper org.kde.kinfocenter.dmidecode root)
install(TARGETS kinfocenter-dmidecode-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
