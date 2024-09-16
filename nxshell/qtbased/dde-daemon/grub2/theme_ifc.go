// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strings"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/utils"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const (
	themeDBusPath      = dbusPath + "/Theme"
	themeDBusInterface = dbusInterface + ".Theme"
)

func (*Theme) GetInterfaceName() string {
	return themeDBusInterface
}

func (theme *Theme) SetBackgroundSourceFile(sender dbus.Sender, filename string) *dbus.Error {
	err := checkInvokePermission(theme.service, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	theme.service.DelayAutoQuit()

	logger.Debugf("SetBackgroundSourceFile: %q", filename)
	err = theme.g.checkAuth(sender, polikitActionIdCommon)
	if err != nil {
		return dbusutil.ToError(err)
	}

	filename = utils.DecodeURI(filename)
	cmd := exec.Command(adjustThemeCmd, "-set-background", filename)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	err = cmd.Run()
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	theme.emitSignalBackgroundChanged()
	return nil
}

func (theme *Theme) GetBackground(sender dbus.Sender) (background string, busErr *dbus.Error) {
	err := checkInvokePermission(theme.service, sender)
	if err != nil {
		return "", dbusutil.ToError(err)
	}
	theme.service.DelayAutoQuit()

	theme.g.PropsMu.RLock()
	themeFile := theme.g.ThemeFile
	theme.g.PropsMu.RUnlock()

	if strings.Contains(themeFile, "/deepin/") {
		_, err := os.Stat(defaultGrubBackground)
		if err != nil {
			if os.IsNotExist(err) {
				background = fallbackGrubBackground
			} else {
				return "", dbusutil.ToError(err)
			}
		} else {
			background = defaultGrubBackground
		}
	} else if strings.Contains(themeFile, "/deepin-fallback/") {
		background = fallbackGrubBackground
	} else {
		return "", nil
	}

	if len(background) == 0 {
		return "", nil
	} else {

		tmpBackground := "dde-grub-background" + path.Ext(defaultGrubBackground)
		backGroundTmpPath := filepath.Join("/tmp", tmpBackground)
		//copy file to /tmp
		err := dutils.CopyFile(background, backGroundTmpPath)
		if err != nil {
			return "", dbusutil.ToError(err)
		}
		logger.Debugf("Copy file %s to %s", background, backGroundTmpPath)

		return backGroundTmpPath, nil
	}
}

func (theme *Theme) emitSignalBackgroundChanged() {
	err := theme.service.Emit(theme, "BackgroundChanged")
	if err != nil {
		logger.Warning(err)
	}
}
