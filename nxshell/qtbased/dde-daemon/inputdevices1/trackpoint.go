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
	trackPointSchema              = "com.deepin.dde.trackpoint"
	trackPointKeyMidButton        = "middle-button-enabled"
	trackPointKeyMidButtonTimeout = "middle-button-timeout"
	trackPointKeyWheel            = "wheel-emulation"
	trackPointKeyWheelButton      = "wheel-emulation-button"
	trackPointKeyWheelTimeout     = "wheel-emulation-timeout"
	trackPointKeyWheelHorizScroll = "wheel-horiz-scroll"
	trackPointKeyAcceleration     = "motion-acceleration"
	trackPointKeyThreshold        = "motion-threshold"
	trackPointKeyScaling          = "motion-scaling"
	trackPointKeyLeftHanded       = "left-handed"
)

type TrackPoint struct {
	service    *dbusutil.Service
	PropsMu    sync.RWMutex
	DeviceList string
	Exist      bool

	// dbusutil-gen: ignore-below
	MiddleButtonEnabled gsprop.Bool `prop:"access:rw"`
	WheelEmulation      gsprop.Bool `prop:"access:rw"`
	WheelHorizScroll    gsprop.Bool `prop:"access:rw"`

	MiddleButtonTimeout   gsprop.Int `prop:"access:rw"`
	WheelEmulationButton  gsprop.Int `prop:"access:rw"`
	WheelEmulationTimeout gsprop.Int `prop:"access:rw"`

	MotionAcceleration gsprop.Double `prop:"access:rw"`
	MotionThreshold    gsprop.Double `prop:"access:rw"`
	MotionScaling      gsprop.Double `prop:"access:rw"`

	LeftHanded gsprop.Bool `prop:"access:rw"`

	devInfos Mouses
	setting  *gio.Settings
}

func newTrackPoint(service *dbusutil.Service) *TrackPoint {
	var tp = new(TrackPoint)

	tp.service = service
	tp.setting = gio.NewSettings(trackPointSchema)
	tp.MiddleButtonEnabled.Bind(tp.setting, trackPointKeyMidButton)
	tp.WheelEmulation.Bind(tp.setting, trackPointKeyWheel)
	tp.WheelHorizScroll.Bind(tp.setting, trackPointKeyWheelHorizScroll)
	tp.MotionAcceleration.Bind(tp.setting, trackPointKeyAcceleration)
	tp.MotionThreshold.Bind(tp.setting, trackPointKeyThreshold)
	tp.MotionScaling.Bind(tp.setting, trackPointKeyScaling)
	tp.MiddleButtonTimeout.Bind(tp.setting, trackPointKeyMidButtonTimeout)
	tp.WheelEmulationButton.Bind(tp.setting, trackPointKeyWheelButton)
	tp.WheelEmulationTimeout.Bind(tp.setting, trackPointKeyWheelTimeout)
	tp.LeftHanded.Bind(tp.setting, trackPointKeyLeftHanded)

	tp.updateDXMouses()

	return tp
}

func (tp *TrackPoint) init() {
	if !tp.Exist {
		return
	}

	tp.enableMiddleButton()
	tp.enableWheelEmulation()
	tp.enableWheelHorizScroll()
	tp.enableLeftHanded()
	tp.middleButtonTimeout()
	tp.wheelEmulationButton()
	tp.wheelEmulationTimeout()
	tp.motionAcceleration()
	tp.motionThreshold()
	tp.motionScaling()
}

func (tp *TrackPoint) updateDXMouses() {
	tp.devInfos = Mouses{}
	for _, info := range getMouseInfos(false) {
		if !info.TrackPoint {
			continue
		}

		tmp := tp.devInfos.get(info.Id)
		if tmp != nil {
			continue
		}
		tp.devInfos = append(tp.devInfos, info)
	}

	tp.PropsMu.Lock()
	var v string
	if len(tp.devInfos) == 0 {
		tp.setPropExist(false)
	} else {
		tp.setPropExist(true)
		v = tp.devInfos.string()
	}
	tp.setPropDeviceList(v)
	tp.PropsMu.Unlock()
}

func (tp *TrackPoint) enableMiddleButton() {
	enabled := tp.MiddleButtonEnabled.Get()
	for _, info := range tp.devInfos {
		err := info.EnableMiddleButtonEmulation(enabled)
		if err != nil {
			logger.Warningf("Enable middle button for '%v %s' failed: %v",
				info.Id, info.Name, err)
		}
	}
}

func (tp *TrackPoint) enableWheelEmulation() {
	enabled := tp.WheelEmulation.Get()
	for _, info := range tp.devInfos {
		err := info.EnableWheelEmulation(enabled)
		if err != nil {
			logger.Warningf("Enable wheel emulation for '%v %s' failed: %v",
				info.Id, info.Name, err)
		}
	}
}

func (tp *TrackPoint) enableWheelHorizScroll() {
	enabled := tp.WheelHorizScroll.Get()
	for _, info := range tp.devInfos {
		err := info.EnableWheelHorizScroll(enabled)
		if err != nil {
			logger.Warningf("Enable wheel horiz scroll for '%v %s' failed: %v",
				info.Id, info.Name, err)
		}
	}
}

func (tp *TrackPoint) enableLeftHanded() {
	enabled := tp.LeftHanded.Get()
	for _, info := range tp.devInfos {
		err := info.EnableLeftHanded(enabled)
		if err != nil {
			logger.Warningf("Enable left-handed for '%v %s' failed: %v",
				info.Id, info.Name, err)
		}
	}
}

func (tp *TrackPoint) middleButtonTimeout() {
	timeout := tp.MiddleButtonTimeout.Get()
	for _, info := range tp.devInfos {
		err := info.SetMiddleButtonEmulationTimeout(int16(timeout))
		if err != nil {
			logger.Warningf("Set middle button timeout for '%v %s' failed: %v",
				info.Id, info.Name, err)
		}
	}
}

func (tp *TrackPoint) wheelEmulationButton() {
	button := tp.WheelEmulationButton.Get()
	for _, info := range tp.devInfos {
		err := info.SetWheelEmulationButton(int8(button))
		if err != nil {
			logger.Warningf("Set wheel button for '%v %s' failed: %v",
				info.Id, info.Name, err)
		}
	}
}

func (tp *TrackPoint) wheelEmulationTimeout() {
	timeout := tp.WheelEmulationTimeout.Get()
	for _, info := range tp.devInfos {
		err := info.SetWheelEmulationTimeout(int16(timeout))
		if err != nil {
			logger.Warningf("Enable wheel timeout for '%v %s' failed: %v",
				info.Id, info.Name, err)
		}
	}
}

func (tp *TrackPoint) motionAcceleration() {
	accel := float32(tp.MotionAcceleration.Get())
	for _, v := range tp.devInfos {
		err := v.SetMotionAcceleration(accel)
		if err != nil {
			logger.Debugf("Set acceleration for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (tp *TrackPoint) motionThreshold() {
	thres := float32(tp.MotionThreshold.Get())
	for _, v := range tp.devInfos {
		err := v.SetMotionThreshold(thres)
		if err != nil {
			logger.Debugf("Set threshold for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}

func (tp *TrackPoint) motionScaling() {
	scaling := float32(tp.MotionScaling.Get())
	for _, v := range tp.devInfos {
		err := v.SetMotionScaling(scaling)
		if err != nil {
			logger.Debugf("Set scaling for '%d - %v' failed: %v",
				v.Id, v.Name, err)
		}
	}
}
