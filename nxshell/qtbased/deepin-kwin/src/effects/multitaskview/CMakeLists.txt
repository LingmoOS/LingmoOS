#######################################
# Effect

# Source files
set(multitaskview_SOURCES
    multitaskview.cpp
    multitaskview.qrc
    main.cpp
)
kwin4_add_effect_module(kwin4_effect_multitaskview ${multitaskview_SOURCES})
target_link_libraries(kwin4_effect_multitaskview PRIVATE
    kwineffects
    kwinglutils
    KF5::GlobalAccel
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Concurrent
    PkgConfig::QGSETTINGS
)

# translations
file(GLOB TS LIST_DIRECTORIES false translations/multitasking*.ts)
foreach(_ts_file ${TS})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR}/translations -ts ${_ts_file})
    execute_process(COMMAND lrelease ${_ts_file})
endforeach()

install(DIRECTORY qml DESTINATION ${KDE_INSTALL_DATADIR}/deepin-kwin/effects/multitaskview)