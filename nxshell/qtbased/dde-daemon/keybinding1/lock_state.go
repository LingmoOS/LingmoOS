// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"errors"
	"fmt"
	"os"
	"time"

	"github.com/godbus/dbus/v5"

	"github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	kwayland "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.kwayland1"
	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/ext/test"
	"github.com/linuxdeepin/go-x11-client/util/keysyms"
)

// NumLockState 数字锁定键状态
type NumLockState uint

const (
	NumLockOff     NumLockState = iota // 关闭，不能输入数字，是方向键
	NumLockOn                          // 开启，能输入数字
	NumLockUnknown                     // 未知，出错
)

func (s NumLockState) String() string {
	switch s {
	case NumLockOff:
		return "off"
	case NumLockOn:
		return "on"
	case NumLockUnknown:
		return "unknown"
	default:
		return fmt.Sprintf("invalid-num-lock-state(%d)", s)
	}
}

// CapsLockState 大小写锁定键状态
type CapsLockState uint

const (
	CapsLockOff     CapsLockState = iota // 关闭，默认输入小写字母
	CapsLockOn                           // 开启，默认输入大写字母
	CapsLockUnknown                      // 未知，出错
)

// 查询 NumLock 数字锁定键状态
func queryNumLockState(conn *x.Conn) (NumLockState, error) {
	rootWin := conn.GetDefaultScreen().Root
	queryPointerReply, err := x.QueryPointer(conn, rootWin).Reply(conn)
	if err != nil {
		return NumLockUnknown, err
	}
	logger.Debugf("query pointer reply %#v", queryPointerReply)
	on := queryPointerReply.Mask&x.ModMask2 != 0
	if on {
		return NumLockOn, nil
	} else {
		return NumLockOff, nil
	}
}

// 查询 CapsLock 大小写锁定键状态
func queryCapsLockState(conn *x.Conn) (CapsLockState, error) {
	rootWin := conn.GetDefaultScreen().Root
	queryPointerReply, err := x.QueryPointer(conn, rootWin).Reply(conn)
	if err != nil {
		return CapsLockUnknown, err
	}
	logger.Debugf("query pointer reply %#v", queryPointerReply)
	on := queryPointerReply.Mask&x.ModMaskLock != 0
	if on {
		return CapsLockOn, nil
	} else {
		return CapsLockOff, nil
	}
}

func setNumLockWl(wl kwayland.OutputManagement, conn *x.Conn, state NumLockState) error {
	if !(state == NumLockOff || state == NumLockOn) {
		return errors.New("invalid num lock state")
	}

	logger.Debug("setNumLockWl", state)

	var state0 NumLockState
	if len(os.Getenv("WAYLAND_DISPLAY")) != 0 {
		sessionBus, err := dbus.SessionBus()
		if err != nil {
			return err
		}
		time.Sleep(200 * time.Millisecond) //+ 添加200ms延时，保证在dde-system-daemon中先获取状态；
		sessionObj := sessionBus.Object("org.kde.KWin", "/Xkb")
		var ret int32
		err = sessionObj.Call("org.kde.kwin.Xkb.getLeds", 0).Store(&ret)
		if err != nil {
			logger.Warning(err)
			return err
		}
		if 0 == (ret & 0x1) {
			state0 = NumLockOff
		} else {
			state0 = NumLockOn
		}
	} else {
		var err error
		state0, err = queryNumLockState(conn)

		if err != nil {
			return err
		}
	}

	if state0 != state {
		return wl.WlSimulateKey(0, 69) //69-kwin对应的NumLock
	}

	return nil
}

// 设置 NumLock 数字锁定键状态
func setNumLockX11(conn *x.Conn, keySymbols *keysyms.KeySymbols, state NumLockState) error {
	logger.Debug("setNumLockState", state)
	if !(state == NumLockOff || state == NumLockOn) {
		return errors.New("invalid num lock state")
	}

	state0, err := queryNumLockState(conn)
	if err != nil {
		return err
	}

	if state0 != state {
		return changeNumLockState(conn, keySymbols)
	}
	return nil
}

// 设置 NumLock 数字锁定键状态
func setNumLockState(wl kwayland.OutputManagement, conn *x.Conn, keySymbols *keysyms.KeySymbols, state NumLockState) error {
	if _useWayland {
		return setNumLockWl(wl, conn, state)
	}

	return setNumLockX11(conn, keySymbols, state)
}

// 设置 CapsLock 大小写锁定键状态
func setCapsLockState(conn *x.Conn, keySymbols *keysyms.KeySymbols, state CapsLockState) error {
	if !(state == CapsLockOff || state == CapsLockOn) {
		return errors.New("invalid caps lock state")
	}

	state0, err := queryCapsLockState(conn)
	if err != nil {
		return err
	}

	if state0 != state {
		return changeCapsLockState(conn, keySymbols)
	}
	return nil
}

// 修改 NumLock 数字锁定键状态，模拟一次相应的键按下和释放。
func changeNumLockState(conn *x.Conn, keySymbols *keysyms.KeySymbols) (err error) {
	// get Num_Lock keycode
	numLockKeycode, err := shortcuts.GetKeyFirstCode(keySymbols, "Num_Lock")
	if err != nil {
		return err
	}
	logger.Debug("numLockKeycode is", numLockKeycode)
	return simulatePressReleaseKey(conn, numLockKeycode)
}

// 修改 CapsLock 大小写锁定键状态，模拟一次相应的键按下释放。
func changeCapsLockState(conn *x.Conn, keySymbols *keysyms.KeySymbols) (err error) {
	// get Caps_Lock keycode
	capsLockKeycode, err := shortcuts.GetKeyFirstCode(keySymbols, "Caps_Lock")
	if err != nil {
		return err
	}
	logger.Debug("capsLockKeycode is", capsLockKeycode)
	return simulatePressReleaseKey(conn, capsLockKeycode)
}

// 模拟键的按下和释放
func simulatePressReleaseKey(conn *x.Conn, code x.Keycode) error {
	rootWin := conn.GetDefaultScreen().Root
	// fake key press
	err := test.FakeInputChecked(conn, x.KeyPressEventCode, byte(code), x.TimeCurrentTime, rootWin, 0, 0, 0).Check(conn)
	if err != nil {
		return err
	}
	// fake key release
	err = test.FakeInputChecked(conn, x.KeyReleaseEventCode, byte(code), x.TimeCurrentTime, rootWin, 0, 0, 0).Check(conn)
	if err != nil {
		return err
	}
	return nil
}
