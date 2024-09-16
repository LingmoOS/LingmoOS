// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package notify

import (
	"deepin-upgrade-manager/pkg/module/util"

	"github.com/godbus/dbus/v5"
)

var msgRollBack = util.Tr("System Recovery")

const (
	NodifydbusDest      = "com.deepin.dde.Notification"
	NodifydbusPath      = "/com/deepin/dde/Notification"
	NodifydbusInterface = NodifydbusDest
)

const (
	NodifydbusDestV23      = "org.deepin.dde.Notification1"
	NodifydbusPathV23      = "/org/deepin/dde/Notification1"
	NodifydbusInterfaceV23 = NodifydbusDestV23
)

func SetNotifyText(text string) error {
	sysBus, err := dbus.SessionBus()
	if err != nil {
		return err
	}
	notifyServiceObj := sysBus.Object(NodifydbusDest,
		NodifydbusPath)
	metho := NodifydbusInterface + ".Notify"
	var arg0 string
	var arg1 uint32
	var arg2 string
	var arg3 string
	var arg4 string
	var arg5 []string
	var map_variable map[string]dbus.Variant
	var arg7 int32
	arg0, _ = util.GetBootKitText(msgRollBack, []string{})
	arg1 = 101
	arg2 = "preferences-system"
	arg3 = text
	arg7 = 10000
	// using deepin clone icon
	if util.IsExists("/usr/share/deepin-clone") {
		arg2 = "deepin-clone"
	}
	err = notifyServiceObj.Call(metho, 0, arg0, arg1, arg2, arg3, arg4, arg5, map_variable, arg7).Store()
	if err != nil {
		notifyServiceObj = sysBus.Object(NodifydbusDestV23,
			NodifydbusPathV23)
		metho = NodifydbusInterfaceV23 + ".Notify"
		err = notifyServiceObj.Call(metho, 0, arg0, arg1, arg2, arg3, arg4, arg5, map_variable, arg7).Store()
	}
	return err
}
