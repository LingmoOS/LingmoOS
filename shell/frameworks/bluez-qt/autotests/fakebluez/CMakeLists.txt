set(fakebluez_SRCS
    main.cpp
    fakebluez.cpp
    testinterface.cpp
    objectmanager.cpp
    object.cpp
    agentmanager.cpp
    profilemanager.cpp
    devicemanager.cpp
    adapterinterface.cpp
    deviceinterface.cpp
    gattserviceinterface.cpp
    gattcharacteristicinterface.cpp
    gattdescriptorinterface.cpp
    batteryinterface.cpp
    inputinterface.cpp
    mediaplayerinterface.cpp
    mediatransportinterface.cpp
    obexagentmanager.cpp
    obexclient.cpp
    mediainterface.cpp
    leadvertisingmanagerinterface.cpp
    gattmanagerinterface.cpp
)

add_executable(fakebluez ${fakebluez_SRCS})

target_link_libraries(fakebluez
    Qt6::Core
    Qt6::DBus
)
