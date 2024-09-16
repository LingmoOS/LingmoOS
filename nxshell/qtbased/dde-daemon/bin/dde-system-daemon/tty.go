// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/keyfile"
	"github.com/linuxdeepin/go-lib/procfs"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const clearData = "\033\143\n"

func (*Daemon) ClearTtys() *dbus.Error {
	for _, tty := range getValidTtys() {
		err := clearTtyAux(tty)
		if err != nil {
			return dbusutil.ToError(err)
		}
	}
	return nil
}

func (*Daemon) ClearTty(number uint32) *dbus.Error {
	err := clearTty(number)
	if err != nil {
		logger.Warning(err)
	}
	return dbusutil.ToError(err)
}

func getValidTtys() (res []string) {
	output, err := exec.Command("w", "-h", "-s").Output()
	if err != nil {
		logger.Error(err)
		return nil
	}
	lines := strings.Split(string(output), "\n")
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}
		context := strings.Fields(line)
		if len(context) > 2 && strings.Contains(context[1], "tty") {
			res = append(res, context[1])
		}
	}
	return res
}

func clearTty(number uint32) error {
	file := fmt.Sprintf("tty%d", number)
	// 判断是否是有效tty, 无效直接返回"this tty number is not exist"
	ttys := getValidTtys()
	for _, tty := range ttys {
		if file == tty {
			return clearTtyAux(file)
		}
	}
	return errors.New("this tty number is not exist")
}

func clearTtyAux(file string) error {
	f, err := os.OpenFile(filepath.Join("/dev/", file), os.O_APPEND|os.O_WRONLY, 0600)
	if err != nil {
		logger.Error("clearTtyAux:", err)
		return err
	}
	defer func() {
		_ = f.Close()
	}()

	_, err = f.WriteString(clearData)
	if err != nil {
		logger.Error("clearTtyAux:", err)
	}
	return err
}

const (
	logindConfDir               = "/etc/systemd/logind.conf.d"
	ddeLogindConf               = "deepin-daemon.conf"
	systemdLogindConfigSection  = "Login"
	systemdLogindConfigNAutoVTs = "NAutoVTs"
)

// SetLogindTTY
// NAutoVTs:默认最多可以自动启动多少个终端;若设为"0"则表示禁止自动启动任何虚拟终端,也就是禁止自动从 autovt@.service 模版实例化.
// resetCustom:重置自定义设置
// live:实时生效tty控制
func (d *Daemon) SetLogindTTY(sender dbus.Sender, NAutoVTs int, resetCustom bool, live bool) *dbus.Error {
	// checkAuth root进程和dde组件管控应用可以调用
	var cmd string
	uid, err := d.service.GetConnUID(string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}
	if uid != 0 {
		pid, err := d.service.GetConnPID(string(sender))
		if err != nil {
			return dbusutil.ToError(err)
		}
		p := procfs.Process(pid)
		cmd, err = p.Exe()
		if err != nil {
			// 当调用者在使用过程中发生了更新,则在获取该进程的exe时,会出现lstat xxx (deleted)此类的error,如果发生的是覆盖,则该路径依旧存在,因此增加以下判断
			pErr, ok := err.(*os.PathError)
			if ok {
				if os.IsNotExist(pErr.Err) {
					errExecPath := strings.Replace(pErr.Path, "(deleted)", "", -1)
					oldExecPath := strings.TrimSpace(errExecPath)
					if dutils.IsFileExist(oldExecPath) {
						cmd = oldExecPath
						err = nil
					}
				}
			} else {
				return dbusutil.ToError(err)
			}
		}
		if cmd != "/usr/bin/dde-component-control" {
			return dbusutil.ToError(fmt.Errorf("not allow %v call this method", cmd))
		}
	}
	// end check auth
	logger.Infof("SetLogindTTY NAutoVTs:%v resetCustom:%v live:%v from: %v", NAutoVTs, resetCustom, live, cmd)
	ddeLogindConfPath := filepath.Join(logindConfDir, ddeLogindConf)
	if resetCustom {
		err := os.RemoveAll(ddeLogindConfPath)
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	} else {
		// 增加tty配置,用于禁用tty启动
		logindDDEConf := keyfile.NewKeyFile()
		logindDDEConf.SetInt(systemdLogindConfigSection, systemdLogindConfigNAutoVTs, NAutoVTs)
		err := logindDDEConf.SaveToFile(ddeLogindConfPath)
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	}
	if live {
		_, err := d.systemd.RestartUnit(0, "systemd-logind.service", "replace")
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	}
	return nil
}
