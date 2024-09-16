// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package appinfo

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestConfigFilePath(t *testing.T) {
	type args struct {
		name string
	}
	tests := []struct {
		name       string
		configHome string
		args       args
		want       string
	}{
		{
			name:       "ConfigFilePath",
			configHome: "../testdata/.config",
			args:       args{name: "launcher/test.ini"},
			want:       "../testdata/.config/launcher/test.ini",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			os.Setenv("XDG_CONFIG_HOME", tt.configHome)
			defer os.Unsetenv("XDG_CONFIG_HOME")

			got := ConfigFilePath(tt.args.name)
			assert.Equal(t, tt.want, got)
		})
	}
}
