set(executable_engine_SRCS
    executable.cpp
    executable.h
)

kcoreaddons_add_plugin(lingmo_engine_executable SOURCES ${executable_engine_SRCS} INSTALL_NAMESPACE lingmo5support/dataengine)

target_link_libraries(lingmo_engine_executable
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::Service
    KF6::CoreAddons
)
