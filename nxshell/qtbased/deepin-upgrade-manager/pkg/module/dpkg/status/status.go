// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// '/var/lib/dpkg/status' parser
package status

import (
	"bufio"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"os"
	"path/filepath"
	"reflect"
	"strings"
)

// Package info in '/var/lib/dpkg/status'
type PackageStatus struct {
	Package       string
	Status        string
	Priority      string
	Section       string
	InstalledSize string
	Maintainer    string
	Architecture  string
	MultiArch     string
	Source        string
	Version       string

	Contents []string
}
type PackageStatusList []*PackageStatus

const (
	_KEY_PACKAGE        = "Package"
	_KEY_STATUS         = "Status"
	_KEY_PRIORITY       = "Priority"
	_KEY_SECTION        = "Section"
	_KEY_INSTALLED_SIZE = "Installed-Size"
	_KEY_MAINTAINER     = "Maintainer"
	_KEY_ARCHITECTURE   = "Architecture"
	_KEY_MULTI_ARCH     = "Multi-Arch"
	_KEY_SOURCE         = "Source"
	_KEY_VERSION        = "Version"
)

const (
	_STATUS_DELIM = ": "
)

func DiffStatusFile(origFile, newFile string) (PackageStatusList, error) {
	origList, err := GetStatusList(origFile)
	if err != nil {
		return nil, err
	}
	newList, err := GetStatusList(newFile)
	if err != nil {
		return nil, err
	}
	var retList PackageStatusList
	for _, info := range newList {
		orig := origList.Exist(info)
		if info.Equal(orig) {
			continue
		}
		retList = append(retList, info)
	}
	return retList, nil
}

func MergeStatusList(srcFile string, list PackageStatusList) (PackageStatusList, error) {
	srcList, err := GetStatusList(srcFile)
	if err != nil {
		return nil, err
	}
	for _, info := range list {
		v := srcList.Exist(info)
		if v == nil {
			srcList = append(srcList, info)
			continue
		}
		v.Set(info)
	}
	return srcList, nil
}

func GetStatusList(filename string) (PackageStatusList, error) {
	fr, err := os.Open(filepath.Clean(filename))
	if err != nil {
		return nil, err
	}
	defer func() {
		if err := fr.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}()

	var statusList PackageStatusList
	scanner := bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) == 0 {
			continue
		}
		statusList = append(statusList, newPackageStatus(line, scanner))
	}
	return statusList, nil
}

func (list PackageStatusList) Exist(info *PackageStatus) *PackageStatus {
	for _, v := range list {
		if v.Package == info.Package && v.Architecture == info.Architecture {
			return v
		}
	}
	return nil
}

func (list PackageStatusList) Get(pkg, arch string) *PackageStatus {
	for _, v := range list {
		if v.Package == pkg && v.Architecture == arch {
			return v
		}
	}
	return nil
}

func (list PackageStatusList) ListPackage() []string {
	var pkgList []string
	for _, v := range list {
		pkgList = append(pkgList, v.Package+_STATUS_DELIM+v.Architecture)
	}
	return pkgList
}

func (list PackageStatusList) Save(filename string) error {
	tmpFile := filename + "-" + util.MakeRandomString(util.MinRandomLen)
	fw, err := os.OpenFile(filepath.Clean(tmpFile), os.O_WRONLY|os.O_TRUNC|os.O_CREATE, 0600)
	if err != nil {
		return err
	}
	defer func() {
		if err := fw.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}()
	for _, info := range list {
		_, err = fw.Write(info.Bytes())
		if err != nil {
			return err
		}
	}

	if !util.IsExists(filename) {
		return os.Rename(tmpFile, filename)
	}

	bakFile := filename + "-bak-" + util.MakeRandomString(util.MinRandomLen)
	err = os.Rename(filename, bakFile)
	if err != nil && err != os.ErrNotExist {
		_ = os.Remove(tmpFile)
		return err
	}

	err = os.Rename(tmpFile, filename)
	if err != nil {
		_ = os.Remove(tmpFile)
		_ = os.Rename(bakFile, filename)
		return err
	}
	_ = os.Remove(bakFile)
	return nil
}

func (info *PackageStatus) Equal(target *PackageStatus) bool {
	return reflect.DeepEqual(info, target)
}

func (info *PackageStatus) Set(target *PackageStatus) {
	info.InstalledSize = target.InstalledSize
	info.Maintainer = target.Maintainer
	info.MultiArch = target.MultiArch
	info.Priority = target.Priority
	info.Section = target.Section
	info.Source = target.Source
	info.Status = target.Status
	info.Version = target.Version
	info.Contents = target.Contents
}

func (info *PackageStatus) Bytes() []byte {
	lines := []string{_KEY_PACKAGE + _STATUS_DELIM + info.Package}
	lines = append(lines, _KEY_STATUS+_STATUS_DELIM+info.Status)
	if len(info.Priority) != 0 {
		lines = append(lines, _KEY_PRIORITY+_STATUS_DELIM+info.Priority)
	}
	if len(info.Section) != 0 {
		lines = append(lines, _KEY_SECTION+_STATUS_DELIM+info.Section)
	}
	lines = append(lines, _KEY_INSTALLED_SIZE+_STATUS_DELIM+info.InstalledSize)
	if len(info.Maintainer) != 0 {
		lines = append(lines, _KEY_MAINTAINER+_STATUS_DELIM+info.Maintainer)
	}
	lines = append(lines, _KEY_ARCHITECTURE+_STATUS_DELIM+info.Architecture)
	if len(info.MultiArch) != 0 {
		lines = append(lines, _KEY_MULTI_ARCH+_STATUS_DELIM+info.MultiArch)
	}
	if len(info.Source) != 0 {
		lines = append(lines, _KEY_SOURCE+_STATUS_DELIM+info.Source)
	}
	lines = append(lines, _KEY_VERSION+_STATUS_DELIM+info.Version)
	if len(info.Contents) != 0 {
		lines = append(lines, info.Contents...)
	}
	lines = append(lines, "\n")
	return []byte(strings.Join(lines, "\n"))
}

func newPackageStatus(line string, scanner *bufio.Scanner) *PackageStatus {
	var info PackageStatus

	parseStatusLine(line, &info)
	for scanner.Scan() {
		line = scanner.Text()
		if len(line) == 0 {
			break
		}

		parseStatusLine(line, &info)
	}
	return &info
}

func parseStatusLine(line string, info *PackageStatus) {
	items := strings.SplitN(line, _STATUS_DELIM, 2)
	if len(items) != 2 {
		info.Contents = append(info.Contents, line)
		return
	}

	switch items[0] {
	case _KEY_PACKAGE:
		info.Package = items[1]
	case _KEY_STATUS:
		info.Status = items[1]
	case _KEY_PRIORITY:
		info.Priority = items[1]
	case _KEY_SECTION:
		info.Section = items[1]
	case _KEY_INSTALLED_SIZE:
		info.InstalledSize = items[1]
	case _KEY_MAINTAINER:
		info.Maintainer = items[1]
	case _KEY_ARCHITECTURE:
		info.Architecture = items[1]
	case _KEY_MULTI_ARCH:
		info.MultiArch = items[1]
	case _KEY_SOURCE:
		info.Source = items[1]
	case _KEY_VERSION:
		info.Version = items[1]
	default:
		info.Contents = append(info.Contents, line)
	}
}
