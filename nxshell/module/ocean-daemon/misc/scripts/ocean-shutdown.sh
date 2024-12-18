#!/bin/sh

originmap=$(setxkbmap -query | grep option | awk -F ' ' '{print $2}');
setxkbmap -option grab:break_actions &&
    xdotool key XF86Ungrab &&
    dbus-send --print-reply --dest=org.lingmo.ocean.ShutdownFront1 /org/lingmo/ocean/ShutdownFront1 org.lingmo.ocean.ShutdownFront1.Show
setxkbmap -option $originmap
