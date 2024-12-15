// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_writeLoginLog(t *testing.T) {
	t.Run("Test writeLoginLog", func(t *testing.T) {
		c := new(loginEventCollector)
		assert.NoError(t, c.writeLoginLog(&loginTimeInfo{Tid: SystemBootTid}))
	})
}

func Test_genLastRebootCmd(t *testing.T) {
	t.Run("Test genLastRebootCmd", func(t *testing.T) {
		cmd := genLastRebootCmd()
		assert.Equal(t, "/bin/bash -c last -x reboot --time-format iso | awk 'NR==2 {print}'", cmd.String())
	})
}

func Test_parseLastRebootCmdOutPut(t *testing.T) {
	t.Run("Test parseLastRebootCmdOutPut", func(t *testing.T) {
		_, err := parseLastRebootCmdOutPut([]byte("reboot   system boot  4.19.0-amd64-des 2022-08-15T14:13:35+08:00 - 2022-08-15T17:03:29+08:00  (02:49)"))
		assert.NoError(t, err)
	})
}
