project(lingmo-session)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(TARGET lingmo-session)

set(SOURCES
        application.cpp
        main.cpp
        process.cpp
        processmanager.cpp
        networkproxymanager.cpp

        powermanager/power.cpp
        powermanager/powerproviders.cpp

        daemon-helper.cpp
        daemon-helper.h
)

qt6_add_dbus_adaptor(DBUS_SOURCES
                     com.lingmo.Session.xml
                     application.h Application
                     sessionadaptor SessionAdaptor)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

find_package(KF6WindowSystem)
find_package(Threads)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_link_libraries(${TARGET}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Quick
        Qt6::DBus
        Qt6::GuiPrivate
        KF6::WindowSystem
        ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lingmo-xsession.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/xsessions/)
