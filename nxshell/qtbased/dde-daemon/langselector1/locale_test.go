// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package langselector

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_GenerateLocaleEnvFile(t *testing.T) {
	example := `LANG=en_US.UTF-8
LANGUAGE=en_US
`
	assert.Equal(t, string(generateLocaleEnvFile("en_US.UTF-8",
		"testdata/pam_environment")), example)
}

func Test_GetLocale(t *testing.T) {
	l, err := getLocaleFromFile("testdata/pam_environment")
	assert.NoError(t, err)
	assert.Equal(t, l, "zh_CN.UTF-8")

	l = getCurrentUserLocale()
	assert.NotEqual(t, len(l), 0)
}

func Test_WriteUserLocale(t *testing.T) {
	assert.Nil(t, writeLocaleEnvFile("zh_CN.UTF-8", "testdata/pam_environment", "testdata/pam"))
	os.RemoveAll("testdata/pam")
}
