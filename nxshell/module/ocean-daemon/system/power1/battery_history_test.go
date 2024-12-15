// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"github.com/stretchr/testify/assert"
	"testing"
)

func Test_getHistoryLength(t *testing.T) {
	bat := Battery{}
	bat.batteryHistory = []float64{1.0, 2.0}
	len := bat.getHistoryLength()
	assert.Equal(t, 2, len)
}

func Test_appendToHistory(t *testing.T) {
	bat := Battery{}

	bat.appendToHistory(1.0)
	assert.Equal(t, 1.0, bat.batteryHistory[0])

	bat.appendToHistory(2.0)
	bat.appendToHistory(3.0)
	bat.appendToHistory(4.0)
	bat.appendToHistory(5.0)
	bat.appendToHistory(6.0)
	bat.appendToHistory(7.0)
	bat.appendToHistory(8.0)
	bat.appendToHistory(9.0)
	bat.appendToHistory(10.0)
	assert.Equal(t, 10.0, bat.batteryHistory[9])
}

func Test_calcHistoryVariance(t *testing.T) {
	bat := Battery{}

	bat.batteryHistory = []float64{1.0, 2.0}
	variance := bat.calcHistoryVariance()
	assert.Equal(t, 0.25, variance)
}
