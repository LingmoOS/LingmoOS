// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"errors"
	"os"
	"strings"

	"github.com/linuxdeepin/dde-daemon/keybinding1/util"
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
)

type kWinShortcut struct {
	BaseShortcut
	wm wm.Wm
}

func newKWinShortcut(id, name string, keystrokes []string, wm wm.Wm) *kWinShortcut {
	return &kWinShortcut{
		BaseShortcut: BaseShortcut{
			Id:         id,
			Type:       ShortcutTypeWM,
			Name:       name,
			Keystrokes: ParseKeystrokes(keystrokes),
		},
		wm: wm,
	}
}

func (ks *kWinShortcut) ReloadKeystrokes() bool {
	oldVal := ks.GetKeystrokes()
	keystrokes, err := ks.wm.GetAccel(0, ks.Id)
	if err != nil {
		logger.Warningf("failed to get accel for %s: %v", ks.Id, err)
		return false
	}
	sessionType := os.Getenv("XDG_SESSION_TYPE")
	if strings.Contains(sessionType, "wayland") {
		if len(keystrokes) != 0 && strings.Contains(keystrokes[0], "SysReq") {
			keystrokes[0] = strings.Replace(keystrokes[0], "SysReq", "<Alt>Print", 1)
		}
		//launcher
		if ks.Id == "launcher" {
			keystrokes[0] = "<Super_L>"
		}
		//system-monitor
		if ks.Id == "system-monitor" {
			for i := 0; i < len(keystrokes); i++ {
				keystrokes[i] = strings.Replace(keystrokes[i], "Esc", "Escape", 1)
			}
		}
	}
	newVal := ParseKeystrokes(keystrokes)
	ks.setKeystrokes(newVal)
	return !keystrokesEqual(oldVal, newVal)
}

func (ks *kWinShortcut) SaveKeystrokes() error {
	accelJson, err := util.MarshalJSON(util.KWinAccel{
		Id:         ks.Id,
		Keystrokes: ks.getKeystrokesStrv(),
	})
	if err != nil {
		return err
	}

	ok, err := ks.wm.SetAccel(0, accelJson)
	if !ok {
		return errors.New("wm.SetAccel failed, id: " + ks.Id)
	}
	return err
}

func (ks *kWinShortcut) ShouldEmitSignalChanged() bool {
	return true
}
