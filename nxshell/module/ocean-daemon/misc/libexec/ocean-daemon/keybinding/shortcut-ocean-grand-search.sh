#!/bin/bash

registered=`dbus-send --session --print-reply --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.NameHasOwner string:com.lingmo.ocean.GrandSearch | grep true`

if [ "$registered" ]
then
	visible=`dbus-send --session --print-reply=literal --dest=com.lingmo.ocean.GrandSearch /com/lingmo/ocean/GrandSearch com.lingmo.ocean.GrandSearch.IsVisible | grep true`
	if [ "$visible" ]
	then
		`dbus-send --session --print-reply=literal --dest=com.lingmo.ocean.GrandSearch /com/lingmo/ocean/GrandSearch com.lingmo.ocean.GrandSearch.SetVisible boolean:false`
	else
		`dbus-send --session --print-reply=literal --dest=com.lingmo.ocean.GrandSearch /com/lingmo/ocean/GrandSearch com.lingmo.ocean.GrandSearch.SetVisible boolean:true`
	fi
else
`dbus-send --session --print-reply=literal --dest=com.lingmo.ocean.GrandSearch /com/lingmo/ocean/GrandSearch com.lingmo.ocean.GrandSearch.SetVisible boolean:true`
fi

