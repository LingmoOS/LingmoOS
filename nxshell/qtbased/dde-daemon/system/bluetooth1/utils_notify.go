// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	btcommon "github.com/linuxdeepin/dde-daemon/common/bluetooth"
)

const (
	notifyIconBluetoothConnected     = "notification-bluetooth-connected"
	notifyIconBluetoothDisconnected  = "notification-bluetooth-disconnected"
	notifyIconBluetoothConnectFailed = "notification-bluetooth-error"
)

func notify(icon string, summary, body *btcommon.LocalizeStr) {
	logger.Info("notify", icon, summary, body)

	args := marshalJSON(btcommon.NotifyMsg{
		Icon:    icon,
		Summary: summary,
		Body:    body,
	})
	ua := _bt.getActiveUserAgent()
	if ua == nil {
		logger.Debug("ua is nil")
		return
	}
	err := ua.SendNotify(0, args)
	if err != nil {
		logger.Warning("send notify err:", err)
	}
}

// Tr 真正的翻译在session级别的模块
func Tr(msgId string) string {
	return msgId
}

func notifyConnected(alias string) {
	notify(notifyIconBluetoothConnected, nil, &btcommon.LocalizeStr{
		Format: Tr("Connect %q successfully"),
		Args:   []string{alias},
	})
}

func notifyDisconnected(alias string) {
	notify(notifyIconBluetoothDisconnected, nil, &btcommon.LocalizeStr{
		Format: Tr("%q disconnected"),
		Args:   []string{alias},
	})
}

func notifyConnectFailedHostDown(alias string) {
	format := Tr("Make sure %q is turned on and in range")
	notifyConnectFailedAux(alias, format)
}

func notifyConnectFailedAux(alias, format string) {
	notify(notifyIconBluetoothConnectFailed,
		&btcommon.LocalizeStr{ // summary
			Format: Tr("Bluetooth connection failed"),
		}, &btcommon.LocalizeStr{ // body
			Format: format,
			Args:   []string{alias},
		})
}

func notifyConnectFailedResourceUnavailable(devAlias, adapterAlias string) {
	notify(notifyIconBluetoothConnectFailed, &btcommon.LocalizeStr{ // summary
		Format: Tr("Bluetooth connection failed"),
	}, &btcommon.LocalizeStr{ //body
		Format: Tr("%q can no longer connect to %q. Try to forget this device and pair it again."),
		Args:   []string{adapterAlias, devAlias},
	})
}
