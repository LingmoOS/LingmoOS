// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package mdbus

import (
	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"

	"deepin-system-upgrade-daemon/pkg/application"
	"deepin-system-upgrade-daemon/pkg/checker"
	"deepin-system-upgrade-daemon/pkg/iso"
	"deepin-system-upgrade-daemon/pkg/module/config"
	"deepin-system-upgrade-daemon/pkg/version"
)

var (
	dbusServiceName = "org.deepin.SystemUpgrade1"
	dbusPath        = "/org/deepin/SystemUpgrade1"
	dbusInterface   = dbusServiceName
)

var _upgradeManager *UpgradeManager

func Export() {
	service, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Warning("failed to get session service:", err)
	}

	_upgradeManager, err = newUpgradeManager(service)
	if err != nil {
		logger.Warning("failed to init upgrade manager:", err)
	}

	err = service.Export(dbus.ObjectPath(dbusPath), _upgradeManager)
	if err != nil {
		logger.Warning("failed to export dbus service:", err)
	}

	checker.ExportChecker(service)
	application.ExportAppManager(service)
	iso.ExportIsoManager(service)
	version.ExportVersionManager(service)
	config.ExportConfigManager(service)

	err = service.RequestName(dbusServiceName)
	if err != nil {
		logger.Warning("failed to  request name:", err)
	}
}

func NotExport() {
	service, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Warning("failed to get session service:", err)
	}
	err = service.ReleaseName(dbusServiceName)
	if err != nil {
		logger.Warning("failed to release name:", err)
	}
}
