// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadp

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path/filepath"
	"unsafe"

	dutils "github.com/linuxdeepin/go-lib/utils"
)

// #include "crypto.h"
// #include <string.h>
// #include <stdlib.h>
// #cgo LDFLAGS: -ldl
// #cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
import "C"

const uadpDir = "/usr/share/uadp/"
const uadpKeyFile = uadpDir + "key.json"

type CryptoContext struct {
	handle       C.TC_HANDLE
	PrimaryIndex C.uint
	KeyIndex     C.uint
	UUID         string
}

func uuid(n int) string {
	id := dutils.GenUuid()
	if n < len(id) {
		return id[:n]
	}
	return id
}

// 创建一个加解密上下文
func NewCryptoContext() *CryptoContext {
	ctx := CryptoContext{nil, 0, 0, ""}

	// 不能合并，代码使用了 C 语言和 Go 语言混合编程导致类型不匹配
	// mismatched types bool and _Ctype__Bool
	if !dutils.IsFileExist("/sys/class/tpm/tpm0/device/description") {
		ctx.handle = nil
		return &ctx
	}

	if !C.cryptoInit(&ctx.handle) {
		ctx.handle = nil
	}

	return &ctx
}

// 判断是否可用
func (ctx *CryptoContext) Available() bool {
	return ctx.handle != nil &&
		ctx.KeyIndex != 0 &&
		ctx.UUID != ""
}

// 保存密钥
func (ctx *CryptoContext) Save(file string) bool {
	data, err := json.MarshalIndent(ctx, "", "    ")
	if err != nil {
		logger.Warning(err)
		return false
	}

	_, err = os.Stat(filepath.Dir(file))
	if os.IsNotExist(err) {
		err = os.MkdirAll(filepath.Dir(file), 0700)
		if err != nil {
			logger.Warning(err)
			return false
		}
	}

	err = ioutil.WriteFile(file, data, 0600)
	if err != nil {
		logger.Warning(err)
		return false
	}

	logger.Debugf("%s:\n%s", file, string(data))
	return true
}

// 加载密钥
func (ctx *CryptoContext) Load(file string) bool {
	data, err := ioutil.ReadFile(file)
	if err != nil {
		logger.Debugf("%s not exist, create it", file)
		return false
	}

	err = json.Unmarshal(data, ctx)
	if err != nil {
		logger.Debugf("%s unable to unmarshal, create it", file)
		return false
	}

	logger.Debugf("get key index: 0x%x", ctx.KeyIndex)
	return true
}

// 创建密钥
func (ctx *CryptoContext) CreateKey() bool {
	ctx.UUID = uuid(8)
	var auth C.TC_BUFFER
	auth.size = C.uint(len(ctx.UUID))
	auth.buffer = (*C.uchar)(C.malloc(C.size_t(auth.size)))
	if auth.size > 0 {
		C.memcpy(unsafe.Pointer(auth.buffer), unsafe.Pointer(&([]byte)(ctx.UUID)[0]), C.size_t(auth.size))
	}

	ret := C.cryptoCreateKey(ctx.handle, &ctx.PrimaryIndex, &ctx.KeyIndex, &auth)
	C.free(unsafe.Pointer(auth.buffer))
	logger.Debugf("key: 0x%x uuid: %s", ctx.KeyIndex, ctx.UUID)
	return bool(ret)
}

// 删除密钥
func (ctx *CryptoContext) DeleteKey() {
	if !ctx.Available() {
		return
	}
	C.cryptoDeleteKey(ctx.handle, ctx.KeyIndex)
}

// 释放上下文
func (ctx *CryptoContext) Free() {
	if !ctx.Available() {
		return
	}
	C.cryptoFree(&ctx.handle)
}

// 进行加密
func (ctx *CryptoContext) Encrypt(data []byte) []byte {
	if !ctx.Available() {
		return []byte("")
	}

	if len(data) == 0 {
		return []byte("")
	}

	// 输入数据
	var input C.TC_BUFFER
	input.size = C.uint(len(data))
	input.buffer = (*C.uchar)(C.malloc(C.size_t(input.size)))
	if input.size > 0 {
		C.memcpy(unsafe.Pointer(input.buffer), unsafe.Pointer(&data[0]), C.size_t(input.size))
	}

	// 认证信息
	var auth C.TC_BUFFER
	auth.size = C.uint(len(ctx.UUID))
	auth.buffer = (*C.uchar)(C.malloc(C.size_t(auth.size)))
	if auth.size > 0 {
		C.memcpy(unsafe.Pointer(auth.buffer), unsafe.Pointer(&([]byte)(ctx.UUID)[0]), C.size_t(auth.size))
	}

	// 加密
	ret := []byte("")
	var output C.TC_BUFFER
	if C.cryptoEncrypt(ctx.handle, ctx.KeyIndex, &input, &output, &auth) {
		ret = C.GoBytes(unsafe.Pointer(output.buffer), C.int(output.size))
	}
	C.free(unsafe.Pointer(input.buffer))
	C.free(unsafe.Pointer(output.buffer))
	C.free(unsafe.Pointer(auth.buffer))

	return ret
}

// 进行解密
func (ctx *CryptoContext) Decrypt(data []byte) []byte {
	if !ctx.Available() {
		return []byte("")
	}

	if len(data) == 0 {
		return []byte("")
	}

	// 输入数据
	var input C.TC_BUFFER
	input.size = C.uint(len(data))
	input.buffer = (*C.uchar)(C.malloc(C.size_t(input.size)))
	if input.size > 0 {
		C.memcpy(unsafe.Pointer(input.buffer), unsafe.Pointer(&data[0]), C.size_t(input.size))
	}

	// 认证信息
	var auth C.TC_BUFFER
	auth.size = C.uint(len(ctx.UUID))
	auth.buffer = (*C.uchar)(C.malloc(C.size_t(auth.size)))
	if auth.size > 0 {
		C.memcpy(unsafe.Pointer(auth.buffer), unsafe.Pointer(&([]byte)(ctx.UUID)[0]), C.size_t(auth.size))
	}

	// 解密
	ret := []byte("")
	var output C.TC_BUFFER
	if C.cryptoDecrypt(ctx.handle, ctx.KeyIndex, &input, &output, &auth) {
		ret = C.GoBytes(unsafe.Pointer(output.buffer), C.int(output.size))
	}

	C.free(unsafe.Pointer(input.buffer))
	C.free(unsafe.Pointer(output.buffer))
	C.free(unsafe.Pointer(auth.buffer))

	return ret
}
