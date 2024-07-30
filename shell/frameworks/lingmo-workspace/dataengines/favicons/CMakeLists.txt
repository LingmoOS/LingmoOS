set(favicons_engine_SRCS
    favicons.cpp
    faviconprovider.cpp
    favicons.h
    faviconprovider.h
)

kcoreaddons_add_plugin(lingmo_engine_favicons SOURCES ${favicons_engine_SRCS} INSTALL_NAMESPACE lingmo5support/dataengine)
target_link_libraries(lingmo_engine_favicons
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::KIOCore
    Qt::Gui
)
