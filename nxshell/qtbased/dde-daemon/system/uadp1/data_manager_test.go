// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadp

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_DataManager(t *testing.T) {
	testData := map[string]string{
		"hello": "hubenchang@uniontech.com",
		"world": "hubenchang0515@outlook.com",
	}

	const uadpDataDir = "testdata/data/"
	const uadpDataMap = "testdata/data.json"
	const aesKey = "0123456789abcdef"

	dm := NewDataManager(uadpDataDir)

	for k, v := range testData {
		err := dm.SetData(uadpDataDir, "dde.out", k, []byte(aesKey), []byte(v))
		assert.NoError(t, err)
	}

	for k, v := range testData {
		key, data, err := dm.GetData("dde.out", k)
		assert.NoError(t, err)
		assert.Equal(t, aesKey, string(key))
		assert.Equal(t, v, string(data))
	}

	dm.Save(uadpDataMap)
	dm.Load(uadpDataMap)

	for k, v := range testData {
		key, data, err := dm.GetData("dde.out", k)
		assert.NoError(t, err)
		assert.Equal(t, aesKey, string(key))
		assert.Equal(t, v, string(data))
	}

	assert.Equal(t, len(testData), len(dm.ListName("dde.out")))
	dm.DeleteProcess("dde.out")
	assert.Equal(t, 0, len(dm.ListName("dde.out")))
}
