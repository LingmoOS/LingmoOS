#!/bin/bash

output_dir="./dbus_gen"
mkdir -p "${output_dir}"

# org.lingmo.ocean.Network1
dbus-send --type=method_call --print-reply --dest=org.lingmo.ocean.Network1 /org/lingmo/ocean/Network1 org.freedesktop.DBus.Introspectable.Introspect | sed 1d | sed -e '1s/^   string "//' | sed '$s/"$//' > "${output_dir}"/dbus_ocean_daemon_network.xml
python3 -m dbus2any -t pydbusclient.tpl -x "${output_dir}"/dbus_ocean_daemon_network.xml > "${output_dir}"/com_lingmo_daemon_Network.py

if [ $? -ne 0 ]; then
  echo "run 'sudo pip3 install dbus2any' and Fix dbus2any templates missing issue manually"
  echo "  dbus2any_tpl_dir=/usr/lib/python3.5/site-packages/dbus2any/templates # or maybe /usr/local/lib/python3.5/dist-packages/dbus2any/templates"
  echo "  sudo mkdir \${dbus2any_tpl_dir}"
  echo "  curl https://raw.githubusercontent.com/hugosenari/dbus2any/master/dbus2any/templates/pydbusclient.tpl | sudo tee \${dbus2any_tpl_dir}/pydbusclient.tpl"
  exit 1
fi

# com.lingmo.daemon.ConnectionSession
session_path=$(dbus-send --type=method_call --print-reply --dest=org.lingmo.ocean.Network1 /org/lingmo/ocean/Network1 org.lingmo.ocean.Network1.CreateConnection string:"vpn-openvpn" objpath:"/" | sed 1d | sed -e 's/   object path "//' | sed -e 's/"$//')
dbus-send --type=method_call --print-reply --dest=org.lingmo.ocean.Network1 ${session_path} org.freedesktop.DBus.Introspectable.Introspect | sed 1d | sed -e '1s/^   string "//' | sed '$s/"$//' > "${output_dir}"/dbus_ocean_daemon_network_connectionsession.xml
python3 -m dbus2any -t pydbusclient.tpl -x "${output_dir}"/dbus_ocean_daemon_network_connectionsession.xml > "${output_dir}"/com_lingmo_daemon_Network_ConnectionSession.py
dbus-send --type=method_call --print-reply --dest=org.lingmo.ocean.Network1 ${session_path} com.lingmo.daemon.ConnectionSession.Close

echo 'Done'
