// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package mountinfo

import (
	"bufio"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/dirinfo"
	"os"
	"path/filepath"
	"strings"
)

const (
	_MOUNTS_DELIM = " "

	_MOUNTS_ITEM_NUM = 6
)

type MountInfo struct {
	Partition  string
	MountPoint string
	FSType     string
	Options    string
}

type MountInfoList []*MountInfo

func (infos MountInfoList) Query(dir string) MountInfoList {
	var list MountInfoList
	for _, info := range infos {
		if !strings.HasPrefix(info.MountPoint, dir) {
			continue
		}
		list = append(list, info)
	}
	return list
}

func (infos MountInfoList) Match(dir string) *MountInfo {
	for _, info := range infos {
		if info.MountPoint == dir {
			return info
		}
	}
	return nil
}

func (infos MountInfoList) MatchPartition(partiton string) *MountInfo {
	for _, info := range infos {
		if info.Partition == partiton {
			return info
		}
	}
	return nil
}

func (infos MountInfoList) MaxPartition(dirs []string) string {
	var max uint64
	var dirPath string
	for _, info := range infos {
		for _, dir := range dirs {
			if info.MountPoint == dir {
				part, err := dirinfo.GetPartitionFreeSize(info.MountPoint)
				if err != nil {
					logger.Warningf("failed get par:%s size, err: %v", dir, err)
					continue
				}
				if max < part {
					max = part
					dirPath = info.MountPoint
				}
			}
		}
	}
	return infos.Match(dirPath).MountPoint
}

func isExist(info MountInfo, list MountInfoList) bool {
	for _, v := range list {
		if info.FSType == v.FSType && info.MountPoint == v.MountPoint &&
			info.Options == v.Options && info.Partition == v.Partition {
			return true
		}
	}
	return false
}

func Load(filename string) (MountInfoList, error) {
	fr, err := os.Open(filepath.Clean(filename))
	if err != nil {
		return nil, err
	}
	defer func() {
		if err := fr.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}()
	var infos MountInfoList
	scanner := bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) == 0 {
			continue
		}
		items := strings.Split(line, _MOUNTS_DELIM)
		if len(items) != _MOUNTS_ITEM_NUM {
			logger.Debug("invalid mounts line:", line)
			continue
		}
		if isFilterFSType(items[2]) {
			continue
		}
		mountInfo := &MountInfo{
			Partition:  items[0],
			MountPoint: items[1],
			FSType:     items[2],
			Options:    items[3],
		}
		if isExist(*mountInfo, infos) {
			continue
		}
		infos = append(infos, mountInfo)
	}
	return infos, nil
}

func GetFilterInfo(filename string) (MountInfoList, error) {
	fr, err := os.Open(filepath.Clean(filename))
	if err != nil {
		return nil, err
	}
	defer func() {
		if err := fr.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}()
	var infos MountInfoList
	scanner := bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) == 0 {
			continue
		}
		items := strings.Split(line, _MOUNTS_DELIM)
		if len(items) != _MOUNTS_ITEM_NUM {
			logger.Debug("invalid mounts line:", line)
			continue
		}
		if isFilterFSType(items[2]) || isFilterFSType(items[0]) {
			mountInfo := &MountInfo{
				Partition:  items[0],
				MountPoint: items[1],
				FSType:     items[2],
				Options:    items[3],
			}
			if isExist(*mountInfo, infos) {
				continue
			}
			infos = append(infos, mountInfo)
		}
	}
	return infos, nil
}

func isFilterFSType(ty string) bool {
	list := []string{
		"proc",
		"sysfs",
		"devpts",
		"devtmpfs",
		"tmpfs",
		"efivarfs",
		"securityfs",
		"cgroup",
		"cgroup2",
		"pstore",
		"bpf",
		"autofs",
		"hugetlbfs",
		"mqueue",
		"debugfs",
		"tracefs",
		"configfs",
		"ramfs",
		"fusectl",
		"fuse.gvfsd-fuse",
		"fuse.portal",
		"binfmt_misc",
		"lxcfs",
		"fuse.lxcfs",
	}
	for _, v := range list {
		if v == ty {
			return true
		}
	}
	return false
}
