// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_checkTimeStabilized(t *testing.T) {
	data := []uint64{
		9455,
		5467,
		3840,
		2962,
		2408,
		2408,
		1754,
		1698,
		1710,
		1675,
	}

	assert.False(t, checkTimeStabilized(data[:2], data[2]))
	assert.False(t, checkTimeStabilized(data[:6], data[6]))
	assert.True(t, checkTimeStabilized(data[:9], data[9]))
}
