include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
)

set(fake_SRCS
    fake.cpp
    parser.cpp
    fake.h
    parser.h
)

qt_add_dbus_adaptor(fake_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.FakeBackend.xml
                     fake.h Fake
)

add_library(KSC_Fake MODULE ${fake_SRCS})

set_target_properties(KSC_Fake PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kscreen")
set_target_properties(KSC_Fake PROPERTIES PREFIX "")
target_link_libraries(KSC_Fake
    Qt::Core
    Qt::DBus
    KF6::Screen
)

install(TARGETS KSC_Fake DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kscreen/)


