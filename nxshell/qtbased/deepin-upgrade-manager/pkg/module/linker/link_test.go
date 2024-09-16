// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package linker

import (
	"deepin-upgrade-manager/pkg/module/util"
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

var hardlinkList = []fileInfo{
	{
		name: "usr",
		ty:   1,
	},
	{
		name: "usr/boot",
		ty:   1,
	},
	{
		name: "usr/boot/config-5.10.60-amd64-desktop",
		ty:   2,
	},
	{
		name:   "boot",
		origin: "usr/boot",
		ty:     3,
	},
	{
		name:   "config-5.10.60-amd64-desktop",
		origin: "boot/config-5.10.60-amd64-desktop",
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
		name: "usr/etc/apt/sources.list",
		ty:   2,
	},
}

var symlinkList = []fileInfo{
	{
		name: "usr",
		ty:   1,
	},
	{
		name: "usr/etc",
		ty:   1,
	},
	{
		name: "usr/etc/apt",
		ty:   1,
	},
	{
		name: "usr/etc/apt/sources.list.bak",
		ty:   2,
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
		name: "etc",
		ty:   1,
	},
	{
		name: "etc/apt",
		ty:   1,
	},
	{
		name:   "etc/apt/sources.list.d",
		origin: "../../usr/etc/apt/sources.list.d",
		ty:     3,
	},
	{
		name:   "etc/apt/sources.list.bak",
		origin: "../../usr/etc/apt/sources.list.bak",
		ty:     3,
	},
}

var mergeList = []fileInfo{
	{
		name: "etc",
		ty:   1,
	},
	{
		name: "etc/fstab",
		ty:   2,
	},
	{
		name: "etc/login.defs",
		ty:   2,
	},
	{
		name: "usr",
		ty:   1,
	},
	{
		name: "usr/boot",
		ty:   1,
	},
	{
		name: "usr/boot/config-5.10.60-amd64-desktop",
		ty:   2,
	},
	{
		name: "usr/grub",
		ty:   1,
	},
	{
		name: "usr/grub/grub.cfg",
		ty:   2,
	},
}

var (
	hardlinkDir = "root-hardlink"
	symlinkDir  = "root-symlink"
	targetDir   = "root-target"
	mergeDir    = "root-merge"
)

func TestCombineDir(t *testing.T) {
	_ = os.RemoveAll(hardlinkDir)
	_ = os.RemoveAll(symlinkDir)
	_ = os.RemoveAll(targetDir)

	makeDirMock(hardlinkDir, hardlinkList)
	makeDirMock(symlinkDir, symlinkList)

	var hasError = false
	err := CombineDir(hardlinkDir, symlinkDir, targetDir)
	if err != nil {
		hasError = true
		t.Fatal("Except nil, but got error:", err)
	}

	checkFile(t, hardlinkList, &hasError)
	checkFile(t, symlinkList, &hasError)

	if !hasError {
		_ = os.RemoveAll(hardlinkDir)
		_ = os.RemoveAll(symlinkDir)
		_ = os.RemoveAll(targetDir)
	}
}

func TestMergeDir(t *testing.T) {
	_ = os.RemoveAll(hardlinkDir)
	_ = os.RemoveAll(mergeDir)

	makeDirMock(hardlinkDir, hardlinkList)
	makeDirMock(mergeDir, mergeList)

	var hasError = false
	err := MergeDir(mergeDir, hardlinkDir, []string{
		filepath.Join(hardlinkDir, "etc/fstab"),
	}, true)
	if err != nil {
		hasError = true
		t.Fatal("Except nil, but got error:", err)
	}

	fileList := []struct {
		filename string
		equal    bool
	}{
		{filename: "etc/fstab", equal: true},
		{filename: "etc/login.defs", equal: true},
		{filename: "usr/boot/config-5.10.60-amd64-desktop", equal: false},
	}
	for _, f := range fileList {
		data1, _ := ioutil.ReadFile(filepath.Join(hardlinkDir, f.filename))
		data2, _ := ioutil.ReadFile(filepath.Join(mergeDir, f.filename))
		if f.equal && string(data1) != string(data2) {
			t.Errorf("Except equal `%s`, but got `%s`, for %s", string(data2),
				string(data1), f.filename)
		} else if !f.equal && string(data1) == string(data2) {
			t.Errorf("Except not equal `%s`, but got `%s`, for %s", string(data1),
				string(data2), f.filename)
		}
	}

	if !hasError {
		_ = os.RemoveAll(hardlinkDir)
		_ = os.RemoveAll(mergeDir)
	}
}

func checkFile(t *testing.T, fiList []fileInfo, error *bool) {
	for _, fi := range fiList {
		_, err := os.Stat(filepath.Join(targetDir, fi.name))
		if err != nil {
			*error = true
			t.Error("Except nil, but got error:", err)
		}
	}
}

func makeDirMock(baseDir string, fiList []fileInfo) {
	_ = os.Mkdir(baseDir, 0755)
	for _, fi := range fiList {
		switch fi.ty {
		case 1:
			_ = os.Mkdir(filepath.Join(baseDir, fi.name), 0755)
		case 2:
			_ = ioutil.WriteFile(filepath.Join(baseDir, fi.name),
				[]byte(util.MakeRandomString(10)), 0644)
		case 3:
			_ = os.Symlink(fi.origin, filepath.Join(baseDir, fi.name))
		}
	}
}
