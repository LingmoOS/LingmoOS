// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
package accounts

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getUidFromUserPath(t *testing.T) {
	tests := []struct {
		name     string
		userPath string
		want     string
	}{
		{
			name:     "getUidFromUserPath",
			userPath: "/org/deepin/dde/Accounts1/User1000",
			want:     "1000",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := getUidFromUserPath(tt.userPath)
			assert.Equal(t, tt.want, got)
		})
	}
}
