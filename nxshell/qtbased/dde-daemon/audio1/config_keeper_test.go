// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
package audio

import (
	"io/ioutil"
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestConfigKeeper_Save(t *testing.T) {
	type fields struct {
		file     string
		muteFile string
		Cards    map[string]*CardConfig
	}
	tests := []struct {
		name        string
		fields      fields
		wantErr     bool
		fileContent string
	}{
		{
			name: "ConfigKeeper_Save",
			fields: fields{
				file:     "./testdata/ConfigKeeper_Save",
				muteFile: "./testdata/ConfigKeeperMute_Save",
				Cards: map[string]*CardConfig{
					"one": {
						Name:  "xxx",
						Ports: map[string]*PortConfig{},
					},
				},
			},
			wantErr: false,
			fileContent: `{
  "one": {
    "Name": "xxx",
    "Ports": {}
  }
}`,
		},
		{
			name: "ConfigKeeper_Save empty",
			fields: fields{
				file:     "./testdata/ConfigKeeper_Save",
				muteFile: "./testdata/ConfigKeeperMute_Save",
				Cards:    map[string]*CardConfig{},
			},
			wantErr:     false,
			fileContent: "{}",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			ck := &ConfigKeeper{
				file:     tt.fields.file,
				muteFile: tt.fields.muteFile,
				Cards:    tt.fields.Cards,
			}
			err := ck.Save()
			if tt.wantErr {
				assert.Error(t, err)
				return
			}

			assert.NoError(t, err)

			s, err := os.Stat(tt.fields.file)
			require.NoError(t, err)
			assert.Equal(t, 0644, int(s.Mode())&0777)

			content, err := ioutil.ReadFile(tt.fields.file)
			require.NoError(t, err)
			assert.Equal(t, tt.fileContent, string(content))

			os.Remove(tt.fields.file)
		})
	}
}
