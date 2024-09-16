// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"fmt"
	"os/exec"
)

type DSPCMode string

const (
	DSPCPerformance DSPCMode = "performance"
	DSPCBalance     DSPCMode = "balance"
	DSPCSaving      DSPCMode = "saving"
	DSPCLowBattery  DSPCMode = "lowbat"
)

type powerConfig struct {
	DSPCConfig DSPCMode

	PowerSavingModeEnabled bool
}

// TODO dconfig 或其他配置存储
var _powerConfigMap = map[string]*powerConfig{
	ddePerformance: {
		DSPCConfig:             DSPCPerformance,
		PowerSavingModeEnabled: false,
	},
	ddeBalance: {
		DSPCConfig:             DSPCBalance,
		PowerSavingModeEnabled: false,
	},
	ddePowerSave: {
		DSPCConfig:             DSPCSaving,
		PowerSavingModeEnabled: true,
	},
	ddeLowBattery: {
		DSPCConfig:             DSPCLowBattery,
		PowerSavingModeEnabled: true,
	},
}

func (m *Manager) setDSPCState(state DSPCMode) {
	args := fmt.Sprintf("/usr/sbin/deepin-system-power-control set %v", state)
	logger.Debug("set deepin tlp state cmd:", args)
	err := exec.Command("/bin/sh", "-c", args).Run()
	if err != nil {
		logger.Warning("failed to set deepin tlp state ", err)
	}
}

// 关联电量、电源连接状态、低电量节能开关、使用电池节能开关四项状态的变动，修改系统的功耗模式
func (m *Manager) updatePowerMode(init bool) {
	logger.Info("start updatePowerMode")
	if !m.initDone {
		// 初始化未完成时，暂不提供功能
		return
	}
	var enablePowerSave bool
	var enableLowPower bool
	m.PropsMu.Lock()
	defer m.PropsMu.Unlock()
	if m.PowerSavingModeAuto && m.OnBattery {
		enablePowerSave = true
	}

	if m.PowerSavingModeAutoWhenBatteryLow && m.batteryLow {
		enableLowPower = true
	}
	logger.Infof("PowerSavingModeAuto: %v\n OnBattery:%v \n PowerSavingModeAutoWhenBatteryLow:%v \n batteryLow:%v \n",
		m.PowerSavingModeAuto, m.OnBattery, m.PowerSavingModeAutoWhenBatteryLow, m.batteryLow)
	logger.Infof("lastMode: %v", m.lastMode)
	if !m.PowerSavingModeAuto && !m.PowerSavingModeAutoWhenBatteryLow && !init {
		return
	}
	mode := m.lastMode
	if init {
		mode = m.Mode
	}
	if enablePowerSave {
		mode = ddePowerSave
	}
	if enableLowPower {
		mode = ddeLowBattery
	}
	m.doSetMode(mode)
}

func (m *Manager) updatePowerSavingState(state bool) {
	if m.setPropPowerSavingModeAutoWhenBatteryLow(state) {
		_ = m.setDsgData(dsettingsPowerSavingModeAutoWhenBatteryLow, state, m.dsgPower)
	}

	if m.setPropPowerSavingModeAuto(state) {
		_ = m.setDsgData(dsettingsPowerSavingModeAuto, state, m.dsgPower)
	}
}
