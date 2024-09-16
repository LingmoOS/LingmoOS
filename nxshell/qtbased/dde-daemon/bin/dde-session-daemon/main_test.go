// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_allowRun(t *testing.T) {
	type env struct {
		name  string
		value string
	}
	tests := []struct {
		name string
		env  []env
		want bool
	}{
		{
			name: "allowRun",
			env: []env{
				{
					name:  "DDE_SESSION_PROCESS_COOKIE_ID",
					value: "1",
				},
			},
			want: true,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			for _, e := range tt.env {
				os.Setenv(e.name, e.value)
			}

			got := allowRun()
			assert.Equal(t, tt.want, got)

			for _, e := range tt.env {
				os.Unsetenv(e.name)
			}
		})
	}
}
