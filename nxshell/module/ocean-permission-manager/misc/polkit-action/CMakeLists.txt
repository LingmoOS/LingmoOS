set(POLKIT-ACTION_FILES
    org.lingmo.ocean.permission.modify-app-permissions.policy
)

install(FILES ${POLKIT-ACTION_FILES} DESTINATION /usr/share/polkit-1/actions/)
