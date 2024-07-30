
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED CONFIG)

include(ECMAddTests)

if(UNIX)
    add_executable(kdbussimpleservice kdbussimpleservice.cpp)
    target_link_libraries(kdbussimpleservice Qt6::Core KF6::DBusAddons)

    ecm_add_tests(
        deadservicetest.cpp
        LINK_LIBRARIES Qt6::Test KF6::DBusAddons
    )

    add_dependencies(deadservicetest kdbussimpleservice)
endif()

ecm_add_tests(
    kdbusservicetest.cpp
    LINK_LIBRARIES Qt6::Test KF6::DBusAddons
)
