// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestManager_GetInterfaceName(t *testing.T) {
	m := Manager{}
	assert.Equal(t, dbusInterface, m.GetInterfaceName())
}

func Test_getBrightnessFilename(t *testing.T) {
	type args struct {
		type0 byte
		name  string
	}
	tests := []struct {
		name    string
		args    args
		want    string
		wantErr bool
	}{
		{
			name: "getBrightnessFilename backlight",
			args: args{
				type0: DisplayBacklight,
				name:  "xx",
			},
			wantErr: false,
			want:    "/sys/class/backlight/xx/brightness",
		},
		{
			name: "getBrightnessFilename keyboard",
			args: args{
				type0: KeyboardBacklight,
				name:  "xx",
			},
			wantErr: false,
			want:    "/sys/class/leds/xx/brightness",
		},
		{
			name: "getBrightnessFilename wrong type",
			args: args{
				type0: 3,
				name:  "xx",
			},
			wantErr: true,
		},
		{
			name: "getBrightnessFilename bad name",
			args: args{
				type0: DisplayBacklight,
				name:  "xx/",
			},
			wantErr: true,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := getBrightnessFilename(tt.args.type0, tt.args.name)
			if tt.wantErr {
				assert.NotNil(t, err)
				return
			}

			require.Nil(t, err)
			assert.Equal(t, tt.want, got)
		})
	}
}
