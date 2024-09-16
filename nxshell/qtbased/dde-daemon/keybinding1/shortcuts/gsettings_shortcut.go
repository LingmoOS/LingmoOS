// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
	"github.com/linuxdeepin/go-gir/gio-2.0"
)

type GSettingsShortcut struct {
	BaseShortcut
	gsettings *gio.Settings
	wm        wm.Wm
}

func NewGSettingsShortcut(gsettings *gio.Settings, wm wm.Wm, id string, type0 int32,
	keystrokes []string, name string) *GSettingsShortcut {
	gs := &GSettingsShortcut{
		BaseShortcut: BaseShortcut{
			Id:         id,
			Type:       type0,
			Keystrokes: ParseKeystrokes(keystrokes),
			Name:       name,
		},
		gsettings: gsettings,
		wm:        wm,
	}

	return gs
}

func (gs *GSettingsShortcut) SaveKeystrokes() error {
	keystrokesStrv := make([]string, 0, len(gs.Keystrokes))
	for _, ks := range gs.Keystrokes {
		keystrokesStrv = append(keystrokesStrv, ks.String())
	}
	if _useWayland {
		ok, err := setShortForWayland(gs, gs.wm)
		if !ok {
			return err
		}
	}
	gs.gsettings.SetStrv(gs.Id, keystrokesStrv)
	gio.SettingsSync()
	logger.Debugf("GSettingsShortcut.SaveKeystrokes id: %v, keystrokes: %v", gs.Id, keystrokesStrv)
	return nil
}

func keystrokesEqual(s1 []*Keystroke, s2 []*Keystroke) bool {
	l1 := len(s1)
	l2 := len(s2)

	if l1 != l2 {
		return false
	}

	for i := 0; i < l1; i++ {
		ks1 := s1[i]
		ks2 := s2[i]

		if ks1.String() != ks2.String() {
			return false
		}
	}
	return true
}

func (gs *GSettingsShortcut) ReloadKeystrokes() bool {
	oldVal := gs.GetKeystrokes()
	id := gs.GetId()
	newVal := ParseKeystrokes(gs.gsettings.GetStrv(id))
	gs.setKeystrokes(newVal)
	return !keystrokesEqual(oldVal, newVal)
}
