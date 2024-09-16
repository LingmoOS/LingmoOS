#!/bin/bash

registered=`dbus-send --session --print-reply --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.NameHasOwner string:com.deepin.dde.GrandSearch | grep true`

if [ "$registered" ]
then
	visible=`dbus-send --session --print-reply=literal --dest=com.deepin.dde.GrandSearch /com/deepin/dde/GrandSearch com.deepin.dde.GrandSearch.IsVisible | grep true`
	if [ "$visible" ]
	then
		`dbus-send --session --print-reply=literal --dest=com.deepin.dde.GrandSearch /com/deepin/dde/GrandSearch com.deepin.dde.GrandSearch.SetVisible boolean:false`
	else
		`dbus-send --session --print-reply=literal --dest=com.deepin.dde.GrandSearch /com/deepin/dde/GrandSearch com.deepin.dde.GrandSearch.SetVisible boolean:true`
	fi
else
`dbus-send --session --print-reply=literal --dest=com.deepin.dde.GrandSearch /com/deepin/dde/GrandSearch com.deepin.dde.GrandSearch.SetVisible boolean:true`
fi

