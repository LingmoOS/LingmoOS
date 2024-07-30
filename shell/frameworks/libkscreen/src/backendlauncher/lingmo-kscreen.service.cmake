[Unit]
Description=KScreen
PartOf=graphical-session.target

[Service]
ExecStart=@KDE_INSTALL_FULL_LIBEXECDIR_KF@/kscreen_backend_launcher
Type=dbus
BusName=org.kde.KScreen
TimeoutSec=5sec
Slice=background.slice
# Disable restart as we're dbus activated anyway
Restart=no
