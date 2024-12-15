#!/bin/bash

#busctl --user monitor org.lingmo.ocean.Notification1

#dbus-send --session --type=method_call --dest=org.lingmo.ocean.Notification1 /org/lingmo/ocean/Notification1 org.lingmo.ocean.Notification1.ReplaceBubble boolean:false

notify-send 1 -a ocean-control-center -t 3000
notify-send 2 -a ocean-control-center -t 4000
notify-send 3 -a ocean-control-center -t 5000
notify-send 4 -a ocean-control-center -t 5000

notify-send 3 -a element

sleep 3

notify-send "Memory Consumption (%):" "$(ps axch -o cmd:15,%mem --sort=-%mem | head -n 30)"

notify-send "long body long body long body long body long body long body long body long body long body" "summary"

dunstify -a ocean-control-center -i ocean-control-center body summary

dunstify -a ocean-control-center -i lingmo-screen-recorder hello-world!hello-world!hello-world!hello-world! summary -A "sdf,defaults"

NOTIFICATION_ID=$(dunstify -p -a ocean-control-center -i ocean-control-center body summary)
dunstify -a ocean-control-center -i ocean-control-center body summary-replace -r ${NOTIFICATION_ID}
