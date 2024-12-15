// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_ipToUint32(t *testing.T) {
	tests := []struct {
		ip   string
		want uint32
	}{
		{
			ip:   "1.2.3.4",
			want: 0x01020304,
		},
		{
			ip:   "255.255.255.255",
			want: 0xffffffff,
		},
	}
	for _, tt := range tests {
		t.Run(tt.ip, func(t *testing.T) {
			got := ipToUint32(tt.ip)
			assert.Equal(t, tt.want, got)
		})
	}
}

func Test_uint32ToIP(t *testing.T) {
	type args struct {
	}
	tests := []struct {
		l    uint32
		want string
	}{
		{
			l:    0x01020304,
			want: "1.2.3.4",
		},
		{
			l:    0xffffffff,
			want: "255.255.255.255",
		},
	}
	for _, tt := range tests {
		t.Run(tt.want, func(t *testing.T) {
			got := uint32ToIP(tt.l)
			assert.Equal(t, tt.want, got)
		})
	}
}
