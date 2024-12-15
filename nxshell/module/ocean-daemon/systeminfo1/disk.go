// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"fmt"

	"github.com/godbus/dbus/v5"
)

// nolint
type diskInfo struct {
	Drive       dbus.ObjectPath // org.freedesktop.UDisks2.Block Drive
	MountPoints []string        // org.freedesktop.UDisks2.Filesystem MountPoints
	Size        uint64          // org.freedesktop.UDisks2.Partition Size
	Table       dbus.ObjectPath // org.freedesktop.UDisks2.Partition Table
}

type diskInfoMap map[dbus.ObjectPath]diskInfo //nolint

func (set diskInfoMap) GetRootDrive() dbus.ObjectPath {
	for _, v := range set {
		if v.Drive == "" {
			continue
		}
		for _, mp := range v.MountPoints {
			if mp == "/" {
				return v.Drive
			}
		}
	}
	return ""
}

func (set diskInfoMap) GetRootTable() dbus.ObjectPath {
	for _, v := range set {
		if v.Table == "" {
			continue
		}
		for _, mp := range v.MountPoints {
			if mp == "/" {
				return v.Table
			}
		}
	}
	return ""
}

func (set diskInfoMap) GetRootSize() uint64 {
	for _, v := range set {
		if len(v.MountPoints) == 0 {
			continue
		}
		for _, mp := range v.MountPoints {
			if mp == "/" {
				return v.Size
			}
		}
	}
	return 0
}

func (set diskInfoMap) Get(key dbus.ObjectPath) *diskInfo {
	if key == "" {
		return nil
	}
	v, ok := set[key]
	if !ok {
		return nil
	}
	return &v
}

func getDiskCap() (uint64, error) {
	dlist, err := GetDiskList()
	if err != nil {
		fmt.Println("Failed to get disk list:", err)
		return 0, err
	}

	rdisk := dlist.GetRoot()
	if rdisk == nil {
		fmt.Println("Failed to get root disk")
		return 0, err
	}
	return uint64(rdisk.Size), err
}
