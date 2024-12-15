#!/bin/bash

count=$(ps -ef | grep lingmo-camera | grep -w -c $USER)
echo $count
if [ $count -ge 2 ]
then
	echo "close lingmo-camera"
	killall lingmo-camera
else
	echo "open lingmo-camera"
	dbus-send --session --print-reply --dest=com.lingmo.SessionManager /com/lingmo/StartManager com.lingmo.StartManager.Launch string:/usr/share/applications/lingmo-camera.desktop
fi

