# SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

# SPDX-License-Identifier: GPL-2.0-or-later

set(alternatecalendar_LIB_SRCS
    alternatecalendarplugin.cpp
    calendarsystem.h
    provider/abstractcalendarprovider.cpp
    provider/qtcalendar.cpp
)

set(alternatecalendar_LINK_LIBRARIES
    KF6::ConfigCore
    KF6::CalendarEvents
    KF6::I18n
)

if(${HAVE_ICU})
    set(alternatecalendar_LIB_SRCS
        ${alternatecalendar_LIB_SRCS}
        provider/icucalendar_p.cpp
        provider/chinesecalendar.cpp
        provider/hebrewcalendar.cpp
        provider/indiancalendar.cpp
        provider/islamiccalendar.cpp
        provider/solarutils.h
    )
    set(alternatecalendar_LINK_LIBRARIES
        ${alternatecalendar_LINK_LIBRARIES}
        ICU::uc
        ICU::i18n
    )
endif()

configure_file(config-ICU.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ICU.h)

kcoreaddons_add_plugin(alternatecalendar SOURCES ${alternatecalendar_LIB_SRCS} INSTALL_NAMESPACE "plasmacalendarplugins")
target_link_libraries(alternatecalendar ${alternatecalendar_LINK_LIBRARIES})

add_subdirectory(config)
