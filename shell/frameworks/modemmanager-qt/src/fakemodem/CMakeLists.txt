include_directories(${MODEMMANAGER_INCLUDE_DIRS})

set(fakeModem_SRCS
    bearer.cpp
    fakemodem.cpp
    modem.cpp
    modem3gpp.cpp
    modem3gppussd.cpp
    modemcdma.cpp
    modemmessaging.cpp
    modemlocation.cpp
    modemsimple.cpp
    modemtime.cpp
    objectmanager.cpp
    sms.cpp
)

if (${MODEMMANAGER_VERSION} VERSION_EQUAL 1.2.0 OR ${MODEMMANAGER_VERSION} VERSION_GREATER 1.2.0)
    set(fakeModem_SRCS
        ${fakeModem_SRCS}
        modemoma.cpp
        )
endif()

add_library(fakeModem STATIC ${fakeModem_SRCS})

target_link_libraries(fakeModem
    KF6ModemManagerQt_static
    Qt6::Core
    Qt6::DBus)
