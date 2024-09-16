// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package airplane_mode

import (
	"bytes"
	"encoding/binary"
	"io/ioutil"
	"unsafe"
)

func readFile(filename string) (string, error) {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		return "", err
	}
	return string(bytes.TrimSpace(content)), nil
}

// get byte order
func getByteOrder() binary.ByteOrder {
	var order binary.ByteOrder
	if isLittleEndian() {
		order = binary.LittleEndian
	} else {
		order = binary.BigEndian
	}
	return order
}

func isLittleEndian() bool {
	n := 0x1234
	f := *((*byte)(unsafe.Pointer(&n)))
	return (f ^ 0x34) == 0
}
