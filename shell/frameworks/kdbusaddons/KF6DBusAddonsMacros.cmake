set(_KDBUSADDONSDIR ${CMAKE_CURRENT_LIST_DIR})

#
# kdbusaddons_generate_dbus_service_file(EXECUTABLE NAME PATH)
#
# This function generates a D-Bus service file and installs it
# into the specified destination.
#
# On Windows the path is not used because D-Bus service executables
# are always installed in the same directory as dbus-daemon.
#
# Parameters:
#
# EXECUTABLE - name of the executable
# NAME       - service path in D-Bus
# PATH       - path to install the service file
#
# Example:
#
#  kdbusaddons_generate_dbus_service_file(kded6 org.kde.kded6 ${KDE_INSTALL_FULL_BINDIR})
#
# Since 5.42.0

function(KDBUSADDONS_GENERATE_DBUS_SERVICE_FILE _executable _name _path)
    set(_file ${CMAKE_CURRENT_BINARY_DIR}/${_name}.service)
    set(_template ${_KDBUSADDONSDIR}/KF6dbus.service.in)
    set(NAME ${_name})
    if(WIN32)
        set(EXEC ${_executable})
    else()
        if(IS_ABSOLUTE ${_path})
            set(EXEC ${_path}/${_executable})
        else()
            message(FATAL_ERROR "Path ${_path} is not absolute")
        endif()
    endif()
    if(NOT EXISTS ${_file})
        message(STATUS "generating ${_file} from ${_template}")
    endif()
    configure_file(${_template} ${_file})
    install(FILES ${_file} DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
endfunction()
