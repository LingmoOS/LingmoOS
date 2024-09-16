// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package apps1

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getHomeByUid(t *testing.T) {
	uid := os.Getuid()
	home, err := getHomeByUid(uid)
	assert.NoError(t, err)
	assert.Equal(t, home, os.Getenv("HOME"))
}

func Test_isDesktopFile(t *testing.T) {
	type args struct {
		path string
	}
	tests := []struct {
		name string
		args args
		want bool
	}{
		{
			name: "isDesktopFile",
			args: args{
				path: "/usr/share/applications/firefox.desktop",
			},
			want: true,
		},
		{
			name: "isDesktopFile not a desktop",
			args: args{
				path: "/usr/share/applications/firefox.xxx",
			},
			want: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := isDesktopFile(tt.args.path)
			assert.Equal(t, tt.want, got)
		})
	}
}

func Test_removeDesktopExt(t *testing.T) {
	type args struct {
		name string
	}
	tests := []struct {
		name string
		args args
		want string
	}{

		{
			name: "removeDesktopFile",
			args: args{
				name: "firefox.desktop",
			},
			want: "firefox",
		},
		{
			name: "removeDesktopFile not a desktop",
			args: args{
				name: "firefox.xxx",
			},
			want: "firefox.xxx",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := removeDesktopExt(tt.args.name)
			assert.Equal(t, tt.want, got)
		})
	}
}
