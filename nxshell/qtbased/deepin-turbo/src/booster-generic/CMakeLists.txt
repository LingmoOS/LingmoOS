set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")
set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON} ${LAUNCHER})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Set sources
set(SRC booster-generic.cpp)

# Set libraries to be linked.
link_libraries("-L../launcherlib -lapplauncherd" ${LIBDL})

# Set executable
add_executable(booster-generic ${SRC} ${MOC_SRC})
add_dependencies(booster-generic applauncherd)

# Add install rule
install(TARGETS booster-generic DESTINATION /usr/libexec/mapplauncherd/)
install(FILES booster-generic.service DESTINATION /usr/lib/systemd/user/)
