set(ACCOUNTS_SRCS
    accountsmanager.cpp
    accountsmanager.h
    accountsmanager_p.h
    useraccount.cpp
    useraccount.h
    useraccount_p.h
    usersmodel.cpp
    usersmodel.h
    usersmodel_p.h
    qmlplugins.cpp
)

qt5_add_dbus_interface(ACCOUNTS_SRCS
                       org.freedesktop.Accounts.User.xml
                       user_interface)
qt5_add_dbus_interface(ACCOUNTS_SRCS
                       org.freedesktop.Accounts.xml
                       accounts_interface)

# set_source_files_properties(${ACCOUNTS_SRCS} PROPERTIES SKIP_AUTOGEN ON)

add_library(lingmoaccounts_qmlplugins SHARED ${ACCOUNTS_SRCS})

target_link_libraries (lingmoaccounts_qmlplugins
    Qt5::Core
    Qt5::DBus
    Qt5::Quick
    Qt5::Gui
)

install(TARGETS lingmoaccounts_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Lingmo/Accounts)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Lingmo/Accounts)