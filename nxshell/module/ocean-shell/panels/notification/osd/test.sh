#!/bin/bash

# Listen to the `org.lingmo.ocean.Osd1` received by `ocean-osd` and forward it to `org.lingmo.ocean.Shell`
#busctl --user monitor --match="type='method_call',eavesdrop='true',destination='org.lingmo.ocean.Osd1'" -j |jq --unbuffered -r '.payload.data[0]' |xargs -I {}  dbus-send --session --type=method_call --print-reply --dest=org.lingmo.ocean.Shell /org/lingmo/osdService org.lingmo.osdService.showText string:{}

audio=("AudioUp" "AudioDown" "AudioMute" "AudioUpAsh" "AudioDownAsh" "AudioMuteAsh")
brightness=("BrightnessUp" "BrightnessDown" "BrightnessUpAsh" "BrightnessDownAsh")
default=("WLANOn" "WLANOff" "CapsLockOn" "CapsLockOff" "NumLockOn" "NumLockOff" "TouchpadOn" "TouchpadOff" "TouchpadToggle" "FnToggle" "AirplaneModeOn" "AirplaneModeOff" "AudioMicMuteOn" "AudioMicMuteOff" "balance" "powersave" "performance" "SwitchWM3D" "SwitchWM2D" "SwitchWMError")
display=("SwitchMonitors")
kblayout=("SwitchLayout")

#e.g: dbus-send --session --type=method_call --print-reply --dest=org.lingmo.ocean.Shell /org/lingmo/osdService org.lingmo.osdService.showText string:AudioUp

for type in  ${audio[@]} ${kblayout[@]} ${display[@]} ${brightness[@]} ${default[@]} ;
do

echo dbus-send --session --type=method_call --print-reply --dest=org.lingmo.ocean.Shell /org/lingmo/osdService org.lingmo.osdService.showText string:$type

dbus-send --session --type=method_call --print-reply --dest=org.lingmo.ocean.Shell /org/lingmo/osdService org.lingmo.osdService.showText string:$type
sleep 1

done
