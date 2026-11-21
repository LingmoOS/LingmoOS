find_package(LingmoSDK REQUIRED)

add_executable(lingmo-polkit-agent
    main.cpp
    polkitagentlistener.cpp
    polkitagentlistener.h
    authsession.cpp
    authsession.h
    dialog.cpp
    dialog.h
)
set_target_properties(lingmo-polkit-agent PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

target_link_libraries(lingmo-polkit-agent PRIVATE Lingmo::SDK)

install(TARGETS lingmo-polkit-agent
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Runtime
)
