// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package mountpoint

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"strings"
)

const (
	_CMD_MOUNT  = "mount"
	_CMD_UMOUNT = "umount"
)

type MountPoint struct {
	Src     string
	Dest    string
	FSType  string
	Options string

	Bind bool
}
type MountPointList []*MountPoint

func (list MountPointList) Mount() (MountPointList, error) {
	var mounted MountPointList
	var err error
	for _, mp := range list {
		err = mp.Mount()
		if err != nil {
			break
		}
		mounted = append(mounted, mp)
	}

	return mounted, err
}

func (list MountPointList) Umount() error {
	var items []string
	for _, mp := range list {
		if isInListByPrefix(mp.Dest, items) {
			continue
		}
		items = append(items, mp.Dest)
	}
	var args []string
	args = append(args, items...)
	logger.Info("Will umount:", args)
	return util.ExecCommand(_CMD_UMOUNT, args)
}

func (mp *MountPoint) Mount() error {
	err := util.Mkdir(mp.Src, mp.Dest)
	if err != nil {
		return err
	}

	var args []string
	if mp.Bind {
		args = append(args, []string{"-o", "bind"}...)
	} else {
		args = append(args, []string{"-t", mp.FSType}...)
		args = append(args, []string{"-o", mp.Options}...)
	}
	args = append(args, mp.Src)
	args = append(args, mp.Dest)
	logger.Info("Will mount:", args)
	return util.ExecCommand(_CMD_MOUNT, args)
}

func (mp *MountPoint) Umount() error {
	return util.ExecCommand(_CMD_UMOUNT, []string{mp.Dest})
}

func isInListByPrefix(item string, list []string) bool {
	for _, v := range list {
		if strings.HasPrefix(item, v) {
			return true
		}
	}
	return false
}
