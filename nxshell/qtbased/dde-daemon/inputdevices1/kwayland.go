// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"os"
	"strconv"
	"strings"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/dxinput"
	"github.com/linuxdeepin/dde-api/dxinput/common"
	"github.com/linuxdeepin/dde-api/dxinput/kwayland"
	kwin "github.com/linuxdeepin/go-dbus-factory/session/org.kde.kwin"
)

var (
	globalWayland bool
)

func init() {
	if len(os.Getenv("WAYLAND_DISPLAY")) != 0 {
		globalWayland = true
	}
}

func (m *Manager) handleInputDeviceChanged(stop bool) {
	if stop && m.kwinManager != nil {
		for _, id := range m.kwinIdList {
			m.kwinManager.RemoveHandler(id)
		}
		return
	}

	m.kwinManager.InitSignalExt(_manager.sessionSigLoop, true)
	addedID, err := m.kwinManager.ConnectDeviceAdded(func(sysName string) {
		logger.Debug("[Device Added]:", sysName)
		doHandleKWinDeviceAdded(sysName)
		m.setWheelSpeed()
	})
	if err == nil {
		m.kwinIdList = append(m.kwinIdList, addedID)
	}
	removedID, err := m.kwinManager.ConnectDeviceRemoved(func(sysName string) {
		logger.Debug("[Device Removed]:", sysName)
		doHandleKWinDeviceRemoved(sysName)
		m.setWheelSpeed()
	})
	if err == nil {
		m.kwinIdList = append(m.kwinIdList, removedID)
	}
}

func (m *Manager) setWaylandWheelSpeed(speed uint32) error {
	deviceNames, err := m.kwinManager.DevicesSysNames().Get(0)
	if err != nil {
		logger.Warning(err)
		return err
	}

	for _, deviceName := range deviceNames {
		device, err := kwin.NewInputDevice(m.sessionSigLoop.Conn(), dbus.ObjectPath("/org/kde/KWin/InputDevice/"+deviceName))
		if err != nil {
			logger.Warning(err)
			continue
		}

		scrollFactor, err := device.ScrollFactor().Get(0)
		if err != nil {
			logger.Warning(err)
			return err
		}

		pointer, err := device.Pointer().Get(0)
		if err != nil {
			logger.Warning(err)
			continue
		}

		if pointer && speed != uint32(scrollFactor) {
			err := device.ScrollFactor().Set(0, float64(speed))

			if err != nil {
				logger.Warning(err)
				continue
			}
		}
	}

	return nil
}

func doHandleKWinDeviceAdded(sysName string) {
	info, err := kwayland.NewDeviceInfo(sysName)
	if err != nil {
		logger.Debug("[Device Added] Failed to new device:", err)
		return
	}
	if info == nil {
		return
	}
	logger.Debug("[Device Changed] added:", info.Id, info.Type, info.Name, info.Enabled)
	switch info.Type {
	case common.DevTypeMouse:
		v, _ := dxinput.NewMouseFromDeviceInfo(info)
		if len(_mouseInfos) == 0 {
			_mouseInfos = append(_mouseInfos, getMouseInfoByDxMouse(v))
		} else {
			for _, tmp := range _mouseInfos {
				if tmp.Id == v.Id {
					continue
				}
				_mouseInfos = append(_mouseInfos, getMouseInfoByDxMouse(v))
			}
		}
		_manager.mouse.handleDeviceChanged()
	case common.DevTypeTouchpad:
		v, _ := dxinput.NewTouchpadFromDevInfo(info)
		if len(_tpadInfos) == 0 {
			_tpadInfos = append(_tpadInfos, getTouchpadInfoByDxTouchpad(v))
		} else {
			for _, tmp := range _tpadInfos {
				if tmp.Id == v.Id {
					continue
				}
				_tpadInfos = append(_tpadInfos, getTouchpadInfoByDxTouchpad(v))
			}
		}
		_manager.tpad.handleDeviceChanged()
	}
}

func doHandleKWinDeviceRemoved(sysName string) {
	str := strings.TrimLeft(sysName, kwayland.SysNamePrefix) //nolint 9
	id, err := strconv.ParseInt(str, 10, 32)
	if err != nil {
		logger.Warning(err)
	}
	logger.Debug("----------------items:", sysName, str, id)

	var minfos Mouses
	var changed bool
	for _, tmp := range _mouseInfos {
		if tmp.Id == int32(id) {
			changed = true
			continue
		}
		minfos = append(minfos, tmp)
	}
	if changed {
		logger.Debug("[Device Removed] mouse:", sysName, minfos)
		_mouseInfos = minfos
		_manager.mouse.handleDeviceChanged()
		return
	}

	var tinfos Touchpads
	for _, tmp := range _tpadInfos {
		if tmp.Id == int32(id) {
			changed = true
			continue
		}
		tinfos = append(tinfos, tmp)
	}
	if changed {
		logger.Debug("[Device Removed] touchpad:", sysName)
		_tpadInfos = tinfos
		_manager.tpad.handleDeviceChanged()
	}
}
