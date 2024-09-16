// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestSessionDaemon_GetInterfaceName(t *testing.T) {
	s := SessionDaemon{}
	assert.Equal(t, dbusInterface, s.GetInterfaceName())
}

func TestFilterList(t *testing.T) {
	var infos = []struct {
		origin    []string
		condition []string
		ret       []string
	}{
		{
			origin:    []string{"power", "audio", "dock"},
			condition: []string{"power", "dock"},
			ret:       []string{"audio"},
		},
		{
			origin:    []string{"power", "audio", "dock"},
			condition: []string{},
			ret:       []string{"power", "audio", "dock"},
		},
		{
			origin:    []string{"power", "audio", "dock"},
			condition: []string{"power", "dock", "audio"},
			ret:       []string(nil),
		},
	}

	for _, info := range infos {
		assert.ElementsMatch(t, info.ret, filterList(info.origin, info.condition))
	}
}
