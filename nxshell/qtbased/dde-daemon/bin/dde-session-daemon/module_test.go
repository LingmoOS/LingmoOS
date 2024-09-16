// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_isStrInList(t *testing.T) {
	type args struct {
		item string
		list []string
	}
	tests := []struct {
		name string
		args args
		want bool
	}{
		{
			name: "isStrInList",
			args: args{
				item: "abc",
				list: []string{"abc", "dev"},
			},
			want: true,
		},
		{
			name: "isStrInList false",
			args: args{
				item: "abcd",
				list: []string{"abc", "dev"},
			},
			want: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := isStrInList(tt.args.item, tt.args.list)
			assert.Equal(t, tt.want, got)
		})
	}
}
