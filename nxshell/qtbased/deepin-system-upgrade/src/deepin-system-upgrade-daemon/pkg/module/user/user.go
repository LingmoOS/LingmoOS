// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package user

import (
	"fmt"
	"os/user"

	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

var logger = log.NewLogger("deepin-system-upgrade/module/user")

func GetHomeDirBySender(sender dbus.Sender) (string, error) {
	service, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Warning("failed to get dbus service:", err)
		return "", err
	}
	uid, err := service.GetConnUID(string(sender))
	if err != nil {
		return "", err
	}
	user, err := user.LookupId(fmt.Sprint(uid))
	return user.HomeDir, nil
}
