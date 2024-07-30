include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)


add_library(KSC_QScreen MODULE)
target_sources(KSC_QScreen PRIVATE
    qscreenbackend.cpp
    qscreenconfig.cpp
    qscreenscreen.cpp
    qscreenoutput.cpp
    qscreenbackend.h
    qscreenconfig.h
    qscreenscreen.h
    qscreenoutput.h
)

set_target_properties(KSC_QScreen PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kscreen")
set_target_properties(KSC_QScreen PROPERTIES PREFIX "")
target_link_libraries(KSC_QScreen Qt::Core
                                 Qt::Gui
                                 KF6::Screen
)
if (TARGET Qt::X11Extras)
    target_link_libraries(KSC_QScreen Qt::X11Extras)
endif()

install(TARGETS KSC_QScreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kscreen/)
