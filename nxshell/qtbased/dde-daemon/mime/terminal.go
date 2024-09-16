// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package mime

import (
	"fmt"
	"os/exec"
	"strings"

	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/appinfo/desktopappinfo"
	"github.com/linuxdeepin/go-lib/strv"
)

const (
	gsSchemaDefaultTerminal = "com.deepin.desktop.default-applications.terminal"
	gsKeyExec               = "exec"
	gsKeyExecArg            = "exec-arg"
	gsKeyAppId              = "app-id"

	categoryTerminalEmulator = "TerminalEmulator"
	execXTerminalEmulator    = "x-terminal-emulator"
	desktopExt               = ".desktop"
)

// ignore quake-style terminal emulator
var termBlackList = strv.Strv{
	"guake",
	"tilda",
	"org.kde.yakuake",
	"qterminal_drop",
	"Terminal",
}

func resetTerminal() {
	settings := gio.NewSettings(gsSchemaDefaultTerminal)

	settings.Reset(gsKeyExec)
	settings.Reset(gsKeyExecArg)
	settings.Reset(gsKeyAppId)

	settings.Unref()
}

// readonly
var execArgMap = map[string]string{
	"gnome-terminal": "-x",
	"mate-terminal":  "-x",
	"terminator":     "-x",
	"xfce4-terminal": "-x",

	//"deepin-terminal": "-e",
	//"xterm":  "-e",
	//"pterm":  "-e",
	//"uxterm": "-e",
	//"rxvt": "-e",
	//"urxvt": "-e",
	//"rxvt-unicode": "-e",
	//"konsole": "-e",
	//"roxterm": "-e",
	//"lxterminal": "-e",
	//"terminology": "-e",
	//"sakura": "-e",
	//"evilvte": "-e",
	//"qterminal": "-e",
	//"termit": "-e",
	//"vala-terminal": "-e",
}

var terms = []string{
	"deepin-terminal",
	"gnome-terminal",
	"terminator",
	"xfce4-terminal",
	"rxvt",
	"xterm",
}

func GetPresetTerminalPath() string {
	for _, exe := range terms {
		file, _ := exec.LookPath(exe)
		if file != "" {
			return file
		}
	}
	return ""
}

func getExecArg(exec string) string {
	execArg := execArgMap[exec]
	if execArg != "" {
		return execArg
	}
	return "-e"
}

func setDefaultTerminal(id string) error {
	settings := gio.NewSettings(gsSchemaDefaultTerminal)
	defer settings.Unref()

	for _, info := range getTerminalInfos() {
		if info.Id == id {
			exec := strings.Split(info.Exec, " ")[0]
			settings.SetString(gsKeyExec, exec)
			settings.SetString(gsKeyExecArg, getExecArg(exec))

			id = strings.TrimSuffix(id, desktopExt)
			settings.SetString(gsKeyAppId, id)
			return nil
		}
	}
	return fmt.Errorf("invalid terminal id '%s'", id)
}

func getDefaultTerminal() (*AppInfo, error) {
	settings := gio.NewSettings(gsSchemaDefaultTerminal)
	appId := settings.GetString(gsKeyAppId)
	// add suffix .desktop
	if !strings.HasSuffix(appId, desktopExt) {
		appId = appId + desktopExt
	}
	settings.Unref()
	list := getTerminalInfos()
	// gs获取默认程序
	for _, info := range list {
		if info.Id == appId {
			return info, nil
		}
	}
	// gs没有,用预设默认程序
	for _, term := range terms {
		for _, info := range list {
			if info.Exec == term {
				return info, nil
			}
		}
	}

	return nil, fmt.Errorf("not found app id for %q", appId)
}

func getTerminalInfos() AppInfos {
	appInfoList := desktopappinfo.GetAll(nil)

	var list AppInfos
	for _, appInfo := range appInfoList {
		if !isTerminalApp(appInfo) {
			continue
		}

		name := getAppName(appInfo)
		var tmp = &AppInfo{
			Id:          appInfo.GetId() + desktopExt,
			Name:        name,
			DisplayName: name,
			Description: appInfo.GetComment(),
			Exec:        appInfo.GetCommandline(),
			Icon:        appInfo.GetIcon(),
			fileName:    appInfo.GetFileName(),
		}
		list = append(list, tmp)
	}
	return list
}

func isTerminalApp(appInfo *desktopappinfo.DesktopAppInfo) bool {
	if termBlackList.Contains(appInfo.GetId()) {
		return false
	}

	categories := appInfo.GetCategories()
	if !strv.Strv(categories).Contains(categoryTerminalEmulator) {
		return false
	}

	exec := appInfo.GetCommandline()
	return !strings.Contains(exec, execXTerminalEmulator)
}

func isStrInList(s string, list []string) bool {
	for _, v := range list {
		if v == s {
			return true
		}
	}

	return false
}
