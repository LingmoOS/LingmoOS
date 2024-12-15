// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_doBluetoothGetDeviceTechnologies(t *testing.T) {
	type args struct {
		filename string
	}
	tests := []struct {
		name    string
		args    args
		want    []string
		wantErr bool
	}{
		{
			name: "doBluetoothGetDeviceTechnologies",
			args: args{
				filename: "./testdata/doBluetoothGetDeviceTechnologies/info1",
			},
			want:    []string{"abc", "ccc"},
			wantErr: false,
		},
		{
			name: "doBluetoothGetDeviceTechnologies not exists",
			args: args{
				filename: "./testdata/nonono",
			},
			want:    []string{},
			wantErr: true,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := doBluetoothGetDeviceTechnologies(tt.args.filename)
			if tt.wantErr {
				assert.NotNil(t, err)
				return
			}

			assert.Nil(t, err)
			assert.ElementsMatch(t, tt.want, got)
		})
	}
}
