// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_writeDConfigLog(t *testing.T) {
	t.Run("Test writeDConfigLog", func(t *testing.T) {
		c := newDconfigLogCollector()
		assert.NoError(t, c.writeDConfigLog(&configInfo{Tid: ConfigTid}))
	})
}
