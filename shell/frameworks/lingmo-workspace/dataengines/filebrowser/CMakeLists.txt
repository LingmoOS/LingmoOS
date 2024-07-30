set(filebrowser_engine_SRCS
    filebrowserengine.cpp
    filebrowserengine.h
)

kcoreaddons_add_plugin(lingmo_engine_filebrowser SOURCES ${filebrowser_engine_SRCS} INSTALL_NAMESPACE lingmo5support/dataengine)
target_link_libraries(lingmo_engine_filebrowser
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::Service
    KF6::KIOCore
)
