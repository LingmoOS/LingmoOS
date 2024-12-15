//go:build i386 || amd64 || arm || arm64 || mipsle || mips64le || ppc64le || riscv64 || wasm
// +build i386 amd64 arm arm64 mipsle mips64le ppc64le riscv64 wasm

// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_ntohl_le(t *testing.T) {
	tests := []struct {
		n    uint32
		want uint32
	}{
		{
			n:    0x01020304,
			want: 0x04030201,
		},
	}
	for _, tt := range tests {
		t.Run(fmt.Sprintf("0x%x", tt.n), func(t *testing.T) {
			got := ntohl(tt.n)
			assert.Equal(t, tt.want, got)
		})
	}
}

func Test_htonl_le(t *testing.T) {
	tests := []struct {
		n    uint32
		want uint32
	}{
		{
			n:    0x01020304,
			want: 0x04030201,
		},
	}
	for _, tt := range tests {
		t.Run(fmt.Sprintf("0x%x", tt.n), func(t *testing.T) {
			got := ntohl(tt.n)
			assert.Equal(t, tt.want, got)
		})
	}
}
