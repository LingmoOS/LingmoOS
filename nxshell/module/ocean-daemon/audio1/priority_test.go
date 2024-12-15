// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_contains(t *testing.T) {
	assert.True(t, contains("hbc.abcd.1234", "world.abcd.1234", "hbc"))
	assert.True(t, contains("hello.abcd.1234", "hbc.abcd.1234", "hbc"))
	assert.True(t, contains("HBC.abcd.1234", "world.abcd.1234", "hbc"))
	assert.True(t, contains("hello.abcd.1234", "HBC.abcd.1234", "hbc"))
	assert.False(t, contains("hello.abcd.1234", "world.abcd.1234", "hbc"))
}

func Test_GetPortType(t *testing.T) {
	assert.Equal(t, GetPortType("hbc.abcd.1234", "world.abcd.1234"), PortTypeUnknown)
	assert.Equal(t, GetPortType("bluez.abcd.1234", "world.abcd.1234"), PortTypeBluetooth)
	assert.Equal(t, GetPortType("hbc.abcd.1234", "bluez.abcd.1234"), PortTypeBluetooth)
	assert.Equal(t, GetPortType("usb.abcd.1234", "world.abcd.1234"), PortTypeHeadset)
	assert.Equal(t, GetPortType("hbc.abcd.1234", "usb.abcd.1234"), PortTypeHeadset)
	assert.Equal(t, GetPortType("hello.abcd.speaker", "world.abcd.1234"), PortTypeBuiltin)
	assert.Equal(t, GetPortType("hdmi.abcd.speaker", "world.abcd.1234"), PortTypeHdmi)
}

func Test_IsInputTypeAfter(t *testing.T) {
	pr := NewPriorities()
	pr.defaultInit(CardList{})
	assert.False(t, pr.IsInputTypeAfter(PortTypeHeadset, PortTypeBluetooth))
	assert.False(t, pr.IsInputTypeAfter(PortTypeBuiltin, PortTypeBluetooth))
	assert.False(t, pr.IsInputTypeAfter(PortTypeHdmi, PortTypeBluetooth))
	assert.False(t, pr.IsInputTypeAfter(PortTypeBuiltin, PortTypeHeadset))
	assert.False(t, pr.IsInputTypeAfter(PortTypeHdmi, PortTypeBuiltin))
	assert.False(t, pr.IsInputTypeAfter(PortTypeBuiltin, PortTypeUsb))

	assert.True(t, pr.IsInputTypeAfter(PortTypeBluetooth, PortTypeHeadset))
	assert.True(t, pr.IsInputTypeAfter(PortTypeBluetooth, PortTypeBuiltin))
	assert.True(t, pr.IsInputTypeAfter(PortTypeBluetooth, PortTypeHdmi))
	assert.True(t, pr.IsInputTypeAfter(PortTypeHeadset, PortTypeBuiltin))
	assert.True(t, pr.IsInputTypeAfter(PortTypeBuiltin, PortTypeHdmi))
	assert.True(t, pr.IsInputTypeAfter(PortTypeUsb, PortTypeBuiltin))
}

func Test_IsOutputTypeAfter(t *testing.T) {
	pr := NewPriorities()
	pr.defaultInit(CardList{})
	assert.False(t, pr.IsOutputTypeAfter(PortTypeHeadset, PortTypeBluetooth))
	assert.False(t, pr.IsOutputTypeAfter(PortTypeBuiltin, PortTypeBluetooth))
	assert.False(t, pr.IsOutputTypeAfter(PortTypeHdmi, PortTypeBluetooth))
	assert.False(t, pr.IsOutputTypeAfter(PortTypeBuiltin, PortTypeHeadset))
	assert.False(t, pr.IsOutputTypeAfter(PortTypeHdmi, PortTypeBuiltin))
	assert.False(t, pr.IsOutputTypeAfter(PortTypeBuiltin, PortTypeUsb))

	assert.True(t, pr.IsOutputTypeAfter(PortTypeBluetooth, PortTypeHeadset))
	assert.True(t, pr.IsOutputTypeAfter(PortTypeBluetooth, PortTypeBuiltin))
	assert.True(t, pr.IsOutputTypeAfter(PortTypeBluetooth, PortTypeHdmi))
	assert.True(t, pr.IsOutputTypeAfter(PortTypeHeadset, PortTypeBuiltin))
	assert.True(t, pr.IsOutputTypeAfter(PortTypeBuiltin, PortTypeHdmi))
	assert.True(t, pr.IsOutputTypeAfter(PortTypeUsb, PortTypeBuiltin))
}
