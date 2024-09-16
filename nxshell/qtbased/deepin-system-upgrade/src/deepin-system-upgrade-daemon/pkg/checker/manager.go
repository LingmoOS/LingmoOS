// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// Check system environment
package checker

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"

	"deepin-system-upgrade-daemon/pkg/module/atomic"
	"deepin-system-upgrade-daemon/pkg/module/dbustools"
	"deepin-system-upgrade-daemon/pkg/module/disk"
	"deepin-system-upgrade-daemon/pkg/module/systeminfo"
	"deepin-system-upgrade-daemon/pkg/module/user"
)

var logger = log.NewLogger("deepin-system-upgrade/checker")

// DBus path and DBus interface
const (
	dbusPath      = "/org/deepin/SystemUpgrade1/Checker"
	dbusInterface = "org.deepin.SystemUpgrade1.Checker"
)

// Logo check phase
const (
	PreUpgrade = iota
	PostUpgrade
)

// Configuration file for obtaining system version information
const (
	SYSTEM_VERSION_FILE = "/etc/os-version"
)

type CheckerManager struct {
	service *dbusutil.Service

	methods *struct {
		Perform func() `in:"phase"`
	}

	signals *struct {
		CheckResult struct {
			result     map[string]string
			percentage uint64
		}

		CheckProgressValue struct {
			value int64
		}
	}
}

func (*CheckerManager) GetInterfaceName() string {
	return dbusInterface
}

func newManager(service *dbusutil.Service) *CheckerManager {
	m := &CheckerManager{
		service: service,
	}
	return m
}

// System upgrade condition check interface, with two stages of check function before upgrade and after upgrade
func (c *CheckerManager) Perform(sender dbus.Sender, phase uint64) *dbus.Error {
	var checkResult = make(map[string]string)
	if phase == PreUpgrade {
		homeDir, err := user.GetHomeDirBySender(sender)
		if err != nil {
			return dbusutil.ToError(err)
		}
		err = c.emitProgressValue(10)
		if err != nil {
			logger.Warning(err)
		}

		// Clean up files left over from the last upgrade
		err = c.checkSystemEnv(homeDir)
		if err != nil {
			logger.Warning(err)
		}

		// Get current system version info
		value, err := systeminfo.ParseConfigFieldValue(SYSTEM_VERSION_FILE, "EditionName")
		if err != nil {
			logger.Warning("failed to parse field value:", err)
		}
		checkResult["editionName"] = value
		value, err = systeminfo.ParseConfigFieldValue(SYSTEM_VERSION_FILE, "MinorVersion")
		if err != nil {
			logger.Warning("failed to parse field value:", err)
		}
		checkResult["minorVersion"] = value
		err = c.emitProgressValue(30)
		if err != nil {
			logger.Warning(err)
		}

		// Get current system activate info
		var state string
		reply, err := dbustools.GetProperty("com.deepin.license", "com.deepin.license.Info", "AuthorizationState", "/com/deepin/license/Info")
		if err != nil {
			logger.Warning("failed to get system activate status")
			state = "0"
		} else {
			state = strconv.Itoa(int(reply.Value().(int32)))
		}
		checkResult["active"] = state
		err = c.emitProgressValue(60)
		if err != nil {
			logger.Warning(err)
		}

		// Get hardware architecture info
		arch, err := exec.Command("/usr/bin/uname", "-m").CombinedOutput()
		if err != nil {
			logger.Warning("failed to get cpu architecture:", err)
		}
		checkResult["arch"] = strings.Trim(string(arch), "\n")
		err = c.emitProgressValue(70)
		if err != nil {
			logger.Warning(err)
		}

		// Calculate remaining disk space
		checkResult, err = disk.CheckDiskSpace(checkResult)
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
		err = c.emitProgressValue(90)
		if err != nil {
			logger.Warning(err)
		}
		err = c.emitCheckResult(checkResult, 20)
		if err != nil {
			return dbusutil.ToError(err)
		}
		err = c.emitProgressValue(100)
		if err != nil {
			logger.Warning(err)
		}

	} else {

	}
	return nil
}

func (c *CheckerManager) checkSystemEnv(homeDir string) error {
	out, err := exec.Command("/usr/bin/sh", "-c", "pgrep -f -l deepin-upgrade-manager | awk -F ' ' '{print $1}'").Output()
	if err == nil {
		if len(out) != 0 {
			pids := strings.Split(string(out), "\n")
			for _, pid := range pids {
				if pid == "" {
					continue
				}
				cmdArgs := fmt.Sprintf("/usr/bin/tail --pid=%s -f /dev/null", pid)
				logger.Warning("print cmd arg:", cmdArgs)
				out, err = exec.Command("/usr/bin/sh", "-c", cmdArgs).Output() // #nosec G204
				if err != nil {
					logger.Warning("failed to monitor atomic upgrade process:", string(out))
				}
			}
		}
	} else {
		logger.Warning("failed to pgrep atomic upgrade process:", string(out))
	}

	out, err = exec.Command("/usr/bin/sh", "-c", "mount |grep extract/live/squashfs-root").CombinedOutput()
	if err != nil {
		logger.Warning("failed to get mount info")
	}
	mountInfos := strings.Split(string(out), "\n")
	if len(mountInfos) != 0 {
		for _, mount := range mountInfos {
			mp := strings.Split(mount, " ")
			if len(mp) > 2 {
				logger.Warning("print mount point:", mp[2])
				_, err := exec.Command("/usr/bin/umount", mp[2]).CombinedOutput()
				if err != nil {
					logger.Warning("failed to umount:", mp)
				}
			}
		}
	}
	extractPath := filepath.Join(homeDir, ".cache/extract")
	if _, err := os.Stat(extractPath); err == nil {
		err = os.RemoveAll(extractPath)
		if err != nil {
			logger.Warning("failed to remove extract:", err)
			return err
		}
	}
	c.emitProgressValue(20)
	repo, err := atomic.GetOStreeRepoInfo("TARGET")
	if err != nil {
		logger.Warning(err)
		return err
	}
	osroot := filepath.Join(repo, "osroot")
	if _, err := os.Stat(osroot); err == nil {
		err = os.RemoveAll(osroot)
		if err != nil {
			logger.Warning("failed to remove osroot:", err)
			return err
		}
	}
	return nil
}

func (c *CheckerManager) emitProgressValue(value int64) error {
	return c.service.Emit(c, "CheckProgressValue", value)
}

func (c *CheckerManager) emitCheckResult(result map[string]string, percentage uint64) error {
	return c.service.Emit(c, "CheckResult", result, percentage)
}
