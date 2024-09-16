// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const (
	wmKbdSchemaID    = "com.deepin.wrap.gnome.desktop.peripherals.keyboard"
	wmKbdKeyRepeat   = "repeat"
	wmKbdKeyDelay    = "delay"
	wmKbdKeyInterval = "repeat-interval"

	wmTPadSchemaID         = "com.deepin.wrap.gnome.desktop.peripherals.touchpad"
	wmTPadKeyEdgeScroll    = "edge-scrolling-enabled"
	wmTPadKeyNaturalScroll = "natural-scroll"
	wmTPadKeyTapClick      = "tap-to-click"
	// enum: mouse, left, right
	wmTPadKeyLeftHanded = "left-handed"
)

func setWMKeyboardRepeat(repeat bool, delay, interval uint32) {
	setting, err := dutils.CheckAndNewGSettings(wmKbdSchemaID)
	if err != nil {
		logger.Warning("Failed to new wm keyboard settings")
		return
	}
	defer setting.Unref()

	if setting.GetBoolean(wmKbdKeyRepeat) != repeat {
		setting.SetBoolean(wmKbdKeyRepeat, repeat)
	}
	if setting.GetUint(wmKbdKeyDelay) != delay {
		setting.SetUint(wmKbdKeyDelay, delay)
	}
	if setting.GetUint(wmKbdKeyInterval) != interval {
		setting.SetUint(wmKbdKeyInterval, interval)
	}
}

func setWMTPadBoolKey(key string, value bool) {
	setting, err := dutils.CheckAndNewGSettings(wmTPadSchemaID)
	if err != nil {
		logger.Warning("Failed to new wm touchpad settings")
		return
	}
	defer setting.Unref()

	switch key {
	case wmTPadKeyEdgeScroll, wmTPadKeyNaturalScroll, wmTPadKeyTapClick:
		if v := setting.GetBoolean(key); v == value {
			return
		}
		setting.SetBoolean(key, value)
		//case wmTPadKeyLeftHanded:
		//v := setting.GetString(key)
		//var tmp = "right"
		//if value {
		//tmp = "left"
		//}
		//if v == tmp {
		//return
		//}
		//setting.SetString(key, tmp)
	}
}
