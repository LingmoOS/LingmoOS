// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadp

import (
	"bytes"
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
)

type AesContext struct {
}

func NewAesContext() *AesContext {
	return &AesContext{}
}

func (ctx *AesContext) GenKey() []byte {
	key := make([]byte, 16)
	_, err := rand.Read(key)
	if err != nil {
		logger.Warning(err)
	}

	return key
}

func (ctx *AesContext) Padding(src []byte, blockSize int) []byte {
	padNum := blockSize - len(src)%blockSize
	pad := bytes.Repeat([]byte{byte(padNum)}, padNum)
	return append(src, pad...)
}

func (ctx *AesContext) Unpadding(src []byte) []byte {
	n := len(src)
	unPadNum := int(src[n-1])
	return src[:n-unPadNum]
}

func (ctx *AesContext) Encrypt(src []byte, key []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}
	src = ctx.Padding(src, block.BlockSize())
	blockMode := cipher.NewCBCEncrypter(block, key)
	blockMode.CryptBlocks(src, src)
	return src, nil
}

func (ctx *AesContext) Decrypt(src []byte, key []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}
	blockMode := cipher.NewCBCDecrypter(block, key)
	blockMode.CryptBlocks(src, src)
	src = ctx.Unpadding(src)
	return src, nil
}
