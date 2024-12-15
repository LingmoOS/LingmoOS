// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadp

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_Crypto(t *testing.T) {
	testData := []string{
		"hubenchang@uniontech.com",
		"hubenchang0515@outlook.com",
	}

	// 没有TPM设备
	_, err := os.Stat("/dev/tpm0")
	if os.IsNotExist(err) {
		return
	}

	ctx := NewCryptoContext()
	ctx.CreateKey()

	// 加解密测试
	for _, data := range testData {
		encrypt := ctx.Encrypt([]byte(data))
		decrypt := ctx.Decrypt(encrypt)
		assert.Equal(t, data, string(decrypt))
	}

	// 持久化测试
	uadpKeyFile := "testdata/key.json"
	ctx.Save(uadpKeyFile)
	ctx.Free()
	ctx = NewCryptoContext()
	ctx.Load(uadpKeyFile)
	for _, data := range testData {
		encrypt := ctx.Encrypt([]byte(data))
		decrypt := ctx.Decrypt(encrypt)
		assert.Equal(t, data, string(decrypt))
	}

	ctx.DeleteKey()
	ctx.Free()
}
