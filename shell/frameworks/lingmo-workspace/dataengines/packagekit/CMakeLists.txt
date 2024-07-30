set(packagekit_engine_SRCS
    packagekitjob.cpp
    packagekitengine.cpp
    packagekitservice.cpp
    packagekitjob.h
    packagekitengine.h
    packagekitservice.h
)

kcoreaddons_add_plugin(lingmo_engine_packagekit SOURCES ${packagekit_engine_SRCS} INSTALL_NAMESPACE lingmo5support/dataengine)

target_link_libraries(lingmo_engine_packagekit Lingmo::Lingmo Lingmo::Lingmo5Support KF6::CoreAddons Qt::DBus )

install(FILES packagekit.operations
    DESTINATION ${LINGMO5SUPPORT_DATA_INSTALL_DIR}/services)

