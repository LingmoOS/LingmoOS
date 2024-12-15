// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"strconv"
	"syscall"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_NBITS(t *testing.T) {
	var data = strconv.IntSize
	assert.Equal(t, NBITS(data), 1)
	data = strconv.IntSize + 1
	assert.Equal(t, NBITS(data), 2)
}

func Test_LONG(t *testing.T) {
	var data = 1
	assert.Equal(t, LONG(data), 0)
	data = strconv.IntSize + 1
	assert.Equal(t, LONG(data), 1)
	data = strconv.IntSize * 2
	assert.Equal(t, LONG(data), 2)
}

func Test_OFF(t *testing.T) {
	var data = 1
	assert.Equal(t, OFF(data), 1)
	data = 33
	assert.Equal(t, OFF(data), data%strconv.IntSize)
}

func Test_testBit(t *testing.T) {
	array := []int{2, 4}
	ok := testBit(1, array)
	assert.True(t, ok)

	ok = testBit(4, array)
	assert.False(t, ok)
}

func Test_upInputStrToBitmask(t *testing.T) {
	bitmask := []int{20}
	numBitsSet := upInputStrToBitmask("a", bitmask)
	assert.Equal(t, 2, numBitsSet)
}

func Test_String(t *testing.T) {
	time := syscall.Timeval{
		Sec:  1,
		Usec: 1,
	}
	ev := InputEvent{
		Time:  time,
		Type:  1,
		Code:  1,
		Value: 1,
	}

	evString := ev.String()
	assert.Equal(t, "event at 1.1, code 01, type 01, val 01", evString)
}
