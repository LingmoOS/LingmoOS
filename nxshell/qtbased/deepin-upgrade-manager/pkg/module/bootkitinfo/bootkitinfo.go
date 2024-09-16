// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bootkitinfo

import (
	"deepin-upgrade-manager/pkg/module/dirinfo"
	"deepin-upgrade-manager/pkg/module/generator"
	"deepin-upgrade-manager/pkg/module/repo/branch"
	"deepin-upgrade-manager/pkg/module/util"
	"encoding/json"
	"errors"
	"io/ioutil"
	"path/filepath"
	"sort"
	"strings"
)

type BootInfo struct {
	Version     string `json:"version"`
	Kernel      string `json:"kernel"`
	Initrd      string `json:"initrd"`
	Scheme      string `json:"scheme"`
	DisplayInfo string `json:"display"`
	UUID        string `json:"uuid"`
}

type BootInfos []*BootInfo

type BootInfoList struct {
	VersionList BootInfos `json:"version_list"`
}

const (
	BOOT_SNAPSHOT_DIR = "/boot/snapshot"
	SCHEME            = "atomic"
	DEEPIN_BOOT_KIT   = "/usr/sbin/deepin-boot-kit"
)

func (infolist BootInfoList) ToJson() string {
	b, _ := json.Marshal(&infolist)
	return string(b)
}

func (list BootInfos) Less(i, j int) bool {
	return generator.Less(list[i].Version, list[j].Version)
}

func (infolist BootInfoList) Sort() BootInfoList {
	list := infolist
	sort.SliceStable(list.VersionList, func(i, j int) bool {
		return list.VersionList.Less(i, j)
	})
	return list
}

func NewVersion() (string, error) {
	action := string("--action=") + "version"
	out, _ := util.ExecCommandWithOut(DEEPIN_BOOT_KIT, []string{action})
	version := strings.TrimSpace(string(out))
	if !branch.IsValid(version) {
		return "", errors.New("failed from boot kit get version")
	}
	return version, nil
}

func Update() error {
	action := string("--action=") + "update"
	err := util.ExecCommand(DEEPIN_BOOT_KIT, []string{action})
	if err != nil {
		return err
	}
	return nil
}

func (infolist *BootInfoList) SetVersionName(version, display string) {
	for _, v := range infolist.VersionList {
		if v.Version == version {
			v.DisplayInfo = display
		}
	}
}

func (infolist *BootInfoList) VmlinuxName(vmlinuxs []string) string {
	var vmlinux, maxVersion string
	for _, v := range vmlinuxs {
		columns := strings.Split(v, "-")
		if len(columns) < 2 {
			continue
		}
		if util.VersionOrdinal(columns[1]) > util.VersionOrdinal(maxVersion) {
			vmlinux = v
			maxVersion = columns[1]
		}
	}
	if len(vmlinux) == 0 {
		vmlinux = vmlinuxs[0]
	}
	return vmlinux
}

func (infolist *BootInfoList) InitrdName(initrdPaths []string, vmlinux string) string {
	index := strings.IndexRune(vmlinux, '-')
	for _, v := range initrdPaths {
		if strings.HasSuffix(v, vmlinux[index:]) {
			return v
		}
	}
	return initrdPaths[0]
}

func Load(versionList []string, RepoUUID string) BootInfoList {
	var infolist BootInfoList
	var isAcrossPart bool

	rootPartition, _ := dirinfo.GetDirPartition("/")
	bootPartition, _ := dirinfo.GetDirPartition("/boot")
	if bootPartition == rootPartition {
		isAcrossPart = false
	} else {
		isAcrossPart = true
	}

	for _, v := range versionList {
		var info BootInfo
		bootDir := filepath.Join(BOOT_SNAPSHOT_DIR, v)
		fiList, err := ioutil.ReadDir(bootDir)
		if err != nil {
			continue
		}
		var initrds, vmlinuxs []string
		for _, fi := range fiList {
			if fi.IsDir() {
				continue
			}
			if strings.HasPrefix(fi.Name(), "vmlinuz-") ||
				strings.HasPrefix(fi.Name(), "kernel-") ||
				strings.HasPrefix(fi.Name(), "vmlinux-") {
				vmlinuxs = append(vmlinuxs, fi.Name())
			}
			if strings.HasPrefix(fi.Name(), "initrd.img-") {
				initrds = append(initrds, fi.Name())
			}
		}
		if len(initrds) != 0 && len(vmlinuxs) != 0 {
			vmlinux := filepath.Join(bootDir, infolist.VmlinuxName(vmlinuxs))
			initrd := filepath.Join(bootDir, infolist.InitrdName(initrds, vmlinux))
			if isAcrossPart {
				info.Initrd = strings.TrimPrefix(initrd, "/boot")
				info.Kernel = strings.TrimPrefix(vmlinux, "/boot")
			} else {
				info.Initrd = initrd
				info.Kernel = vmlinux
			}
			info.Version = v
			info.Scheme = SCHEME
			info.UUID = RepoUUID
		} else {
			continue
		}
		infolist.VersionList = append(infolist.VersionList, &info)
	}
	return infolist
}
