//go:build mips || mips64 || ppc64 || s390x
// +build mips mips64 ppc64 s390x

// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_ntohl_be(t *testing.T) {
	tests := []struct {
		n    uint32
		want uint32
	}{
		{
			n:    0x01020304,
			want: 0x01020304,
		},
	}
	for _, tt := range tests {
		t.Run(fmt.Sprintf("0x%x", tt.n), func(t *testing.T) {
			got := ntohl(tt.n)
			assert.Equal(t, tt.want, got)
		})
	}
}

func Test_htonl_be(t *testing.T) {
	tests := []struct {
		n    uint32
		want uint32
	}{
		{
			n:    0x01020304,
			want: 0x01020304,
		},
	}
	for _, tt := range tests {
		t.Run(fmt.Sprintf("0x%x", tt.n), func(t *testing.T) {
			got := ntohl(tt.n)
			assert.Equal(t, tt.want, got)
		})
	}
}
