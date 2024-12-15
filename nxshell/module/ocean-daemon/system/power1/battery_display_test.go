// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_rightPercentage(t *testing.T) {
	var data float64
	data = -50.75
	assert.Equal(t, rightPercentage(data), 0.0)
	data = 66.66
	assert.Equal(t, rightPercentage(data), 66.66)
	data = 123.111
	assert.Equal(t, rightPercentage(data), 100.0)
}

func Test_changeBatteryLowByBatteryPercentage(t *testing.T) {
	m := Manager{}

	percentage := float64(m.PowerSavingModeAutoBatteryPercent - 1)
	m.changeBatteryLowByBatteryPercentage(percentage)

	percentage = float64(m.PowerSavingModeAutoBatteryPercent + 1)
	m.changeBatteryLowByBatteryPercentage(percentage)

	percentage = float64(m.PowerSavingModeAutoBatteryPercent + 1)
	m.batteryLow = false
	m.changeBatteryLowByBatteryPercentage(percentage)

	percentage = float64(m.PowerSavingModeAutoBatteryPercent + 1)
	m.batteryLow = true
	m.changeBatteryLowByBatteryPercentage(percentage)
}

func Test_ManagerSimpleFunc(t *testing.T) {
	m := Manager{}

	m.resetBatteryDisplay()
}
