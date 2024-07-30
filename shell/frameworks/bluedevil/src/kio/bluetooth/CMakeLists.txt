set(kio_bluetooth_SRCS
    kiobluetooth.cpp
)

set(kded_bluedevil.xml ${CMAKE_SOURCE_DIR}/src/interfaces/kded_bluedevil.xml)
set_source_files_properties(${kded_bluedevil.xml} PROPERTIES INCLUDE ${CMAKE_SOURCE_DIR}/src/interfaces/kded_bluedevil_types.h)
qt_add_dbus_interface(kio_bluetooth_SRCS ${kded_bluedevil.xml} kdedbluedevil)

kconfig_add_kcfg_files(kio_bluetooth_SRCS GENERATE_MOC ../../settings/filereceiversettings.kcfgc)

add_library(kio_bluetooth MODULE ${kio_bluetooth_SRCS})

target_link_libraries(kio_bluetooth
    Qt::Core
    Qt::DBus
    Qt::Network

    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
)

target_include_directories(kio_bluetooth
    PUBLIC $<TARGET_PROPERTY:KF6::BluezQt,INTERFACE_INCLUDE_DIRECTORIES>
)

########### install files ###############
install(TARGETS kio_bluetooth DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kio)
install(FILES bluetooth-network.desktop DESTINATION ${KDE_INSTALL_DATADIR}/remoteview)
