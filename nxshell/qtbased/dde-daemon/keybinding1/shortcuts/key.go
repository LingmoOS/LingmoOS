// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"fmt"
	"strings"

	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/util/keybind"
	"github.com/linuxdeepin/go-x11-client/util/keysyms"
)

type Keycode x.Keycode
type Modifiers uint16

func (mods Modifiers) String() string {
	var keys []string
	if mods&keysyms.ModMaskShift > 0 {
		keys = append(keys, "Shift")
	}
	if mods&keysyms.ModMaskCapsLock > 0 {
		keys = append(keys, "CapsLock")
	}
	if mods&keysyms.ModMaskControl > 0 {
		keys = append(keys, "Control")
	}
	if mods&keysyms.ModMaskAlt > 0 {
		keys = append(keys, "Alt")
	}
	if mods&keysyms.ModMaskNumLock > 0 {
		keys = append(keys, "NumLock")
	}
	if mods&x.ModMask3 > 0 {
		keys = append(keys, "Mod3")
	}
	if mods&keysyms.ModMaskSuper > 0 {
		keys = append(keys, "Super")
	}
	if mods&keysyms.ModMaskModeSwitch > 0 {
		keys = append(keys, "ModeSwitch")
	}
	return fmt.Sprintf("[%d|%s]", uint16(mods), strings.Join(keys, "-"))
}

type Key struct {
	Mods Modifiers
	Code Keycode
}

func (k Key) String() string {
	return fmt.Sprintf("Key<Mods=%s Code=%d>", k.Mods, k.Code)
}

func (k Key) ToKeystroke(keySymbols *keysyms.KeySymbols) *Keystroke {
	mods := k.Mods
	mods &^= keysyms.ModMaskShift
	str, ok := keySymbols.LookupString(x.Keycode(k.Code), uint16(mods))
	if !ok {
		return nil
	}
	// if LookupString success, StringToKeysym must be success
	sym, _ := keysyms.StringToKeysym(str)
	ks := Keystroke{
		Mods:   k.Mods,
		Keystr: str,
		Keysym: sym,
	}
	return ks.fix()
}

func (k Key) Ungrab(conn *x.Conn) {
	rootWin := conn.GetDefaultScreen().Root
	keybind.Ungrab(conn, rootWin, uint16(k.Mods), x.Keycode(k.Code))
}

func (k Key) Grab(conn *x.Conn) error {
	rootWin := conn.GetDefaultScreen().Root
	return keybind.GrabChecked(conn, rootWin, uint16(k.Mods), x.Keycode(k.Code))
}
