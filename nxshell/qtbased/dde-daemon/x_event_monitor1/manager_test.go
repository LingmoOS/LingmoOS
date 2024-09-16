// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package x_event_monitor

import (
	"fmt"

	"github.com/linuxdeepin/go-lib/strv"
	"github.com/stretchr/testify/assert"

	"testing"
)

func Test_isPidAreaRegistered(t *testing.T) {
	m := Manager{}
	str0 := []string{"tong", "xin"}
	str1 := []string{"ruan", "jian"}
	m.pidAidsMap = map[uint32]strv.Strv{
		0: str0,
		1: str1,
	}

	assert.True(t, m.isPidAreaRegistered(0, "tong"))
}

func Test_getIdList(t *testing.T) {
	m := Manager{}
	var area = []coordinateRange{
		{100, 100, 200, 200},
	}
	var key = "tongxin"

	var coordinateInfo_ = coordinateInfo{
		areas:        area,
		moveIntoFlag: false,
		motionFlag:   false,
		buttonFlag:   false,
		keyFlag:      false,
	}

	m.idAreaInfoMap = map[string]*coordinateInfo{
		key: &coordinateInfo_,
	}

	inList, _ := m.getIdList(101, 101)

	for i, in := range inList {
		assert.Equal(t, 0, i)
		assert.Equal(t, key, in)
	}

	_, outList := m.getIdList(99, 99)
	for i, out := range outList {
		assert.Equal(t, 0, i)
		assert.Equal(t, key, out)
	}
}

func Test_sumAreasMd5(t *testing.T) {
	m := Manager{}
	var areasNil []coordinateRange
	var expectMd5 = "eef73b4ff31a5d5e32c54719fee950c7"

	md5Str, ok := m.sumAreasMd5(areasNil, 1)
	assert.False(t, ok)
	assert.Equal(t, md5Str, "")

	areas := []coordinateRange{
		{100, 200, 100, 200},
	}

	md5Str, ok = m.sumAreasMd5(areas, 1)
	assert.Equal(t, expectMd5, md5Str)
	assert.True(t, ok)
}

func Test_DebugGetPidAreasMap(t *testing.T) {
	m := Manager{}
	str0 := []string{"tong", "xin"}
	str1 := []string{"ruan", "jian"}
	var expectRtn string = "{\"0\":[\"tong\",\"xin\"],\"1\":[\"ruan\",\"jian\"]}"
	m.pidAidsMap = map[uint32]strv.Strv{
		0: str0,
		1: str1,
	}

	rtnStr, err := m.DebugGetPidAreasMap()

	assert.Nil(t, err)
	assert.Equal(t, expectRtn, rtnStr)
	fmt.Printf("rtnStr:%s\n", rtnStr)
}
