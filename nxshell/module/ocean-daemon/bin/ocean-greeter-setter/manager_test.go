// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestManager_GetInterfaceName(t *testing.T) {
	m := Manager{}
	assert.Equal(t, dbusInterface, m.GetInterfaceName())
}

func Test_isInteger(t *testing.T) {
	type args struct {
		str string
	}
	tests := []struct {
		name string
		args args
		want bool
	}{
		{
			name: "isInteger",
			args: args{
				str: "123",
			},
			want: true,
		},
		{
			name: "isInteger false",
			args: args{
				str: "123a",
			},
			want: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := isInteger(tt.args.str)
			assert.Equal(t, tt.want, got)
		})
	}
}
