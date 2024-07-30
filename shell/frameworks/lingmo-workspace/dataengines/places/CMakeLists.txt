set(places_engine_SRCS
    placesengine.cpp
    placeservice.cpp
    placesproxymodel.cpp
    setupdevicejob.cpp
    modeljob.h
)

set(CMAKE_AUTOMOC TRUE)

kcoreaddons_add_plugin(lingmo_engine_places SOURCES ${places_engine_SRCS} INSTALL_NAMESPACE lingmo5support/dataengine)

target_link_libraries(lingmo_engine_places
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::Solid
)

install(FILES org.kde.places.operations
        DESTINATION ${LINGMO5SUPPORT_DATA_INSTALL_DIR}/services )

