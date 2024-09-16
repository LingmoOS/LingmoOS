// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
package audio

import (
	"testing"

	"github.com/linuxdeepin/go-lib/pulse"
	"github.com/stretchr/testify/assert"
)

func Test_getCardName(t *testing.T) {
	type args struct {
		card *pulse.Card
	}
	tests := []struct {
		name     string
		args     args
		wantName string
	}{
		{
			name: "getCardName",
			args: args{
				card: &pulse.Card{
					Name: "abcd",
					PropList: map[string]string{
						"alsa.card_name":     "",
						"device.api":         "fdf",
						"device.description": "",
					},
				},
			},
			wantName: "abcd",
		},
		{
			name: "getCardName with alsa card name",
			args: args{
				card: &pulse.Card{
					Name: "abcd",
					PropList: map[string]string{
						"alsa.card_name":     "cdcd",
						"device.api":         "fdf",
						"device.description": "",
					},
				},
			},
			wantName: "cdcd",
		},
		{
			name: "getCardName bluez",
			args: args{
				card: &pulse.Card{
					Name: "abcd",
					PropList: map[string]string{
						"alsa.card_name":     "",
						"device.api":         "bluez",
						"device.description": "xxxxxx",
					},
				},
			},
			wantName: "xxxxxx",
		},
		{
			name: "getCardName bluez no description",
			args: args{
				card: &pulse.Card{
					Name: "abcd",
					PropList: map[string]string{
						"alsa.card_name":     "vvvvvv",
						"device.api":         "bluez",
						"device.description": "",
					},
				},
			},
			wantName: "vvvvvv",
		},
		{
			name: "getCardName bluez no description and alsa card name",
			args: args{
				card: &pulse.Card{
					Name: "abcd",
					PropList: map[string]string{
						"alsa.card_name":     "",
						"device.api":         "bluez",
						"device.description": "",
					},
				},
			},
			wantName: "abcd",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			gotName := getCardName(tt.args.card)
			assert.Equal(t, tt.wantName, gotName)
		})
	}
}
