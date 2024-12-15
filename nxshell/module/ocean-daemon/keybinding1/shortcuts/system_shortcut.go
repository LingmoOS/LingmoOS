// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"encoding/json"
	"os"
	"path"
	"sync"

	"github.com/adrg/xdg"
	dutils "github.com/lingmoos/go-lib/utils"
)

const (
	// Under '/usr/share' or '/usr/local/share'
	systemActionsFile   = "ocean-daemon/keybinding/system_actions.json"
	screenshotCmdPrefix = "dbus-send --print-reply --dest=com.lingmo.Screenshot /com/lingmo/Screenshot com.lingmo.Screenshot."
)

type SystemShortcut struct {
	*GSettingsShortcut
	arg *ActionExecCmdArg
}

func (ss *SystemShortcut) SetName(name string) error {
	return ErrOpNotSupported
}

func (ss *SystemShortcut) GetAction() *Action {
	return &Action{
		Type: ActionTypeExecCmd,
		Arg:  ss.arg,
	}
}

func (ss *SystemShortcut) SetAction(newAction *Action) error {
	if newAction == nil {
		return ErrNilAction
	}
	if newAction.Type != ActionTypeExecCmd {
		return ErrInvalidActionType
	}

	arg, ok := newAction.Arg.(*ActionExecCmdArg)
	if !ok {
		return ErrTypeAssertionFail
	}
	ss.arg = arg
	return nil
}

var loadSysActionsFileOnce sync.Once
var actionsCache *actionHandler

func GetSystemActionCmd(id string) string {
	return getSystemActionCmd(id)
}

func getSystemActionCmd(id string) string {
	loadSysActionsFileOnce.Do(func() {
		file := getSystemActionsFile()
		actions, err := loadSystemActionsFile(file)
		if err != nil {
			logger.Warning("failed to load system actions file:", err)
			return
		}
		actionsCache = actions
	})

	if actionsCache != nil {
		if cmd, ok := actionsCache.getCmd(id); ok {
			return cmd
		}
	}
	if id == "lock-screen" && _useWayland {
		id = "lock-screen-wayland"
	}
	return defaultSysActionCmdMap[id]
}

// key is id, value is commandline.
var defaultSysActionCmdMap = map[string]string{
	"launcher":       "dbus-send --print-reply --dest=org.lingmo.ocean.Launcher1 /org/lingmo/ocean/Launcher1 org.lingmo.ocean.Launcher1.Toggle",
	"terminal":       "/usr/lib/lingmo-daemon/default-terminal",
	"terminal-quake": "ocean-am lingmo-terminal quake-mode",
	"lock-screen":    "originmap=$(setxkbmap -query | grep option | awk -F ' ' '{print $2}');/usr/bin/setxkbmap -option grab:break_actions&&/usr/bin/xdotool key XF86Ungrab&&dbus-send --print-reply --dest=org.lingmo.ocean.LockFront1 /org/lingmo/ocean/LockFront1 org.lingmo.ocean.LockFront1.Show&&/usr/bin/setxkbmap -option $originmap",
	//wayland不能设置XF86Ungrab，否则会导致Bug-224309
	"lock-screen-wayland":    "originmap=$(setxkbmap -query | grep option | awk -F ' ' '{print $2}');/usr/bin/setxkbmap -option grab:break_actions&&dbus-send --print-reply --dest=org.lingmo.ocean.LockFront1 /org/lingmo/ocean/LockFront1 org.lingmo.ocean.LockFront1.Show&&/usr/bin/setxkbmap -option $originmap",
	"logout":                 "dbus-send --print-reply --dest=org.lingmo.ocean.ShutdownFront1 /org/lingmo/ocean/ShutdownFront1 org.lingmo.ocean.ShutdownFront1.Show",
	"lingmo-screen-recorder": "dbus-send --print-reply --dest=com.lingmo.ScreenRecorder /com/lingmo/ScreenRecorder com.lingmo.ScreenRecorder.stopRecord",
	"system-monitor":         "/usr/bin/lingmo-system-monitor",
	"color-picker":           "dbus-send --print-reply --dest=com.lingmo.Picker /com/lingmo/Picker com.lingmo.Picker.Show",
	// screenshot actions:
	"screenshot":             screenshotCmdPrefix + "StartScreenshot",
	"screenshot-fullscreen":  screenshotCmdPrefix + "FullscreenScreenshot",
	"screenshot-window":      screenshotCmdPrefix + "TopWindowScreenshot",
	"screenshot-delayed":     screenshotCmdPrefix + "DelayScreenshot int64:5",
	"screenshot-ocr":         screenshotCmdPrefix + "OcrScreenshot",
	"screenshot-scroll":      screenshotCmdPrefix + "ScrollScreenshot",
	"file-manager":           "/usr/lib/lingmo-daemon/default-file-manager",
	"disable-touchpad":       "gsettings set com.lingmo.ocean.touchpad touchpad-enabled false",
	"wm-switcher":            "dbus-send --type=method_call --dest=org.lingmo.ocean.WMSwitcher1 /org/lingmo/ocean/WMSwitcher1 org.lingmo.ocean.WMSwitcher1.RequestSwitchWM",
	"turn-off-screen":        "sleep 0.5; xset dpms force off",
	"notification-center":    "dbus-send --print-reply --dest=org.lingmo.ocean.Osd1 /org/freedesktop/ocean/Notifications org.lingmo.ocean.Notification1.Toggle",
	"clipboard":              "dbus-send --print-reply --dest=org.lingmo.ocean.Clipboard1 /org/lingmo/ocean/Clipboard1 org.lingmo.ocean.Clipboard1.Toggle; dbus-send --print-reply --dest=org.lingmo.ocean.Launcher1 /org/lingmo/ocean/Launcher1 org.lingmo.ocean.Launcher1.Hide",
	"global-search":          "/usr/libexec/ocean-daemon/keybinding/shortcut-ocean-grand-search.sh",
	"switch-next-kbd-layout": "dbus-send --print-reply --dest=org.lingmo.ocean.Keybinding1 /org/lingmo/ocean/InputDevice1/Keyboard org.lingmo.ocean.InputDevice1.Keyboard.ToggleNextLayout",
	"switch-monitors":        "dbus-send --print-reply --dest=org.lingmo.ocean.Osd1 / org.lingmo.ocean.Osd1.ShowOSD string:SwitchMonitors",
	// cmd
	"calculator": "/usr/bin/lingmo-calculator",
	"search":     "/usr/libexec/ocean-daemon/keybinding/shortcut-ocean-grand-search.sh",
	"script":     "/usr/libexec/ocean-daemon/keybinding/shortcut-ocean-script.sh",
}

type actionHandler struct {
	Actions []struct {
		Key    string `json:"Key"`
		Action string `json:"Action"`
	} `json:"Actions"`
}

func (a *actionHandler) getCmd(id string) (cmd string, ok bool) {
	for _, v := range a.Actions {
		if v.Key == id {
			return v.Action, true
		}
	}
	return "", false
}

func loadSystemActionsFile(file string) (*actionHandler, error) {
	logger.Debug("load system action file:", file)

	// #nosec G304
	content, err := os.ReadFile(file)
	if err != nil {
		return nil, err
	}

	var handler actionHandler
	err = json.Unmarshal(content, &handler)
	if err != nil {
		return nil, err
	}

	return &handler, nil
}

func getSystemActionsFile() string {
	var file = path.Join("/usr/local/share", systemActionsFile)
	if dutils.IsFileExist(file) {
		return file
	}

	file = path.Join("/usr/share", systemActionsFile)
	if dutils.IsFileExist(file) {
		return file
	}

	filepath, _ := xdg.SearchDataFile(systemActionsFile)
	return filepath
}
