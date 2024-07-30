lingmo_install_package(activitypager org.kde.lingmo.activitypager)

add_subdirectory(kicker)
add_subdirectory(kickoff)
add_subdirectory(trash)

add_subdirectory(taskmanager)
lingmo_install_package(window-list org.kde.lingmo.windowlist)
lingmo_install_package(icontasks org.kde.lingmo.icontasks)
lingmo_install_package(margins-separator org.kde.lingmo.marginsseparator)

if(LingmoActivities_FOUND)
   add_subdirectory(pager)
   add_subdirectory(showActivityManager)
endif()

add_subdirectory(minimizeall)
add_subdirectory(showdesktop)
add_subdirectory(kimpanel)
lingmo_install_package(keyboardlayout org.kde.lingmo.keyboardlayout)
