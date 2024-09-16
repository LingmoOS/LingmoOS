// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package disk

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/linuxdeepin/go-lib/log"

	"deepin-system-upgrade-daemon/pkg/module/atomic"
	"deepin-system-upgrade-daemon/pkg/module/config"
)

var logger = log.NewLogger("deepin-system-upgrade/module/diskinfo")

var (
	RootPartitionRequire = 3.0
	NewSystemRootRequire = 17.0
)

var (
	_DirInfoList  = []string{"/", "/home"}
	_PartitionMap = make(map[string]float64)
)

func GetDiskFromPartition(path string) (string, error) {
	out, err := exec.Command("/usr/bin/lsblk", "-ndo", "pkname", path).CombinedOutput()
	if err != nil {
		return "", err
	}
	return filepath.Join("/dev", strings.Trim(string(out), "\n")), nil
}

func GetPartitionByPath(path string) (string, error) {
	out, err := exec.Command("/usr/bin/df", path).CombinedOutput()
	if err != nil {
		return "", err
	}
	lines := strings.Split(string(out), "\n")
	if len(lines) < 2 {
		return "", errors.New("df output not avaliable")
	}
	items := strings.Split(lines[1], " ")
	return strings.TrimSpace(items[0]), nil
}

func getDirSize(dirpath string) (float64, error) {
	var size int64
	files, err := ioutil.ReadDir(dirpath)
	if err != nil {
		logger.Warning("failed to read dir:", err)
		return -1, err
	}

	for _, file := range files {
		if file.Mode().IsRegular() {
			size += file.Size()
		}
	}
	return float64(size) / 1024.0 / 1024.0 / 1024.0, nil
}

func computeDirFreeSpace(path string) error {
	out, err := exec.Command("/usr/bin/df", path).CombinedOutput()
	if err != nil {
		return err
	}
	outLines := strings.Split(string(out), "\n")
	if len(outLines) < 2 {
		return errors.New("failed to get dir partition")
	}

	line := strings.Split(outLines[1], " ")
	if err != nil {
		return err
	}
	p := strings.TrimSpace(line[0])
	if _, ok := _PartitionMap[p]; !ok {
		line = strings.Fields(outLines[1])
		if err != nil {
			return err
		}
		freeSize := strings.TrimSpace(line[3])
		size, err := strconv.Atoi(freeSize)
		if err != nil {
			return err
		}

		f, _ := strconv.ParseFloat(fmt.Sprintf("%.1f", (float64(size)/(float64(1024)*float64(1024)))), 64)
		_PartitionMap[p] = f
	}
	return nil
}

// func computeFileSize(path string) (float64, error) {
// 	out, err := exec.Command("/usr/bin/du", "-s", path).CombinedOutput()
// 	if err != nil {
// 		logger.Warning("failed to get dir size:", path)
// 		return -1, err
// 	}
// 	size, err := strconv.Atoi(strings.Trim(strings.Split(strings.TrimSpace(string(out)), "/")[0], "\t"))
// 	if err == nil {
// 		return float64(size) / 1000.0 / 1000.0, nil
// 	}
// 	return -1, err
// }

func findMountForDir(path string) string {
	out, err := exec.Command("/usr/bin/findmnt", "-n", "-o", "TARGET", "--target", path).CombinedOutput()
	if err != nil {
		return ""
	}
	logger.Warning(string(out))
	return strings.Trim(string(out), "\n")
}

func computeRequireSizeForRoot() (float64, error) {
	target, err := config.GetBackupTarget()
	if err != nil {
		return -1, err
	}
	var needSize float64
	for _, dir := range target.BackupList {
		if findMountForDir(dir) != "/" {
			size, err := getDirSize(dir)
			if err != nil {
				return -1, err
			}
			needSize += size
		}
	}
	for _, dir := range target.HoldList {
		if findMountForDir(dir) != "/" {
			size, err := getDirSize(dir)
			if err != nil {
				return -1, err
			}
			needSize -= size
		}
	}
	return needSize, nil
}

func computeOstreeRepoSize() (float64, error) {
	target, err := config.GetBackupTarget()
	if err != nil {
		return -1, err
	}
	var repoSize float64
	for _, dir := range target.BackupList {
		size, err := getDirSize(dir)
		if err != nil {
			return -1, err
		}
		logger.Debug("print dir path:", dir)
		logger.Debug("print dir size:", size)

		repoSize += size
	}
	for _, dir := range target.HoldList {
		size, err := getDirSize(dir)
		if err != nil {
			return -1, err
		}

		repoSize -= size
	}
	return repoSize, nil
}

func CheckDiskSpace(checkResult map[string]string) (map[string]string, error) {
	// Get disk partition free space
	for _, path := range _DirInfoList {
		err := computeDirFreeSpace(path)
		if err != nil {
			continue
		}
	}
	repoLocation, err := atomic.GetOStreeRepoInfo("SOURCE")
	if err != nil {
		return nil, err
	}
	var rootPartitionRequire float64
	needSize, err := computeRequireSizeForRoot()
	logger.Warning("needsize:", needSize)
	if err != nil {
		logger.Warning("failed to compute root require size:", err)
		rootPartitionRequire = RootPartitionRequire
	} else {
		if needSize > RootPartitionRequire {
			rootPartitionRequire = needSize
		} else {
			rootPartitionRequire = RootPartitionRequire
		}
	}
	for dev, free := range _PartitionMap {
		var base float64
		if dev == repoLocation {
			repoSize, err := computeOstreeRepoSize()
			if err != nil {
				repoSize = 10.0
			}
			base = repoSize + rootPartitionRequire
		}
		out, err := exec.Command("/usr/bin/findmnt", dev).CombinedOutput()
		if err != nil {
			return nil, err
		}
		outLines := strings.Split(string(out), "\n")
		if len(outLines) < 2 {
			return nil, errors.New("failed to get mount point")
		}
		lines := strings.Split(outLines[1], " ")
		if lines[0] == "/" {
			if len(_PartitionMap) == 1 {
				base += NewSystemRootRequire
			}
			checkResult["systembase"] = fmt.Sprintf("%.2f", rootPartitionRequire+base)
			checkResult["systemfree"] = fmt.Sprintf("%.2f", free)
		} else {
			logger.Debug("init data base", base)
			checkResult["database"] = fmt.Sprintf("%.2f", NewSystemRootRequire+base)
			checkResult["datafree"] = fmt.Sprintf("%.2f", free)
		}
	}
	for k := range _PartitionMap {
		delete(_PartitionMap, k)
	}
	return checkResult, nil
}
