#NOTE: must set QT_QPA_PLATFORM here, dbus activated services from DDE env will 
#need it. kwin_wayland_helper won't use it since kwin_wayland will reset it.
sleep 1
export QT_QPA_PLATFORM=xcb
/usr/bin/dde_update_dbus_env
if [ -z "$DBUS_SESSION_BUS_ADDRESS" ]; then
  exec dbus-launch kwin_wayland_helper
else
  exec kwin_wayland_helper
fi
