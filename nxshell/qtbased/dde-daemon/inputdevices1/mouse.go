// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"sync"

	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
)

const (
	mouseSchema = "com.deepin.dde.mouse"

	mouseKeyLeftHanded      = "left-handed"
	mouseKeyDisableTouchpad = "disable-touchpad"
	mouseKeyMiddleButton    = "middle-button-enabled"
	mouseKeyNaturalScroll   = "natural-scroll"
	mouseKeyAcceleration    = "motion-acceleration"
	mouseKeyThreshold       = "motion-threshold"
	mouseKeyScaling         = "motion-scaling"
	mouseKeyDoubleClick     = "double-click"
	mouseKeyDragThreshold   = "drag-threshold"
	mouseKeyAdaptiveAccel   = "adaptive-accel-profile"
)

type Mouse struct {
	service    *dbusutil.Service
	PropsMu    sync.RWMutex
	DeviceList string
	Exist      bool

	// dbusutil-gen: ignore-below
	Enabled               bool        `prop:"access:rw"`
	LeftHanded            gsprop.Bool `prop:"access:rw"`
	DisableTpad           gsprop.Bool `prop:"access:rw"`
	NaturalScroll         gsprop.Bool `prop:"access:rw"`
	MiddleButtonEmulation gsprop.Bool `prop:"access:rw"`
	AdaptiveAccelProfile  gsprop.Bool `prop:"access:rw"`

	MotionAcceleration gsprop.Double `prop:"access:rw"`
	MotionThreshold    gsprop.Double `prop:"access:rw"`
	MotionScaling      gsprop.Double `prop:"access:rw"`

	DoubleClick   gsprop.Int `prop:"access:rw"`
	DragThreshold gsprop.Int `prop:"access:rw"`

	devInfos Mouses
	setting  *gio.Settings
	touchPad *Touchpad
}

func newMouse(service *dbusutil.Service, touchPad *Touchpad) *Mouse {
	var m = new(Mouse)

	m.service = service
	m.touchPad = touchPad
	m.setting = gio.NewSettings(mouseSchema)
	m.LeftHanded.Bind(m.setting, mouseKeyLeftHanded)
	m.DisableTpad.Bind(m.setting, mouseKeyDisableTouchpad)
	m.NaturalScroll.Bind(m.setting, mouseKeyNaturalScroll)
	m.MiddleButtonEmulation.Bind(m.setting, mouseKeyMiddleButton)
	m.MotionAcceleration.Bind(m.setting, mouseKeyAcceleration)
	m.MotionThreshold.Bind(m.setting, mouseKeyThreshold)
	m.MotionScaling.Bind(m.setting, mouseKeyScaling)
	m.DoubleClick.Bind(m.setting, mouseKeyDoubleClick)
	m.DragThreshold.Bind(m.setting, mouseKeyDragThreshold)
	m.AdaptiveAccelProfile.Bind(m.setting, mouseKeyAdaptiveAccel)

	m.updateDXMouses()

	return m
}

func (m *Mouse) init() {
	//触摸板和鼠标都是用mouse的doubleClick，检测到只有触摸板时也同步到xsettings
	m.syncConfigToXsettings()

	tpad := m.touchPad

	if !m.Exist {
		if tpad.Exist && !tpad.TPadEnable.Get() {
			tpad.enable(true)
		}
		return
	}

	m.enableLeftHanded()
	m.enableMidBtnEmu()
	m.enableNaturalScroll()
	m.enableAdaptiveAccelProfile()
	m.motionAcceleration()
	m.motionThreshold()
	if m.DisableTpad.Get() && tpad.TPadEnable.Get() {
		m.disableTouchPad()
	}
}

func (m *Mouse) handleDeviceChanged() {
	m.updateDXMouses()
	m.init()
}

func (m *Mouse) updateDXMouses() {
	m.devInfos = Mouses{}
	// 在有鼠标连接下，len(_mouseInfos)不等于0，传参false，
	// 切换用户后，handleDeviceChanged，getMouseInfos直接返回，未进行忽略触摸板设备的判断
	// 导致识别到2个鼠标，拔掉鼠标后，还剩一个，在设置‘插入鼠标时禁用触控板’后，触摸板无法使用
	for _, info := range getMouseInfos(true) {
		if info.TrackPoint {
			continue
		}

		if info.IsEnabled() {
			m.Enabled = true
		}

		if !globalWayland {
			tmp := m.devInfos.get(info.Id)
			if tmp != nil {
				continue
			}
		}
		m.devInfos = append(m.devInfos, info)
	}

	m.PropsMu.Lock()
	var v string
	if len(m.devInfos) == 0 {
		m.setPropExist(false)
	} else {
		m.setPropExist(true)
		v = m.devInfos.string()
	}
	m.setPropDeviceList(v)
	m.PropsMu.Unlock()
}

func (m *Mouse) disableTouchPad() {
	m.PropsMu.RLock()
	mouseExist := m.Exist
	m.PropsMu.RUnlock()
	if !mouseExist {
		return
	}

	touchPad := m.touchPad
	touchPad.PropsMu.RLock()
	touchPadExist := touchPad.Exist
	touchPad.PropsMu.RUnlock()
	if !touchPadExist {
		return
	}

	if !m.DisableTpad.Get() && !touchPad.TPadEnable.Get() {
		touchPad.enable(true)
		return
	}

	touchPad.enable(false)
}

func (m *Mouse) enable(enabled bool) error {
	for _, v := range m.devInfos {
		err := v.Enable(enabled)
		if err != nil {
			logger.Debugf("Enable left handed for '%d - %v' failed: %v",
				v.Id, v.Name, err)
			return err
		}
	}

	m.Enabled = enabled

	return nil
}

func (m *Mouse) enableLeftHanded() {
	enabled := m.LeftHanded.Get()
	for _, v := range m.devInfos {
		err := v.EnableLeftHanded(enabled)
		if err != nil {
			logger.Debugf("Enable left handed for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (m *Mouse) enableNaturalScroll() {
	enabled := m.NaturalScroll.Get()
	for _, v := range m.devInfos {
		err := v.EnableNaturalScroll(enabled)
		if err != nil {
			logger.Debugf("Enable natural scroll for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (m *Mouse) enableMidBtnEmu() {
	enabled := m.MiddleButtonEmulation.Get()
	for _, v := range m.devInfos {
		if v.TrackPoint {
			continue
		}

		err := v.EnableMiddleButtonEmulation(enabled)
		if err != nil {
			logger.Debugf("Enable mid btn emulation for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (m *Mouse) enableAdaptiveAccelProfile() {
	enabled := m.AdaptiveAccelProfile.Get()
	for _, v := range m.devInfos {
		if !v.CanChangeAccelProfile() {
			continue
		}

		err := v.SetUseAdaptiveAccelProfile(enabled)
		if err != nil {
			logger.Debugf("Enable adaptive accel profile for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (m *Mouse) motionAcceleration() {
	accel := float32(m.MotionAcceleration.Get())
	for _, v := range m.devInfos {
		if v.TrackPoint {
			continue
		}

		err := v.SetMotionAcceleration(accel)
		if err != nil {
			logger.Debugf("Set acceleration for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (m *Mouse) motionThreshold() {
	thres := float32(m.MotionThreshold.Get())
	for _, v := range m.devInfos {
		if v.TrackPoint {
			continue
		}

		err := v.SetMotionThreshold(thres)
		if err != nil {
			logger.Debugf("Set threshold for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (m *Mouse) motionScaling() {
	scaling := float32(m.MotionScaling.Get())
	for _, v := range m.devInfos {
		if v.TrackPoint {
			continue
		}

		err := v.SetMotionScaling(scaling)
		if err != nil {
			logger.Debugf("Set scaling for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (m *Mouse) doubleClick() {
	xsSetInt32(xsPropDoubleClick, m.DoubleClick.Get())
}

func (m *Mouse) dragThreshold() {
	xsSetInt32(xsPropDragThres, m.DragThreshold.Get())
}

func (m *Mouse) syncConfigToXsettings() {
	m.doubleClick() // 初始化时,将默认配置同步到xsettings中
}
