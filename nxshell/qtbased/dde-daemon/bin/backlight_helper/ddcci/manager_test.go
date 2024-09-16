// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package ddcci

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestManager_GetInterfaceName(t *testing.T) {
	m := Manager{}
	assert.Equal(t, dbusInterface, m.GetInterfaceName())
}

func TestManager_RefreshDisplays(t *testing.T) {
	m := Manager{}
	assert.Nil(t, m.RefreshDisplays())
}
