// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestAuthority_GetInterfaceName(t *testing.T) {
	a := Authority{}
	assert.Equal(t, dbusInterface, a.GetInterfaceName())
}
