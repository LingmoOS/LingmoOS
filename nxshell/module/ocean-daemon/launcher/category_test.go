// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getXCategory(t *testing.T) {
	assert.Equal(t, CategoryOthers, getXCategory(nil))
	assert.Equal(t, CategoryMusic, getXCategory([]string{"audio"}))
	assert.Equal(t, CategoryVideo, getXCategory([]string{"video"}))
	assert.Equal(t, CategoryVideo, getXCategory([]string{"audiovideo", "player", "recorder"}))
}
