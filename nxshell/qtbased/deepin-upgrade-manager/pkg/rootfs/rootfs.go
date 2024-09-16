// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package rootfs

import (
	config "deepin-upgrade-manager/pkg/config/upgrader"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/mountpoint"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

type Rootfs struct {
	conf *config.Config

	preBindList  mountpoint.MountPointList
	repoBindList mountpoint.MountPointList

	rootMP  string
	baseDir string

	RootDir string
}

func NewRootfs(conf *config.Config, rootMP, baseDir string) (*Rootfs, error) {
	info := Rootfs{
		conf:    conf,
		rootMP:  rootMP,
		baseDir: baseDir,
		RootDir: filepath.Join(rootMP, conf.CacheDir, baseDir),
	}

	info.preBindList = info.getPredefinedBindList()
	info.repoBindList = info.getRepoBindList()

	err := info.symlinkDirList()
	if err != nil {
		return nil, err
	}
	return &info, nil
}

func (fs *Rootfs) Make() error {
	return fs.mountBindList()
}

func (fs *Rootfs) Clean() error {
	err := fs.umountBindList()
	if err != nil {
		return err
	}
	return os.RemoveAll(fs.RootDir)
}

func (fs *Rootfs) mountBindList() error {
	preList, err := fs.preBindList.Mount()
	if err != nil {
		return preList.Umount()
	}
	repoList, err := fs.repoBindList.Mount()
	if err != nil {
		if e := preList.Umount(); e != nil {
			logger.Warning("umount predefined list because repo list:", e)
		}
		return repoList.Umount()
	}
	return nil
}

func (fs *Rootfs) umountBindList() error {
	err := fs.repoBindList.Umount()
	if err != nil {
		return err
	}
	return fs.preBindList.Umount()
}

func (fs *Rootfs) symlinkDirList() error {
	usrName := "usr"
	list := getSymlinkDirList(fs.rootMP)
	for _, v := range list {
		dir := filepath.Join(fs.RootDir, usrName, v)
		err := os.MkdirAll(dir, 0750)
		if err != nil {
			return err
		}
		err = os.Symlink(filepath.Join(usrName, v), filepath.Join(fs.RootDir, v))
		if err != nil {
			return err
		}
	}
	return nil
}

func (fs *Rootfs) getPredefinedBindList() mountpoint.MountPointList {
	mpList := mountpoint.MountPointList{
		&mountpoint.MountPoint{
			Src:  "/usr",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:  "/var",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:  "/opt",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:  "/root",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:  "/home",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:  "/tmp",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:  "/run",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:  "/dev",
			Bind: true,
		},
		&mountpoint.MountPoint{
			Src:    "/dev/pts",
			FSType: "devpts",
		},
		&mountpoint.MountPoint{
			Src:    "/proc",
			FSType: "proc",
		},
		&mountpoint.MountPoint{
			Src:    "/sys",
			FSType: "sysfs",
		},
	}
	for _, mp := range mpList {
		mp.Src = filepath.Join(fs.rootMP, mp.Src)
		mp.Dest = filepath.Join(fs.RootDir, mp.Src)
	}

	return mpList
}

func (fs *Rootfs) getRepoBindList() mountpoint.MountPointList {
	var mpList mountpoint.MountPointList
	for _, repoConfig := range fs.conf.RepoList {
		for _, v := range repoConfig.SubscribeList {
			mpList = append(mpList, &mountpoint.MountPoint{
				Src:  filepath.Join(fs.rootMP, repoConfig.SnapshotDir, fs.baseDir, v),
				Dest: filepath.Join(fs.RootDir, v),
				Bind: true,
			})
		}
	}
	return mpList
}

func getSymlinkDirList(dir string) []string {
	fiList, err := ioutil.ReadDir(dir)
	if err != nil {
		return nil
	}

	list := []string{
		"boot",
		"bin",
		"sbin",
	}
	for _, fi := range fiList {
		if fi.IsDir() || fi.Mode().IsRegular() {
			continue
		}
		if strings.HasPrefix(fi.Name(), "lib") {
			list = append(list, fi.Name())
		}
	}
	return list
}
