# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2023 Adrian Edwards <adrian@adriancedwards.com>

add_library(ksystemstats_plugin_pressure MODULE pressure.cpp)
target_link_libraries(ksystemstats_plugin_pressure Qt::Core Qt::Gui KF6::CoreAddons KF6::I18n KSysGuard::SystemStats)

ecm_qt_declare_logging_category(ksystemstats_plugin_pressure
    HEADER pressure_logging.h
    IDENTIFIER KSYSTEMSTATS_PRESSURE
    CATEGORY_NAME org.kde.ksystemstats.plugins.pressure
    DESCRIPTION "ksystemstats pressure plugin"
	DEFAULT_SEVERITY Warning
    EXPORT KSYSTEMSTATS_PRESSURE
)


install(TARGETS ksystemstats_plugin_pressure DESTINATION ${KSYSTEMSTATS_PLUGIN_INSTALL_DIR})
