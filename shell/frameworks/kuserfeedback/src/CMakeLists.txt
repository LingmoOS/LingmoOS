add_subdirectory(common)
add_subdirectory(provider)

if (ENABLE_PHP AND ENABLE_PHP_UNIT)
    add_subdirectory(server)
    add_subdirectory(testserver)
endif()

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_generic_lambdas has_generic_lambdas)
if((Qt6Charts_FOUND AND Qt6Svg_FOUND AND Qt6PrintSupport_FOUND)
    AND NOT has_generic_lambdas LESS 0 )
    if (ENABLE_CONSOLE)
        add_subdirectory(console)
    endif()

    if (ENABLE_CLI)
        add_subdirectory(cli)
    endif()
endif()
