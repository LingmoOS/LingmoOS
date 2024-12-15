// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getLingmoReleaseType(t *testing.T) {

	var versions = []string{"Desktop", "Professional",
		"Server", "Personal",
		"",
	}

	assert.Contains(t, versions, getLingmoReleaseType())
}
