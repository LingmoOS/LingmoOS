#!/bin/bash

# SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-only

# enableVerboseLogging, Enable verbose logging information output
dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.enableVerboseLogging

dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.setDelayReleaseTime int32:1000

echo delayReleaseTime: $(dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.delayReleaseTime)

#acquireManager
DCONFIG_RESOURCE_PATH=$(dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.acquireManager string:'dconfig-example' string:'example' string:'')

#acquireManagerV2
USERID=$(id -u)
DCONFIG_RESOURCE_PATH=$(dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.acquireManagerV2 uint32:$USERID, string:'dconfig-example' string:'example' string:'')

#object path
echo path: $DCONFIG_RESOURCE_PATH

#monitor valueChanged
dbus-monitor --system "type='signal', interface='org.desktopspec.ConfigManager.Manager',member=valueChanged" &

#description
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.description string:'canExit' string:''

#name
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.name string:'canExit' string:''

dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.name string:'canExit' string:'zh_CN'

#visibility
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.visibility string:'canExit'

#permissions
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.permissions string:'canExit'

#flags
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.flags string:'canExit'


#value
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.value string:'canExit'


#setValue
PROPERTY_VALUE=$(dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.value string:'canExit')
if [[ "$PROPERTY_VALUE" = *true ]];then
  dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.setValue string:'canExit' variant:boolean:false
else
  dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.setValue string:'canExit' variant:boolean:true
fi

#reset
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.reset string:'canExit'

#value
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.value string:'canExit'

#isDefaultValue
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.isDefaultValue string:'canExit'

#release
# dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.release

#update
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.update string:'/usr/share/dsg/configs/dconfig-example/example.json'

#sync
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.sync string:'/usr/share/dsg/configs/dconfig-example/example.json'

# disableVerboseLogging, Disable verbose logging information output
dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.disableVerboseLogging

# killall dbus-monitor
# kill $DBUS_MONITOR_PID

#acquireManagerV2
DCONFIG_RESOURCE_PATH_BY_UID=$(dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager / org.desktopspec.ConfigManager.acquireManagerV2 uint32:$USERID, string:'dconfig-example' string:'example' string:'')

#object path
echo $DCONFIG_RESOURCE_PATH_BY_UID

#value
dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.value string:'canExit'

#setValue
PROPERTY_VALUE=$(dbus-send --system --type=method_call --print-reply=literal --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.value string:'canExit')
if [[ "$PROPERTY_VALUE" = *true ]];then
  dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.setValue string:'canExit' variant:boolean:false
else
  dbus-send --system --type=method_call --print-reply --dest=org.desktopspec.ConfigManager $DCONFIG_RESOURCE_PATH org.desktopspec.ConfigManager.Manager.setValue string:'canExit' variant:boolean:true
fi

jobs -p |xargs kill 
