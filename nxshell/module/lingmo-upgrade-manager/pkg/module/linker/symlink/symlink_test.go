// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package symlink

import (
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"
)

type fileInfo struct {
	name   string
	origin string
	ty     int
}

var originList = []fileInfo{
	{
		name: "usr",
		ty:   1,
	},
	{
		name: "usr/boot",
		ty:   1,
	},
	{
		name: "usr/boot/config",
		ty:   2,
	},
	{
		name:   "boot",
		origin: "usr/boot",
		ty:     3,
	},
	{
		name: "usr/etc",
		ty:   1,
	},
	{
		name: "usr/etc/fstab",
		ty:   2,
	},
	{
		name: "etc",
		ty:   1,
	},
	{
		name:   "etc/fstab",
		origin: "../usr/etc/fstab",
		ty:     3,
	},
	{
		name: "usr/etc/apt",
		ty:   1,
	},
	{
		name: "usr/etc/apt/sources.list.d",
		ty:   1,
	},
	{
		name: "usr/etc/apt/sources.list.d/backport.list",
		ty:   2,
	},
	{
		name: "etc/apt",
		ty:   1,
	},
	{
		name: "etc/apt/sources.list",
		ty:   2,
	},
	{
		name:   "etc/apt/sources.list.d",
		origin: "../../usr/etc/apt/sources.list.d",
		ty:     3,
	},
	{
		name:   "etc/apt/sources.list",
		origin: "../../usr/etc/apt/sources.list",
		ty:     3,
	},
}

var targetList = []fileInfo{
	{
		name: "usr",
		ty:   1,
	},
	{
		name: "usr/etc",
		ty:   1,
	},
}

var originDir = "root-origin"
var targetDir = "root-target"

func TestSymlinkDir(t *testing.T) {
	_ = os.RemoveAll(originDir)
	_ = os.RemoveAll(targetDir)

	makeDirMock(originDir, originList)
	makeDirMock(targetDir, targetList)

	var hasError = false
	err := SymlinkDir(originDir, targetDir)
	if err != nil {
		hasError = true
		t.Fatal("Except nil, but got error:", err)
	}

	for _, fi := range originList {
		_, err = os.Stat(filepath.Join(targetDir, fi.name))
		if err != nil {
			hasError = true
			t.Error("Except nil, but got error:", err)
		}
	}

	if !hasError {
		_ = os.RemoveAll(originDir)
		_ = os.RemoveAll(targetDir)
	}
}

func makeDirMock(baseDir string, fiList []fileInfo) {
	_ = os.Mkdir(baseDir, 0755)
	for _, fi := range fiList {
		switch fi.ty {
		case 1:
			_ = os.Mkdir(filepath.Join(baseDir, fi.name), 0755)
		case 2:
			_ = ioutil.WriteFile(filepath.Join(baseDir, fi.name), nil, 0644)
		case 3:
			_ = os.Symlink(fi.origin, filepath.Join(baseDir, fi.name))
		}
	}
}
