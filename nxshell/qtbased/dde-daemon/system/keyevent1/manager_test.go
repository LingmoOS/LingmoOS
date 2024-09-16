// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keyevent1

import "testing"

func Test_handleEvent(t *testing.T) {
	m := Manager{}
	ev := &KeyEvent{
		Keycode: KEY_LEFTCTRL,
	}
	m.handleEvent(ev)

	ev.Keycode = KEY_RIGHTCTRL
	m.handleEvent(ev)

	ev.Keycode = KEY_LEFTALT
	m.handleEvent(ev)

	ev.Keycode = KEY_RIGHTALT
	m.handleEvent(ev)

	ev.Keycode = KEY_LEFTSHIFT
	m.handleEvent(ev)

	ev.Keycode = KEY_RIGHTSHIFT
	m.handleEvent(ev)

	ev.Keycode = KEY_LEFTMETA
	m.handleEvent(ev)

	ev.Keycode = KEY_RIGHTMETA
	m.handleEvent(ev)
}
