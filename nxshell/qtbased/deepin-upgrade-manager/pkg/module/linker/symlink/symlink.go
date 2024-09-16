// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package symlink

import (
	"deepin-upgrade-manager/pkg/module/util"
	"io/ioutil"
	"os"
	"path/filepath"
)

func SymlinkDir(srcDir, dstDir string) error {
	err := util.Mkdir(srcDir, dstDir)
	if err != nil {
		return err
	}
	return doSymlinkDir(srcDir, srcDir, dstDir)
}

func doSymlinkDir(baseDir, srcDir, dstDir string) error {
	fiList, err := ioutil.ReadDir(srcDir)
	if err != nil {
		return err
	}

	var symlinkFiles []string
	for _, fi := range fiList {
		sname := filepath.Join(srcDir, fi.Name())
		dname := filepath.Join(dstDir, fi.Name())
		switch {
		case fi.IsDir():
			if util.IsExists(dname) {
				err = doSymlinkDir(baseDir, sname, dname)
				if err != nil {
					return err
				}
				continue
			}
			fallthrough
		case fi.Mode().IsRegular():
			dstRel, err := filepath.Rel(dstDir, baseDir)
			if err != nil {
				return err
			}
			srcRel, err := filepath.Rel(baseDir, sname)
			if err != nil {
				return err
			}
			err = os.Symlink(filepath.Join(dstRel, srcRel), dname)
			if err != nil {
				return err
			}
		default:
			symlinkFiles = append(symlinkFiles, sname)
		}
	}

	err = doSymlinkFiles(symlinkFiles, baseDir, dstDir)
	if err != nil {
		return err
	}

	return nil
}

func doSymlinkFiles(files []string, baseDir, dstDir string) error {
	for _, file := range files {
		srcRel, err := filepath.Rel(baseDir, file)
		if err != nil {
			return err
		}
		srcReal, err := os.Readlink(file)
		if err != nil {
			return err
		}
		err = os.Symlink(srcReal, filepath.Join(dstDir, srcRel))
		if err != nil {
			return err
		}
	}
	return nil
}
