# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

add_library(plasma_engine_comic STATIC)

set_property(TARGET plasma_engine_comic PROPERTY POSITION_INDEPENDENT_CODE ON)

ecm_qt_declare_logging_category(plasma_engine_comic
    HEADER comic_debug.h
    IDENTIFIER PLASMA_COMIC
    CATEGORY_NAME ork.kde.plasma.comic
    DESCRIPTION "Plasma Comic Engine"
    EXPORT KDEPLASMAADDONS
)

target_sources(plasma_engine_comic PRIVATE
    comicprovider.cpp
    cachedprovider.cpp
    comic.cpp
    comicproviderkross.cpp
    comicproviderwrapper.cpp
)
target_link_libraries(plasma_engine_comic
    Qt::Qml
    KF6::WidgetsAddons
    KF6::Package
    KF6::KIOCore
)

kcoreaddons_add_plugin(plasma_comic SOURCES comic_package.cpp INSTALL_NAMESPACE "kf6/packagestructure")
target_link_libraries(plasma_comic KF6::ConfigCore KF6::Package)
