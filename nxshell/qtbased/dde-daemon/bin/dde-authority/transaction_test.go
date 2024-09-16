// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_baseTransaction_GetInterfaceName(t *testing.T) {
	b := baseTransaction{}
	assert.Equal(t, dbusTxInterface, b.GetInterfaceName())
}
