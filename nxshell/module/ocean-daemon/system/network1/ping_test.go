// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network1

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_calcCheckSum(t *testing.T) {
	var data []byte
	calcCheckSum(data)

	data = []byte{
		0x1, 0x2, 0x3,
	}
	sum := calcCheckSum(data)
	assert.Equal(t, uint16(0xfefa), sum)

	data = []byte{
		0x1, 0x2, 0x3, 0x4,
	}
	sum = calcCheckSum(data)
	assert.Equal(t, uint16(0xfbf9), sum)
}

func Test_getSequenceNum(t *testing.T) {
	num := getSequenceNum()
	assert.Equal(t, uint16(1), num)
}

func Test_unmarshalIPHeader(t *testing.T) {
	var datas []byte
	header, err := unmarshalIPHeader(datas)
	assert.Nil(t, header)
	assert.NotNil(t, err)

	datas = []byte{
		0xF1, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
		0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
		0x10,
	}

	header, err = unmarshalIPHeader(datas)
	assert.Equal(t, uint8(0x10), header.Flag)
	assert.Nil(t, err)
}
