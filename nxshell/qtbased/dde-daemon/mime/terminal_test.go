// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package mime

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getPresetTerminalPath(t *testing.T) {
	type env struct {
		name  string
		value string
	}
	type test struct {
		name string
		env  []env
		want string
	}

	tests := []test{
		func() test {
			p, _ := filepath.Abs("./testdata/getTerminalPath/notfound")

			return test{
				name: "getTerminalPath",
				env: []env{
					{
						name:  "PATH",
						value: p,
					},
				},
				want: "",
			}
		}(),
		func() test {
			p, _ := filepath.Abs("./testdata/getTerminalPath/path1")

			return test{
				name: "getTerminalPath",
				env: []env{
					{
						name:  "PATH",
						value: p,
					},
				},
				want: filepath.Join(p, "terminator"),
			}
		}(),
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			for _, e := range tt.env {
				os.Setenv(e.name, e.value)
			}

			got := GetPresetTerminalPath()
			assert.Equal(t, tt.want, got)

			for _, e := range tt.env {
				os.Unsetenv(e.name)
			}
		})
	}
}
