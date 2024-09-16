// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package users

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_isStrInArray(t *testing.T) {

	var var1 = []string{"/usr/share", "/ect/passwd", "/etc/cpus"}
	assert.Equal(t, isStrInArray("/etc/cpus", var1), true)
	assert.Equal(t, isStrInArray("/etc/abc", var1), false)
}

func Test_strToInt(t *testing.T) {

	fmt.Println(strToInt("4", -1))

	assert.Equal(t, strToInt("4", -1), 4)
	assert.Equal(t, strToInt("\\4", -1), -1)
	assert.Equal(t, strToInt("-2", -1), -2)

}

func Test_getDeepinReleaseType(t *testing.T) {

	var versions = []string{"Desktop", "Professional",
		"Server", "Personal",
		"",
	}

	assert.Contains(t, versions, systemType())
}
