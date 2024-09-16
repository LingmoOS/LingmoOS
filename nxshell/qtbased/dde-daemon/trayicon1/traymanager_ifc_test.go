// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package trayicon

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestTrayManager_GetInterfaceName(t *testing.T) {
	m := TrayManager{}
	assert.Equal(t, dbusInterface, m.GetInterfaceName())
}
