// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package logined

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_Equal(t *testing.T) {

	var testSessionInfo1 = &SessionInfo{Uid: 1, Desktop: "deepin", Display: ":0", sessionPath: "/org/freedesktop/login1/session/self"}
	var testSessionInfo2 = &SessionInfo{Uid: 1, Desktop: "deepin", Display: ":0", sessionPath: "/org/freedesktop/login1/session/self"}
	var testSessionInfo3 = &SessionInfo{Uid: 1, Desktop: "uos", Display: ":0", sessionPath: "/org/freedesktop/login1/session/self"}

	assert.Equal(t, testSessionInfo1.Equal(testSessionInfo2), true)
	assert.Equal(t, testSessionInfo1.Equal(testSessionInfo3), false)
}
