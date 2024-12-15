// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package gesture1

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_isInWindowBlacklist(t *testing.T) {
	slice := []string{"window1", "window2", "window3"}
	assert.True(t, isInWindowBlacklist("window1", slice))
	assert.True(t, isInWindowBlacklist("window2", slice))
	assert.True(t, isInWindowBlacklist("window3", slice))
	assert.False(t, isInWindowBlacklist("window4", slice))
}
