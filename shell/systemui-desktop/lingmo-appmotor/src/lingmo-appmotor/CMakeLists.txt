set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")
set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON} ${LAUNCHER})

set(QT Widgets Quick QuickControls2)
find_package(Qt5 REQUIRED ${QT})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set sources
set(SRC lingmo-appmotor.cpp)

# Set libraries to be linked.
link_libraries("-L../launcherlib -lapplauncherd" ${LIBDL})

# Set executable
add_executable(lingmo-appmotor ${SRC} ${MOC_SRC})

target_link_libraries(lingmo-appmotor
    Qt5::Widgets
    Qt5::Quick
)

# Add install rule
install(TARGETS lingmo-appmotor DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

if(INSTALL_SYSTEMD_UNITS)
	install(FILES lingmo-appmotor.service DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/)
endif()
