// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestDaemon_GetDependencies(t *testing.T) {
	d := Daemon{}
	assert.ElementsMatch(t, []string{}, d.GetDependencies())
}
