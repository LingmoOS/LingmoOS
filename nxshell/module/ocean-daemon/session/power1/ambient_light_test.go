// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_calcBrWithLightLevel(t *testing.T) {
	var arr = []struct {
		lightLevel float64
		br         byte
	}{
		{-1, 0},
		{0, 0},
		{1, 2},
		{2, 3},
		{17, 29},
		{60, 48},
		{350, 62},
		{9999.9, 255},
		{10000, 255},
		{10000.1, 255},
	}

	for _, value := range arr {
		assert.Equal(t, calcBrWithLightLevel(value.lightLevel), value.br)
	}
}
