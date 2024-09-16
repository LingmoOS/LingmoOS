// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_toPinyinAndAbbr(t *testing.T) {
	type dataStruct struct {
		origin     string
		pinyin     string
		shortening string
	}

	testData := []dataStruct{
		{"音乐", "yinyue", "yy"},
		{"计算机", "jisuanji", "jsj"},
		{"Qt 5 设计器", "Qt5shejiqi", "Qt5sjq"},
		{"VmWare Workstation", "VmWareWorkstation", "VmWareWorkstation"},
		{"is 奔图 printer 打印机", "isbentuprinterdayinji", "isbtprinterdyj"},
		{"木头人", "mutouren", "mtr"},
	}
	for _, data := range testData {
		pinyin, shortening := toPinyinAndAbbr(data.origin)
		assert.Equal(t, pinyin, data.pinyin, "they should be equal")
		assert.Equal(t, shortening, data.shortening, "they should be equal")
	}
}
