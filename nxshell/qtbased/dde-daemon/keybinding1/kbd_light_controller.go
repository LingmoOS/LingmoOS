// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"errors"

	. "github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	backlight "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.backlighthelper1"
	commonbl "github.com/linuxdeepin/go-lib/backlight/common"
	kbdbl "github.com/linuxdeepin/go-lib/backlight/keyboard"
)

const backlightTypeKeyboard = 2

type KbdLightController struct {
	backlightHelper backlight.Backlight
}

func NewKbdLightController(backlightHelper backlight.Backlight) *KbdLightController {
	return &KbdLightController{
		backlightHelper: backlightHelper,
	}
}

func (c *KbdLightController) Name() string {
	return "Kbd Light"
}

func (c *KbdLightController) ExecCmd(cmd ActionCmd) error {
	switch cmd {
	case KbdLightBrightnessUp:
		return c.changeBrightness(true)
	case KbdLightBrightnessDown:
		return c.changeBrightness(false)
	case KbdLightToggle:
		return c.toggle()
	default:
		return ErrInvalidActionCmd{cmd}
	}
}

func getKbdBlController() (*commonbl.Controller, error) {
	controllers, err := kbdbl.List()
	if err != nil {
		return nil, err
	}
	if len(controllers) > 0 {
		return controllers[0], nil
	}
	return nil, errors.New("not found keyboard backlight controller")
}

// Let the keyboard light brightness switch between 0 to max
func (c *KbdLightController) toggle() error {
	if c.backlightHelper == nil {
		return ErrIsNil{"KbdLightController.backlightHelper"}
	}

	controller, err := getKbdBlController()
	if err != nil {
		return err
	}
	value, err := controller.GetBrightness()
	if err != nil {
		return err
	}

	if value == 0 {
		value = controller.MaxBrightness
	} else {
		value = 0
	}
	logger.Debug("[KbdLightController.toggle] will set kbd backlight to:", value)
	return c.backlightHelper.SetBrightness(0, backlightTypeKeyboard, controller.Name, int32(value))
}

var kbdBacklightStep int = 0

func (c *KbdLightController) changeBrightness(raised bool) error {
	if c.backlightHelper == nil {
		return ErrIsNil{"KbdLightController.backlightHelper"}
	}

	controller, err := getKbdBlController()
	if err != nil {
		return err
	}
	value, err := controller.GetBrightness()
	if err != nil {
		return err
	}

	maxValue := controller.MaxBrightness

	// step: (max < 10?1:max/10)
	if kbdBacklightStep == 0 {
		tmp := maxValue / 10
		if tmp == 0 {
			tmp = 1
		}
		// 4舍5入
		if float64(maxValue)/10 < float64(tmp)+0.5 {
			kbdBacklightStep = tmp
		} else {
			kbdBacklightStep = tmp + 1
		}
	}
	logger.Debug("[KbdLightController.changeBrightness] kbd backlight info:", value, maxValue, kbdBacklightStep)
	if raised {
		value += kbdBacklightStep
	} else {
		value -= kbdBacklightStep
	}

	if value < 0 {
		value = 0
	} else if value > maxValue {
		value = maxValue
	}

	logger.Debug("[KbdLightController.changeBrightness] will set kbd backlight to:", value)
	return c.backlightHelper.SetBrightness(0, backlightTypeKeyboard, controller.Name, int32(value))
}
