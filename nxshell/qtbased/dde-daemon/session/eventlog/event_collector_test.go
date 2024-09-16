// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"os"
	"testing"

	dutils "github.com/linuxdeepin/go-lib/utils"
	"github.com/stretchr/testify/assert"
)

func Test_getUserExpStateFromUserExpPath(t *testing.T) {
	t.Run("Test getUserExpStateFromUserExpPath", func(t *testing.T) {
		userExpPath = "testdata/testuser"
		e := new(EventLog)
		if !dutils.IsFileExist(userExpPath) {
			assert.False(t, e.getUserExpStateFromUserExpPath())
		} else {
			assert.True(t, e.getUserExpStateFromUserExpPath())
		}
	})
}

func Test_getUserExpStateFromDefaultPath(t *testing.T) {
	t.Run("Test getUserExpStateFromDefaultPath", func(t *testing.T) {
		e := new(EventLog)
		defaultExpPath = "testdata/testdefault"
		if !dutils.IsFileExist(defaultExpPath) {
			assert.False(t, e.getUserExpStateFromDefaultPath())
		} else {
			assert.True(t, e.getUserExpStateFromDefaultPath())
		}
	})

}

func Test_setUserExpFileState(t *testing.T) {
	t.Run("Test setUserExpFileState", func(t *testing.T) {
		e := new(EventLog)
		e.currentUserVarTmpExpPath = "testdata/current_testuser"
		assert.NoError(t, e.setUserExpFileState(true))
		_ = os.RemoveAll(e.currentUserVarTmpExpPath)
	})
}
