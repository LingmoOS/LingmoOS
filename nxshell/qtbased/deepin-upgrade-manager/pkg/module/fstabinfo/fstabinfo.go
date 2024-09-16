// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package fstabinfo

import (
	"bufio"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/dirinfo"
	"deepin-upgrade-manager/pkg/module/diskinfo"
	"deepin-upgrade-manager/pkg/module/util"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type FsInfo struct {
	SrcPoint  string
	DiskUUID  string
	DestPoint string
	FSType    string
	Options   string
	Bind      bool
	Remote    bool
}

type FsInfoList []*FsInfo

func (fs FsInfoList) MaxFreePartitionPoint() (string, string) {
	var maxFree uint64
	var point, uuid string
	for _, v := range fs {
		free, err := dirinfo.GetPartitionFreeSize(v.DestPoint)
		if err != nil {
			logger.Warningf("failed get par:%s size, err: %v", v.DestPoint, err)
			continue
		}
		if maxFree < free && !v.Remote {
			maxFree = free
			point = v.DestPoint
			uuid = v.DiskUUID
		}
	}
	return point, uuid
}

func (fs FsInfoList) MatchDestPoint(point string) FsInfo {
	var info FsInfo
	for _, v := range fs {
		if v.DestPoint == point {
			info = *v
		}
	}
	return info
}

func (fs FsInfoList) IsInFstabPoint(rootdir, point string) bool {
	for _, v := range fs {
		src := util.TrimRootdir(rootdir, v.SrcPoint)
		dst := util.TrimRootdir(rootdir, v.DestPoint)
		if dst == point || src == point {
			return true
		}
	}
	return false
}

func getPartiton(spec string, dsInfos diskinfo.DiskIDList) (string, string, error) {
	bits := strings.Split(spec, "=")
	var specvalue string
	var dsInfo *diskinfo.DiskID
	if len(bits) == 1 {
		specvalue = spec
	} else {
		specvalue = bits[1]
	}

	switch strings.ToUpper(bits[0]) {
	case "UUID":
		dsInfo = dsInfos.MatchUUID(specvalue)
	case "LABEL":
		dsInfo = dsInfos.MatchLabel(specvalue)
	case "PARTUUID":
		dsInfo = dsInfos.MatchPartUUID(specvalue)
	case "PARTLABEL":
		dsInfo = dsInfos.MatchPartLabel(specvalue)
	default:
		dsInfo = dsInfos.MatchPartition(specvalue)
	}
	if nil == dsInfo {
		return "", "", fmt.Errorf("cannot match the corresponding partition of the disk,specvalue:%s", specvalue)
	}
	return dsInfo.Partition, dsInfo.UUID, nil
}

func parseOptions(optionsString string) (options map[string]string) {
	options = make(map[string]string)
	var key, value string
	for i, option := range strings.Split(optionsString, ",") {
		if i == 0 {
			key = option
		} else {
			value += option
		}
	}
	options[key] = value
	return
}

// /etc/fstab
func Load(filename, rootDir string) (FsInfoList, error) {
	fr, err := os.Open(filepath.Clean(filename))
	if err != nil {
		return nil, err
	}
	defer func() {
		if err := fr.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}()
	var infos FsInfoList
	dsInfos, err := diskinfo.Load("/dev/disk")
	if err != nil {
		return infos, err
	}
	scanner := bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		line = strings.TrimSpace(line)
		if (line == "") || (line[0] == '#') {
			continue
		}
		items := strings.Fields(line)
		if len(items) < 4 {
			logger.Warningf("too few fields (%d), at least 4 are expected", len(items))
			continue
		} else {
			var srcMountPoint, uuid string
			var isBind, isRemote bool
			if items[1] == "none" {
				continue
			}
			optionsMap := parseOptions(items[3])
			if items[3] == "bind" || optionsMap["defaults"] == "bind" {
				srcMountPoint = filepath.Join(rootDir, items[0])
				isBind = true
			} else if items[2] == "cifs" || items[2] == "nfs" || strings.Contains(items[2], "fuse") {
				srcMountPoint = filepath.Join(rootDir, items[0])
				isRemote = true
			} else {
				srcMountPoint, uuid, err = getPartiton(items[0], dsInfos)
				if err != nil {
					logger.Warning("failed get mount point, err:", err)
					return infos, err
				}
				isBind = false
			}

			infos = append(infos, &FsInfo{
				SrcPoint:  srcMountPoint,
				DiskUUID:  uuid,
				DestPoint: items[1],
				FSType:    items[2],
				Options:   items[3],
				Bind:      isBind,
				Remote:    isRemote,
			})
		}
	}
	return infos, nil
}
