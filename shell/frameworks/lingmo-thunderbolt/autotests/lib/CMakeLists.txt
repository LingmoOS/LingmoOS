include(ECMAddTests)

macro(add_libkbolt_test name)
    ecm_add_test(
        ${name}.cpp
        LINK_LIBRARIES kbolt libboltfakeserver Qt::Test Qt::Core Qt::DBus
        NAME_PREFIX libkbolt-
    )
    set_target_properties(${name} PROPERTIES CXX_STANDARD 14)
endmacro()

add_libkbolt_test(managertest)
add_libkbolt_test(devicetest)

