// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package checkers

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_isStrInArray(t *testing.T) {
	type args struct {
		str   string
		array []string
	}
	var a = []args{
		{"uniontech", []string{"uniontech", "aaaaa", "bbbbb"}},
		{"aaaaa", []string{"uniontec", "aaaaa", "bbbbb"}},
		{"bbbbb", []string{"uniontec", "aaaaa", "bbbbb"}},
	}
	for _, array := range a {
		assert.Equal(t, isStrInArray(array.str, array.array), true)
	}
}
