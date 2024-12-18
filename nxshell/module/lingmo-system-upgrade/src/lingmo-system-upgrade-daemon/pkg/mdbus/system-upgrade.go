// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package mdbus

import (
	"path/filepath"
	"syscall"

	"github.com/godbus/dbus"
	"github.com/lingmoos/go-lib/dbusutil"
	"github.com/lingmoos/go-lib/log"

	"lingmo-system-upgrade-daemon/pkg/iso"
	"lingmo-system-upgrade-daemon/pkg/module/user"
)

var logger = log.NewLogger("lingmo-system-upgrade/dbus")

type UpgradeManager struct {
	service *dbusutil.Service

	methods *struct {
		StopUpgrade func()
	}

	signals *struct {
	}
}

func (*UpgradeManager) GetInterfaceName() string {
	return dbusInterface
}

func newUpgradeManager(service *dbusutil.Service) (*UpgradeManager, error) {
	u := &UpgradeManager{
		service: service,
	}
	return u, nil
}

func (u *UpgradeManager) StopUpgrade(sender dbus.Sender) *dbus.Error {
	homeDir, err := user.GetHomeDirBySender(sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	iso.UmountSystemDir(filepath.Join(homeDir, ".cache/extract/live/squashfs-root"))
	iso.UmountSystemDir("/")
	logger.Warning("receive stop signal, stop upgrade daemon!")
	defer syscall.Exit(0)
	return nil
}
