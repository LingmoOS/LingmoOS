plasma_install_package(org.lingmo.ocean org.lingmo.ocean.desktop look-and-feel lookandfeel)
plasma_install_package(org.lingmo.oceandark org.lingmo.oceandark.desktop look-and-feel lookandfeel)

# if (INSTALL_SDDM_THEME)
#     configure_file(sddm-theme/theme.conf.cmake ${CMAKE_CURRENT_BINARY_DIR}/sddm-theme/theme.conf)

# # Install the login theme into the SDDM directory
# # Longer term we need to look at making SDDM load from look and feel somehow.. and allow copying at runtime
#     #NOTE this trailing slash is important to rename the directory
#     install(DIRECTORY sddm-theme/ DESTINATION ${KDE_INSTALL_FULL_DATADIR}/sddm/themes/breeze PATTERN "README.txt" EXCLUDE PATTERN "components" EXCLUDE PATTERN "dummydata" EXCLUDE
#     PATTERN "theme.conf.cmake" EXCLUDE)
#     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sddm-theme/theme.conf DESTINATION ${KDE_INSTALL_FULL_DATADIR}/sddm/themes/breeze)
#     install(DIRECTORY org.lingmo.ocean/contents/components DESTINATION ${KDE_INSTALL_FULL_DATADIR}/sddm/themes/breeze PATTERN "README.txt" EXCLUDE)

# # Install Debian-themed SDDM theme
#     file(COPY "sddm-theme/Main.qml" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/sddm-theme-lingmo")
#     string(CONCAT SED_EXPR
#       "s=^[[:space:]]*import[[:space:]]*\\\"components\\\"[[:space:]]*$"
#       "=import \\\"../breeze\\\"\\nimport \\\"../breeze/components\\\"=;"
#       "s=\\\"components/=\\\"../breeze/components/=g"
#       )
#     # Patch Main.qml to directly use ../breeze qml files
#     execute_process(COMMAND sed -i -e "${SED_EXPR}" "${CMAKE_CURRENT_SOURCE_DIR}/sddm-theme-lingmo/Main.qml" RESULT_VARIABLE SED_RES OUTPUT_VARIABLE SED_OUT ERROR_VARIABLE SED_ERR)
#     message("Result of sed replacement for lingmo-breeze imports in Main.qml: (Res: ${SED_RES}, Out: ${SED_OUT}, Err: ${SED_ERR})")

#     #NOTE this trailing slash is important to rename the directory
#     install(DIRECTORY sddm-theme-lingmo/ DESTINATION ${KDE_INSTALL_FULL_DATADIR}/sddm/themes/lingmo-breeze)

# endif()

if (BUILD_TESTING)
    add_subdirectory(autotests/lockscreen)
endif()
