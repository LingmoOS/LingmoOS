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
			candidateID: "com.lingmo.Control_Center",
			want:        "com.lingmo.control-center",
		},
		{
			name:        "NormalizeAppID",
			candidateID: "com.lingmo.ControlCenter",
			want:        "com.lingmo.controlcenter",
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
			candidateID: "com.lingmo.Control_Center",
			want:        "com.lingmo.Control-Center",
		},
		{
			name:        "NormalizeAppIDWithCaseSensitive not change",
			candidateID: "com.lingmo.ControlCenter",
			want:        "com.lingmo.ControlCenter",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := NormalizeAppIDWithCaseSensitive(tt.candidateID)
			assert.Equal(t, tt.want, got)
		})
	}
}
