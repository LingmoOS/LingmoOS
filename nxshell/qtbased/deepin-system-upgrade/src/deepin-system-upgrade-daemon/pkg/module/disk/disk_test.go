// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package disk

import (
	"testing"
)

func TestGetDirSize(t *testing.T) {
	size := GetDirSize("/usr")
	t.Error(size)
}

func TestComputeFileSize(t *testing.T) {
	size, err := computeFileSize("/usr")
	if err != nil {
		t.Error(err)
	}
	t.Error(size)
}

// func TestCheckDiskSpace(t *testing.T) {
// 	var result = make(map[string]string)
// 	r, err := CheckDiskSpace(result)
// 	if err != nil {
// 		t.Error(err)
// 	}
// 	t.Error(r)
// }

// func TestComputeOstreeRepoSize(t *testing.T) {
// 	size, err := computeOstreeRepoSize()
// 	if err != nil {
// 		t.Error(err)
// 	}
// 	t.Error(size)
// }
