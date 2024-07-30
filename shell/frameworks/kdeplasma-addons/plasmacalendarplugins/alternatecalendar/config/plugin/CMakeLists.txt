# SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(plasmacalendaralternatecalendarconfig
    URI org.kde.plasma.private.alternatecalendarconfig
    GENERATE_PLUGIN_SOURCE
)

target_sources(plasmacalendaralternatecalendarconfig
    PRIVATE
        ../../calendarsystem.h
        configstorage.cpp
        configstorage.h
)

target_link_libraries(plasmacalendaralternatecalendarconfig
    PRIVATE
        Qt::Qml
        Qt::Core
        KF6::ConfigCore
        KF6::I18n
)

ecm_finalize_qml_module(plasmacalendaralternatecalendarconfig)
