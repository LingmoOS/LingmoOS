// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"github.com/linuxdeepin/dde-daemon/network1/nm"
)

func initSettingSectionIpv6(data connectionData) {
	addSetting(data, nm.NM_SETTING_IP6_CONFIG_SETTING_NAME)
	setSettingIP6ConfigMethod(data, nm.NM_SETTING_IP6_CONFIG_METHOD_AUTO)
}
