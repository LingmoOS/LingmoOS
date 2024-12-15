// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getPortByName(t *testing.T) {
	type args struct {
		ports []Port
		name  string
	}
	tests := []struct {
		name  string
		args  args
		want  Port
		want1 bool
	}{
		{
			name: "getPortByName",
			args: args{
				ports: []Port{
					{
						Name:        "a",
						Description: "xxx",
					},
					{
						Name:        "b",
						Description: "xxx",
					},
					{
						Name:        "c",
						Description: "xxx",
					},
				},
				name: "b",
			},
			want: Port{
				Name:        "b",
				Description: "xxx",
			},
			want1: true,
		},
		{
			name: "getPortByName not found",
			args: args{
				ports: []Port{
					{
						Name:        "a",
						Description: "xxx",
					},
					{
						Name:        "b",
						Description: "xxx",
					},
					{
						Name:        "c",
						Description: "xxx",
					},
				},
				name: "d",
			},
			want:  Port{},
			want1: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, got1 := getPortByName(tt.args.ports, tt.args.name)
			assert.Equal(t, tt.want, got)
			assert.Equal(t, tt.want1, got1)
		})
	}
}

func TestPort_String(t *testing.T) {
	tests := []struct {
		name string
		port *Port
		want string
	}{
		{
			name: "Port_String",
			port: &Port{
				Name:        "abc",
				Description: "def",
				Available:   0,
			},
			want: `<Port name="abc" desc="def" available=Unknown>`,
		},
		{
			name: "Port_String",
			port: &Port{
				Name:        "abc",
				Description: "def",
				Available:   1,
			},
			want: `<Port name="abc" desc="def" available=No>`,
		},
		{
			name: "Port_String",
			port: &Port{
				Name:        "abc",
				Description: "def",
				Available:   2,
			},
			want: `<Port name="abc" desc="def" available=Yes>`,
		},
		{
			name: "Port_String",
			port: nil,
			want: `<nil>`,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := tt.port.String()
			assert.Equal(t, tt.want, got)
		})
	}
}
