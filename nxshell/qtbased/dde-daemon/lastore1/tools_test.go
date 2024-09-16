// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package lastore

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

// Hook up gocheck into the "go test" runner.

func TestStrSliceSetEqual(t *testing.T) {
	assert.Equal(t, strSliceSetEqual([]string{}, []string{}), true)
	assert.Equal(t, strSliceSetEqual([]string{"a"}, []string{"a"}), true)
	assert.Equal(t, strSliceSetEqual([]string{"a", "b"}, []string{"a"}), false)
	assert.Equal(t, strSliceSetEqual([]string{"a", "b", "d"}, []string{"b", "d", "a"}), true)
}
