set(POLKIT-ACTION_FILES
    org.deepin.dde.permission.modify-app-permissions.policy
)

install(FILES ${POLKIT-ACTION_FILES} DESTINATION /usr/share/polkit-1/actions/)
