#!/bin/bash

#busctl --user monitor org.deepin.dde.Notification1

#dbus-send --session --type=method_call --dest=org.deepin.dde.Notification1 /org/deepin/dde/Notification1 org.deepin.dde.Notification1.ReplaceBubble boolean:false

notify-send 1 -a dde-control-center -t 3000
notify-send 2 -a dde-control-center -t 4000
notify-send 3 -a dde-control-center -t 5000
notify-send 4 -a dde-control-center -t 5000

notify-send 3 -a element

sleep 3

notify-send "Memory Consumption (%):" "$(ps axch -o cmd:15,%mem --sort=-%mem | head -n 30)"

notify-send "long body long body long body long body long body long body long body long body long body" "summary"

dunstify -a dde-control-center -i dde-control-center body summary

dunstify -a dde-control-center -i deepin-screen-recorder hello-world!hello-world!hello-world!hello-world! summary -A "sdf,defaults"

NOTIFICATION_ID=$(dunstify -p -a dde-control-center -i dde-control-center body summary)
dunstify -a dde-control-center -i dde-control-center body summary-replace -r ${NOTIFICATION_ID}
