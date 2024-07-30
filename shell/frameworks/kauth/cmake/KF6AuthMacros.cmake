# SPDX-FileCopyrightText: 2006-2009 Alexander Neundorf <neundorf@kde.org>
# SPDX-FileCopyrightText: 2006, 2007 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

# This macro adds the needed files for an helper executable meant to be used by applications using KAuth.
# It accepts the helper target, the helper ID (the DBUS name) and the user under which the helper will run on.
# This macro takes care of generate the needed files, and install them in the right location. This boils down
# to a DBus policy to let the helper register on the system bus, and a service file for letting the helper
# being automatically activated by the system bus.
# *WARNING* You have to install the helper in ${KAUTH_HELPER_INSTALL_DIR} to make sure everything will work.
function(KAUTH_INSTALL_HELPER_FILES HELPER_TARGET HELPER_ID HELPER_USER)
    if(KAUTH_HELPER_BACKEND_NAME STREQUAL "DBUS")
        configure_file(${KAUTH_STUB_FILES_DIR}/dbus_policy.stub
                        ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf
                DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d/)

        configure_file(${KAUTH_STUB_FILES_DIR}/dbus_service.stub
                        ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service
                DESTINATION ${KDE_INSTALL_DBUSSYSTEMSERVICEDIR})
    endif()
endfunction()

# This macro generates an action file, depending on the backend used, for applications using KAuth.
# It accepts the helper id (the DBUS name) and a file containing the actions (check kdelibs/kdecore/auth/example
# for file format). The macro will take care of generating the file according to the backend specified,
# and to install it in the right location. This (at the moment) means that on Linux (PolicyKit) a .policy
# file will be generated and installed into the policykit action directory (usually /usr/share/PolicyKit/policy/),
# and on Mac (Authorization Services) will be added to the system action registry using the native MacOS API during
# the install phase
function(KAUTH_INSTALL_ACTIONS HELPER_ID ACTIONS_FILE)

  if(KAUTH_BACKEND_NAME STREQUAL "APPLE" OR KAUTH_BACKEND_NAME STREQUAL "OSX")
    get_target_property(kauth_policy_gen KF6::kauth-policy-gen LOCATION)
    install(CODE "execute_process(COMMAND ${kauth_policy_gen} ${ACTIONS_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
    message(STATUS "installation will execute ${kauth_policy_gen} ${ACTIONS_FILE} in ${CMAKE_CURRENT_SOURCE_DIR}")
  elseif(KAUTH_BACKEND_NAME STREQUAL "POLKITQT6-1")
    set(_output ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.policy)
    get_filename_component(_input ${ACTIONS_FILE} ABSOLUTE)

    add_custom_command(OUTPUT ${_output}
                       COMMAND KF6::kauth-policy-gen ${_input} ${_output}
                       MAIN_DEPENDENCY ${_input}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                       COMMENT "Generating ${HELPER_ID}.policy"
                       DEPENDS KF6::kauth-policy-gen)
    add_custom_target(${HELPER_ID}.policy-customtarget ALL COMMENT "actions for ${HELPER_ID}" DEPENDS ${_output})

    if(INSTALL_BROKEN_KAUTH_POLICY_FILES)
      set(OVERRIDEN_KAUTH_POLICY_FILES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions/)
      if(NOT OVERRIDEN_KAUTH_POLICY_FILES_INSTALL_DIR STREQUAL KAUTH_POLICY_FILES_INSTALL_DIR)
        MESSAGE(WARNING "INSTALL_BROKEN_KAUTH_POLICY_FILES is enabled. The following files will be installed to ${OVERRIDEN_KAUTH_POLICY_FILES_INSTALL_DIR} instead of ${KAUTH_POLICY_FILES_INSTALL_DIR}.\n${_output}")
      endif()
      install(FILES ${_output} DESTINATION ${OVERRIDEN_KAUTH_POLICY_FILES_INSTALL_DIR})
    else()
      install(FILES ${_output} DESTINATION ${KAUTH_POLICY_FILES_INSTALL_DIR})
    endif()
  endif()

endfunction()
