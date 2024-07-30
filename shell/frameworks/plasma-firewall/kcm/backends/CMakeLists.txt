# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
# SPDX-FileCopyrightText: 2022 Lucas Biaggi <lucas.biaggi@kde.org>

if(BUILD_FIREWALLD_BACKEND AND ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_subdirectory(firewalld)
endif()
if(BUILD_UFW_BACKEND AND ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_subdirectory(ufw)
endif()

add_subdirectory(netstat)
