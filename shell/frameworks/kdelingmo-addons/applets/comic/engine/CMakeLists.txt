# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

add_library(lingmo_engine_comic STATIC)

set_property(TARGET lingmo_engine_comic PROPERTY POSITION_INDEPENDENT_CODE ON)

ecm_qt_declare_logging_category(lingmo_engine_comic
    HEADER comic_debug.h
    IDENTIFIER LINGMO_COMIC
    CATEGORY_NAME ork.kde.lingmo.comic
    DESCRIPTION "Lingmo Comic Engine"
    EXPORT KDELINGMOADDONS
)

target_sources(lingmo_engine_comic PRIVATE
    comicprovider.cpp
    cachedprovider.cpp
    comic.cpp
    comicproviderkross.cpp
    comicproviderwrapper.cpp
)
target_link_libraries(lingmo_engine_comic
    Qt::Qml
    KF6::WidgetsAddons
    KF6::Package
    KF6::KIOCore
)

kcoreaddons_add_plugin(lingmo_comic SOURCES comic_package.cpp INSTALL_NAMESPACE "kf6/packagestructure")
target_link_libraries(lingmo_comic KF6::ConfigCore KF6::Package)
