// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
package appinfo

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestNormalizeAppID(t *testing.T) {
	tests := []struct {
		name        string
		candidateID string
		want        string
	}{
		{
			name:        "NormalizeAppID",
			candidateID: "com.deepin.Control_Center",
			want:        "com.deepin.control-center",
		},
		{
			name:        "NormalizeAppID",
			candidateID: "com.deepin.ControlCenter",
			want:        "com.deepin.controlcenter",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := NormalizeAppID(tt.candidateID)
			assert.Equal(t, tt.want, got)
		})
	}
}

func TestNormalizeAppIDWithCaseSensitive(t *testing.T) {
	tests := []struct {
		name        string
		candidateID string
		want        string
	}{
		{
			name:        "NormalizeAppIDWithCaseSensitive",
			candidateID: "com.deepin.Control_Center",
			want:        "com.deepin.Control-Center",
		},
		{
			name:        "NormalizeAppIDWithCaseSensitive not change",
			candidateID: "com.deepin.ControlCenter",
			want:        "com.deepin.ControlCenter",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := NormalizeAppIDWithCaseSensitive(tt.candidateID)
			assert.Equal(t, tt.want, got)
		})
	}
}
