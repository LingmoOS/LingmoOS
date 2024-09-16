// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"github.com/godbus/dbus/v5"
	. "github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	inputdevices "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.inputdevices1"
)

type TouchPadController struct {
	touchPad inputdevices.TouchPad
}

func NewTouchPadController(sessionConn *dbus.Conn) *TouchPadController {
	c := new(TouchPadController)
	c.touchPad = inputdevices.NewTouchPad(sessionConn)
	return c
}

func (*TouchPadController) Name() string {
	return "TouchPad"
}

func (c *TouchPadController) ExecCmd(cmd ActionCmd) error {
	switch cmd {
	case TouchpadToggle:
		err := c.toggle()
		if err != nil {
			return err
		}
	case TouchpadOn:
		err := c.enable(true)
		if err != nil {
			return err
		}
	case TouchpadOff:
		err := c.enable(false)
		if err != nil {
			return err
		}
	default:
		return ErrInvalidActionCmd{cmd}
	}
	return nil
}

func (c *TouchPadController) enable(val bool) error {
	exist, err := c.touchPad.Exist().Get(0)
	if err != nil {
		return err
	}
	if !exist {
		return nil
	}

	err = c.touchPad.TPadEnable().Set(0, val)
	if err != nil {
		return err
	}

	osd := "TouchpadOn"
	if !val {
		osd = "TouchpadOff"
	}
	showOSD(osd)
	return nil
}

func (c *TouchPadController) toggle() error {
	// check touchpad exist?
	exist, err := c.touchPad.Exist().Get(0)
	if err != nil {
		return err
	}
	if !exist {
		return nil
	}

	if globalConfig.HandleTouchPadToggle {
		enabled, err := c.touchPad.TPadEnable().Get(0)
		if err != nil {
			return err
		}
		err = c.touchPad.TPadEnable().Set(0, !enabled)
		if err != nil {
			return err
		}
	}

	showOSD("TouchpadToggle")
	return nil
}
