// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"os/exec"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	configManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

var plymouthLocker sync.Mutex

const (
	dsettingsAppID                       = "org.deepin.dde.daemon"
	dsettingsAppearanceName              = "org.deepin.dde.daemon.appearance"
	dsettingsScaleWithoutPlymouthEnabled = "scaleWithoutPlymouthEnabled"
)

func (d *Daemon) getScaleWithoutPlymouthEnabled() bool {
	ds := configManager.NewConfigManager(d.systemSigLoop.Conn())

	appearancePath, err := ds.AcquireManager(0, dsettingsAppID, dsettingsAppearanceName, "")
	if err != nil {
		logger.Warning(err)
		return false
	}

	dsAppearance, err := configManager.NewManager(d.systemSigLoop.Conn(), appearancePath)
	if err != nil {
		logger.Warning(err)
		return false
	}

	v, err := dsAppearance.Value(0, dsettingsScaleWithoutPlymouthEnabled)
	if err != nil {
		logger.Warning(err)
		return false
	}

	if enabled, ok := v.Value().(bool); ok {
		return enabled
	}

	return false
}

func (d *Daemon) ScalePlymouth(scale uint32) *dbus.Error {
	return dbusutil.ToError(d.scalePlymouth(scale))
}

func (d *Daemon) scalePlymouth(scale uint32) error {
	if d.getScaleWithoutPlymouthEnabled() {
		logger.Info("skip scale plymouth")
		return nil
	}

	plymouthLocker.Lock()
	defer plymouthLocker.Unlock()
	defer logger.Debug("end ScalePlymouth", scale)

	edition, err := getEditionName()
	if err != nil {
		return err
	}
	var themeNames map[uint32]string
	if edition == "Community" {
		themeNames = map[uint32]string{
			1: "deepin-ssd-logo",
			2: "deepin-hidpi-ssd-logo",
		}
	} else {
		themeNames = map[uint32]string{
			1: "uos-ssd-logo",
			2: "uos-hidpi-ssd-logo",
		}
	}
	name, ok := themeNames[scale]
	if !ok {
		return fmt.Errorf("invalid scale value: %d", scale)
	}
	out, err := exec.Command("plymouth-set-default-theme", name).CombinedOutput()

	if err != nil {
		return fmt.Errorf("failed to set plymouth theme: %s, err: %v", string(out), err)
	}

	kernel, err := exec.Command("uname", "-r").CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to get kernel, err: %v", err)
	}
	out, err = exec.Command("update-initramfs",
		"-u", "-k", string(bytes.TrimSpace(kernel))).CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to update initramfs: %s, err: %v", string(out), err)
	}

	return nil
}

func (d *Daemon) SetPlymouthTheme(themeName string) *dbus.Error {
	return dbusutil.ToError(d.setPlymouthTheme(themeName))
}

func (d *Daemon) setPlymouthTheme(themeName string) error {
	plymouthLocker.Lock()
	defer plymouthLocker.Unlock()
	defer logger.Debug("end ScalePlymouth", themeName)
	themelistout, err := exec.Command("plymouth-set-default-theme", "--list").CombinedOutput()
	if err != nil {
		return fmt.Errorf("seems cannot find the plymouth-set-default-theme: %v", err)
	}
	themelist := string(themelistout)
	if !strings.Contains(themelist, themeName) {
		return fmt.Errorf("The themeName %s does not exist in plymouth themelist", themeName)
	}

	out, err := exec.Command("plymouth-set-default-theme", themeName).CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to set plymouth theme: %s, err: %v", string(out), err)
	}

	kernel, err := exec.Command("uname", "-r").CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to get kernel, err: %v", err)
	}
	out, err = exec.Command("update-initramfs",
		"-u", "-k", string(bytes.TrimSpace(kernel))).CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to update initramfs: %s, err: %v", string(out), err)
	}

	return nil
}

func getEditionName() (string, error) {
	conf, err := parseInfoFile("/etc/os-version", "=")
	if err != nil {
		return "", err
	}
	value, ok := conf["EditionName"]
	if !ok {
		return "", fmt.Errorf("Can not find the EditionName")
	}
	return value, nil
}

func parseInfoFile(file, delim string) (map[string]string, error) {
	content, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}
	var ret = make(map[string]string)
	lines := strings.Split(string(content), "\n")
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}
		array := strings.Split(line, delim)
		if len(array) != 2 {
			continue
		}
		ret[strings.TrimSpace(array[0])] = strings.TrimSpace(array[1])
	}
	return ret, nil
}
