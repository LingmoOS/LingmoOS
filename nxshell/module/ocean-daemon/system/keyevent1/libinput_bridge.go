// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keyevent1

// #include "libinput_bridge.h"
// #cgo pkg-config: libinput glib-2.0
// #cgo LDFLAGS: -ludev -lm
// #cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
import "C"

// nolint
// 按键状态
const (
	KEY_STATE_RELEASED = 0 // 松开
	KEY_STATE_PRESSED  = 1 // 按下
)

type KeyEvent struct {
	Keycode uint32 // 按键码
	State   uint32 // KEY_STATE_RELEASED,KEY_STATE_PRESSED
}

var eventChanList []chan *KeyEvent

// 开始监控按键
func startKeyEventMonitor() {
	go func() {
		C.loop_startup()
	}()
}

// 停止监控按键事件
func stopKeyEventMonitor() {
	C.loop_stop()
}

// 添加channel用于读取按键事件
func addKeyEventChannel(ch chan *KeyEvent) {
	eventChanList = append(eventChanList, ch)
}

//export pushKeyEvent
func pushKeyEvent(keycode uint32, state uint32) {
	event := &KeyEvent{
		Keycode: keycode,
		State:   state,
	}

	for _, ch := range eventChanList {
		select {
		case ch <- event:
		default:
		}
	}
}
