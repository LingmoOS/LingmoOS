// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"github.com/linuxdeepin/dde-api/powersupply/battery"
)

const (
	dbusServiceName = "org.deepin.dde.Power1"
	dbusPath        = "/org/deepin/dde/Power1"
	dbusInterface   = dbusServiceName
)

func (m *Manager) setPropBatteryIsPresent(val bool) {
	old, exist := m.BatteryIsPresent[batteryDisplay]
	if old != val || !exist {
		m.BatteryIsPresent[batteryDisplay] = val
		m.emitPropChangedBatteryIsPresent()
	}
}

func (m *Manager) emitPropChangedBatteryIsPresent() {
	err := m.service.EmitPropertyChanged(m, "BatteryIsPresent", m.BatteryIsPresent)
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) setPropBatteryPercentage(val float64) {
	logger.Debugf("set batteryDisplay percentage %.1f%%", val)
	old, exist := m.BatteryPercentage[batteryDisplay]
	if old != val || !exist {
		m.BatteryPercentage[batteryDisplay] = val
		m.emitPropChangedBatteryPercentage()
	}
}

func (m *Manager) emitPropChangedBatteryPercentage() {
	err := m.service.EmitPropertyChanged(m, "BatteryPercentage", m.BatteryPercentage)
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) setPropBatteryState(val uint32) {
	logger.Debug("set BatteryDisplay status", battery.Status(val), val)
	old, exist := m.BatteryState[batteryDisplay]
	if old != val || !exist {
		m.BatteryState[batteryDisplay] = val
		m.emitPropChangedBatteryState()
	}
}

func (m *Manager) emitPropChangedBatteryState() {
	err := m.service.EmitPropertyChanged(m, "BatteryState", m.BatteryState)
	if err != nil {
		logger.Warning(err)
	}
}
