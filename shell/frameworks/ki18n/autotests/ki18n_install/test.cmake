# Make sure file names with dots in them are properly processed.
# https://bugs.kde.org/show_bug.cgi?id=379116
if(NOT EXISTS "destdir/share/locale/de/LC_MESSAGES/lingmo_applet_org.kde.lingmo.katesessions.mo")
    message(SEND_ERROR "destdir/share/locale/de/LC_MESSAGES/lingmo_applet_org.kde.lingmo.katesessions.mo was not found")
endif()
