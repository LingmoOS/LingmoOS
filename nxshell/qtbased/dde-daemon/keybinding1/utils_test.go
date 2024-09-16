// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_shouldUseDDEKwin(t *testing.T) {
	_, err := os.Stat("/usr/bin/kwin_no_scale")
	exist1 := err == nil
	exist2 := shouldUseDDEKwin()
	assert.Equal(t, exist1, exist2)
}
