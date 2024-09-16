// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"fmt"
	"io"
	"os"
	"regexp"
	"strconv"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/keyfile"
	"github.com/linuxdeepin/go-lib/procfs"
)

//go:generate dbusutil-gen em -type Manager

const (
	dbusServiceName = "org.deepin.dde.Greeter1"
	dbusPath        = "/org/deepin/dde/Greeter1"
	dbusInterface   = dbusServiceName
)

const (
	themeFile                  = "/etc/lightdm/deepin/qt-theme.ini"
	themeSection               = "Theme"
	themeKeyIconThemeName      = "IconThemeName"
	themeKeyFontSize           = "FontSize"
	themeKeyFont               = "Font"
	themeKeyMonoFont           = "MonoFont"
	themeKeyScreenScaleFactors = "ScreenScaleFactors"
	themeKeyScaleLogicalDpi    = "ScaleLogicalDpi"
)

const (
	xsettingsFile             = "/etc/lightdm/deepin/xsettingsd.conf"
	xsettingsKeyIconThemeName = "Net/IconThemeName"
	xsettingsKeyFontSize      = "Qt/FontPointSize"
	xsettingsKeyFont          = "Qt/FontName"
	xsettingsKeyMonoFont      = "Qt/MonoFontName"
	xsettingsKeyDpi           = "Xft/DPI"
)

// theme的key
var globalKeyConvertMap = map[string]string{
	themeKeyIconThemeName: xsettingsKeyIconThemeName,
	themeKeyFontSize:      xsettingsKeyFontSize,
	themeKeyFont:          xsettingsKeyFont,
	themeKeyMonoFont:      xsettingsKeyMonoFont,
}

type Manager struct {
	service *dbusutil.Service
}

func (m *Manager) UpdateGreeterQtTheme(sender dbus.Sender, fd dbus.UnixFD) *dbus.Error {
	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		logger.Warning(err)
	} else {
		p := procfs.Process(pid)
		cmd, err := p.Exe()
		if err != nil {
			logger.Warning(err)
		} else {
			logger.Info("Calling UpdateGreeterQtTheme by ", cmd)
		}
	}

	m.service.DelayAutoQuit()
	err = updateGreeterQtTheme(fd)
	if err != nil {
		logger.Warning(err)
	}
	updateXSettingsConfig()
	return dbusutil.ToError(err)
}

func updateGreeterQtTheme(fd dbus.UnixFD) error {
	f := os.NewFile(uintptr(fd), "")
	defer f.Close()
	err := os.MkdirAll("/etc/lightdm/deepin", 0755)
	if err != nil {
		return err
	}
	const themeFileTemp = themeFile + ".tmp"
	dest, err := os.Create(themeFileTemp)
	if err != nil {
		return err
	}
	// limit file size: 100KB
	src := io.LimitReader(f, 1024*100)
	_, err = io.Copy(dest, src)
	if err != nil {
		closeErr := dest.Close()
		if closeErr != nil {
			logger.Warning(closeErr)
		}
		return err
	}

	err = dest.Close()
	if err != nil {
		return err
	}

	err = os.Rename(themeFileTemp, themeFile)
	return err
}

func updateXSettingsConfig() {
	kf := keyfile.NewKeyFile()
	err := kf.LoadFromFile(themeFile)
	if err != nil {
		logger.Warning(err)
		return
	}

	xf, err := os.OpenFile(xsettingsFile, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
	if err != nil {
		logger.Warning(err)
		return
	}
	defer xf.Close()

	// 将theme的配置转换成xsettings的配置
	for themeKey, xsettingsKey := range globalKeyConvertMap {
		value, err := kf.GetString(themeSection, themeKey)
		if err == nil {
			if isInteger(value) {
				_, err = xf.WriteString(fmt.Sprintf("%s %s\n", xsettingsKey, value))
			} else {
				_, err = xf.WriteString(fmt.Sprintf("%s \"%s\"\n", xsettingsKey, value))
			}
		}

		if err != nil {
			logger.Warning(err)
		}
	}

	// 换算缩放 xsettingsKeyDpi = themeKeyScreenScaleFactors * themeKeyScaleLogicalDpi * 1024
	themeDpiStr, err := kf.GetString(themeSection, themeKeyScaleLogicalDpi)
	if err != nil {
		logger.Warningf("cannot get DPI: %v", err)
		return
	}
	exp, err := regexp.Compile(`^([0-9]+),([0-9]+)$`)
	if err != nil {
		logger.Warning(err)
		return
	}
	substr := exp.FindStringSubmatch(themeDpiStr)
	if len(substr) < 2 {
		logger.Warning("cannot parse theme dpi")
		return
	}
	themeDpi, err := strconv.ParseUint(substr[1], 10, 32)
	if err != nil {
		logger.Warning(err)
		return
	}
	scale, err := kf.GetFloat64(themeSection, themeKeyScreenScaleFactors)
	xDpi := uint64(scale * float64(themeDpi) * 1024)
	_, err = xf.WriteString(fmt.Sprintf("%s %d\n", xsettingsKeyDpi, xDpi))
	if err != nil {
		logger.Warning(err)
	}
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func isInteger(str string) bool {
	_, err := strconv.ParseInt(str, 10, 32)
	return err == nil
}
