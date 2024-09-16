// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"github.com/linuxdeepin/dde-api/dxinput"
	"github.com/linuxdeepin/go-lib/gsettings"
)

func (m *Manager) handleGSettings() {
	gsettings.ConnectChanged(gsSchemaInputDevices, gsKeyWheelSpeed, func(key string) {
		m.setWheelSpeed()
	})
}

func (kbd *Keyboard) handleGSettings() {
	gsettings.ConnectChanged(kbdSchema, "*", func(key string) {
		switch key {
		case kbdKeyRepeatEnable, kbdKeyRepeatDelay,
			kbdKeyRepeatInterval:
			kbd.applyRepeat()
		case kbdKeyCursorBlink:
			kbd.applyCursorBlink()
		case kbdKeyLayoutOptions:
			kbd.applyOptions()
		}
	})
}

func (m *Mouse) handleGSettings() {
	gsettings.ConnectChanged(mouseSchema, "*", func(key string) {
		switch key {
		case mouseKeyLeftHanded:
			m.enableLeftHanded()
		case mouseKeyDisableTouchpad:
			m.disableTouchPad()
		case mouseKeyNaturalScroll:
			m.enableNaturalScroll()
		case mouseKeyMiddleButton:
			m.enableMidBtnEmu()
		case mouseKeyAcceleration:
			m.motionAcceleration()
		case mouseKeyThreshold:
			m.motionThreshold()
		case mouseKeyScaling:
			m.motionScaling()
		case mouseKeyDoubleClick:
			m.doubleClick()
		case mouseKeyDragThreshold:
			m.dragThreshold()
		case mouseKeyAdaptiveAccel:
			m.enableAdaptiveAccelProfile()
		}
	})

	gsettings.ConnectChanged(tpadSchema, tpadKeyEnabled, func(key string) {
		logger.Debug("setting changed", key)
		if m.DisableTpad.Get() {
			m.disableTouchPad()
		} else {
			m.touchPad.enable(m.touchPad.TPadEnable.Get())
		}
	})
}

func (tp *TrackPoint) handleGSettings() {
	gsettings.ConnectChanged(trackPointSchema, "*", func(key string) {
		switch key {
		case trackPointKeyMidButton:
			tp.enableMiddleButton()
		case trackPointKeyMidButtonTimeout:
			tp.middleButtonTimeout()
		case trackPointKeyWheel:
			tp.enableWheelEmulation()
		case trackPointKeyWheelButton:
			tp.wheelEmulationButton()
		case trackPointKeyWheelTimeout:
			tp.wheelEmulationTimeout()
		case trackPointKeyWheelHorizScroll:
			tp.enableWheelHorizScroll()
		case trackPointKeyLeftHanded:
			tp.enableLeftHanded()
		case trackPointKeyAcceleration:
			tp.motionAcceleration()
		case trackPointKeyThreshold:
			tp.motionThreshold()
		case trackPointKeyScaling:
			tp.motionScaling()
		}
	})
}

func (tpad *Touchpad) handleGSettings() {
	gsettings.ConnectChanged(tpadSchema, "*", func(key string) {
		switch key {
		case tpadKeyLeftHanded:
			tpad.enableLeftHanded()
			tpad.enableTapToClick()
		case tpadKeyTapClick:
			tpad.enableTapToClick()
		case tpadKeyNaturalScroll:
			tpad.enableNaturalScroll()
		case tpadKeyScrollDelta:
			tpad.setScrollDistance()
		case tpadKeyEdgeScroll:
			tpad.enableEdgeScroll()
		case tpadKeyVertScroll, tpadKeyHorizScroll:
			tpad.enableTwoFingerScroll()
		case tpadKeyDisableWhileTyping:
			tpad.disableWhileTyping()
		case tpadKeyAcceleration:
			tpad.motionAcceleration()
		case tpadKeyThreshold:
			tpad.motionThreshold()
		case tpadKeyScaling:
			tpad.motionScaling()
		case tpadKeyPalmDetect:
			tpad.enablePalmDetect()
		case tpadKeyPalmMinWidth, tpadKeyPalmMinZ:
			tpad.setPalmDimensions()
		}
	})
}

func (w *Wacom) handleGSettings() {
	gsettings.ConnectChanged(wacomSchema, "*", func(key string) {
		logger.Debugf("wacom gsettings changed %v", key)
		switch key {
		case wacomKeyLeftHanded:
			w.enableLeftHanded()
		case wacomKeyCursorMode:
			w.enableCursorMode()
		case wacomKeySuppress:
			w.setSuppress()
		case wacomKeyForceProportions:
			w.setArea()
		}
	})

	gsettings.ConnectChanged(wacomStylusSchema, "*", func(key string) {
		logger.Debugf("wacom.stylus gsettings changed %v", key)
		switch key {
		case wacomKeyPressureSensitive:
			w.setPressureSensitiveForType(dxinput.WacomTypeStylus)
		case wacomKeyUpAction:
			w.setStylusButtonAction(btnNumUpKey, w.KeyUpAction.Get())
		case wacomKeyDownAction:
			w.setStylusButtonAction(btnNumDownKey, w.KeyDownAction.Get())
		case wacomKeyThreshold:
			w.setThresholdForType(dxinput.WacomTypeStylus)
		case wacomKeyRawSample:
			w.setRawSampleForType(dxinput.WacomTypeStylus)
		}
	})

	gsettings.ConnectChanged(wacomEraserSchema, "*", func(key string) {
		logger.Debugf("wacom.eraser gsettings changed %v", key)
		switch key {
		case wacomKeyPressureSensitive:
			w.setPressureSensitiveForType(dxinput.WacomTypeEraser)
		case wacomKeyThreshold:
			w.setThresholdForType(dxinput.WacomTypeEraser)
		case wacomKeyRawSample:
			w.setRawSampleForType(dxinput.WacomTypeEraser)
		}
	})
}
