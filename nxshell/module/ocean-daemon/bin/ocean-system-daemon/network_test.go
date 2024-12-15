// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func Test_getConnectionFiles(t *testing.T) {
	type args struct {
		dir string
	}
	tests := []struct {
		name    string
		args    args
		want    []string
		wantErr bool
	}{
		{
			name: "getConnectionFiles",
			args: args{
				dir: "testdata/getConnectionFiles",
			},
			want:    []string{"testdata/getConnectionFiles/abc", "testdata/getConnectionFiles/ddd"},
			wantErr: false,
		},
		{
			name: "getConnectionFiles not found",
			args: args{
				dir: "testdata/ccc",
			},
			want:    nil,
			wantErr: true,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := getConnectionFiles(tt.args.dir)
			if tt.wantErr {
				assert.NotNil(t, err)
				return
			}

			require.Nil(t, err)
			assert.ElementsMatch(t, tt.want, got)
		})
	}
}
