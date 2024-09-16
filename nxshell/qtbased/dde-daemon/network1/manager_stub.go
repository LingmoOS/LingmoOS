// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"errors"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/network1/nm"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

func (m *Manager) networkingEnabledWriteCb(write *dbusutil.PropertyWrite) *dbus.Error {
	// currently not need
	return nil
}

func (m *Manager) vpnEnabledWriteCb(write *dbusutil.PropertyWrite) *dbus.Error {
	enabled, ok := write.Value.(bool)
	if !ok {
		err := errors.New("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	// FIXME:
	// 由于断开是VPN是在system中完成，断开会触发VPN自动连接重试，这里提前先调整状态
	// 避免自动重连
	if !enabled {
		m.setPropVpnEnabled(enabled)
	}

	err := m.sysNetwork.VpnEnabled().Set(0, enabled)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	// if vpn enable state is set as true, try to auto connect vpn.
	// when vpn enable state is set as false, close all vpn connections in system/network module.
	if enabled {
		err := enableNetworking()
		if err != nil {
			logger.Warning(err)
			return nil
		}
		m.setVpnEnable(true)
	}

	return nil
}

func (m *Manager) updatePropActiveConnections() {
	activeConnections, _ := marshalJSON(m.activeConnections)
	m.setPropActiveConnections(activeConnections)
}

func (m *Manager) updatePropState() {
	state := nmGetManagerState()
	m.setPropState(state)
}

func (m *Manager) updatePropDevices() {
	filteredDevices := make(map[string][]*device)
	for key, devices := range m.devices {
		filteredDevices[key] = make([]*device, 0)
		for _, d := range devices {
			ignoreIphoneUsbDevice := d.UsbDevice &&
				d.State <= nm.NM_DEVICE_STATE_UNAVAILABLE &&
				d.Driver == "ipheth"
			if !ignoreIphoneUsbDevice {
				filteredDevices[key] = append(filteredDevices[key], d)
			}
		}
	}
	devices, _ := marshalJSON(filteredDevices)
	m.setPropDevices(devices)
}

func (m *Manager) updatePropConnections() {
	connections, _ := marshalJSON(m.connections)
	m.setPropConnections(connections)
}

func (m *Manager) updatePropWirelessAccessPoints() {
	aps, _ := marshalJSON(m.accessPoints)
	m.setPropWirelessAccessPoints(aps)
}
