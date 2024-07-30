include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)


add_library(KSC_XRandR MODULE)
target_sources(KSC_XRandR PRIVATE
    xrandr.cpp
    xrandrconfig.cpp
    xrandrcrtc.cpp
    xrandroutput.cpp
    xrandrmode.cpp
    xrandrscreen.cpp
    ../xcbwrapper.cpp
    ../xcbeventlistener.cpp
    ../utils.cpp
)

set_target_properties(KSC_XRandR PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kscreen")
set_target_properties(KSC_XRandR PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR Qt::Core
                                 Qt::Gui
                                 Qt::GuiPrivate # for QX11Info
                                 ${XCB_LIBRARIES}
                                 KF6::Screen
)

install(TARGETS KSC_XRandR DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kscreen/)
