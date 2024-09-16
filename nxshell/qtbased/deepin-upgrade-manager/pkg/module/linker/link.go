// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package linker

import (
	"deepin-upgrade-manager/pkg/module/linker/hardlink"
	"deepin-upgrade-manager/pkg/module/linker/symlink"
	"deepin-upgrade-manager/pkg/module/util"
	"fmt"
	"io/ioutil"
	"path/filepath"
	"syscall"
)

// CombineDir regenerate 'dstDir' base on 'hardlinkDir' and 'symlinkDir'
func CombineDir(hardlinkDir, symlinkDir, dstDir string) error {
	err := hardlink.HardlinkDir(hardlinkDir, dstDir)
	if err != nil {
		return err
	}

	if len(symlinkDir) == 0 {
		return nil
	}
	if !util.IsExists(symlinkDir) {
		return nil
	}
	return symlink.SymlinkDir(symlinkDir, dstDir)
}

// MergeDir merge files in 'dir' to 'dstDir'
func MergeDir(srcDir, dstDir string, replaceList []string, force bool) error {
	fiList, err := ioutil.ReadDir(srcDir)
	if err != nil {
		return err
	}

	for _, fi := range fiList {
		srcSub := filepath.Join(srcDir, fi.Name())
		dstSub := filepath.Join(dstDir, fi.Name())

		fiStat, ok := fi.Sys().(*syscall.Stat_t)
		if !ok {
			return fmt.Errorf("not get raw stat for: %s", srcSub)
		}

		isSymlink := false
		skip := false
		switch {
		case fiStat.Mode&syscall.S_IFLNK == syscall.S_IFLNK:
			isSymlink = true
		case fiStat.Mode&syscall.S_IFDIR == syscall.S_IFDIR:
			err = MergeDir(srcSub, dstSub, replaceList, force)
			if err != nil {
				return err
			}
			skip = true
		case fiStat.Mode&syscall.S_IFREG == syscall.S_IFREG:
		default:
			skip = true
		}

		if skip {
			continue
		}

		if util.IsExists(dstSub) {
			if !util.IsItemInList(dstSub, replaceList) && !force {
				continue
			}
		}
		if isSymlink {
			err = util.Symlink(srcSub, dstSub)
		} else {
			err = util.CopyFile2(srcSub, dstSub, fi, true)
		}
		if err != nil {
			return err
		}
	}
	return nil
}
