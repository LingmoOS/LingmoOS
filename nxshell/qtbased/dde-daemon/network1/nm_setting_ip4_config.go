// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"github.com/linuxdeepin/dde-daemon/network1/nm"
)

func initSettingSectionIpv4(data connectionData) {
	addSetting(data, nm.NM_SETTING_IP4_CONFIG_SETTING_NAME)
	setSettingIP4ConfigMethod(data, nm.NM_SETTING_IP4_CONFIG_METHOD_AUTO)
	setSettingIP4ConfigNeverDefault(data, false)
}
