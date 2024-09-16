// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package generator

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/repo/branch"
	"deepin-upgrade-manager/pkg/module/util"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"strings"
)

type ListManager struct {
	Path        string
	versionList []string
}

var listManager ListManager

func (list ListManager) Len() int {
	return len(list.versionList)
}

func (list *ListManager) Save() error {
	var data string
	_ = os.MkdirAll(filepath.Dir(list.Path), 0644)
	for _, v := range list.versionList {
		if len(v) == 0 {
			continue
		}
		data = data + v + "\n"
	}
	err := ioutil.WriteFile(list.Path, []byte(data), 0600)
	if err != nil {
		return err
	}
	return nil
}

func (list *ListManager) NewVersion(distribution string) (string, error) {
	var version string

	if list.Len() == 0 {
		version = branch.GenInitName(distribution)
		return version, nil
	}
	branchName := list.versionList[list.Len()-1]
	if !branch.IsValid(branchName) {
		return "", fmt.Errorf("invalid branch name: %s", branchName)
	}
	return branch.Increment(branchName)
}

func correctVersionList(list []string) []string {
	var new []string
	for _, v := range list {
		if branch.IsValid(v) {
			new = append(new, v)
		}
	}
	return new
}

func Less(v1, v2 string) bool {
	var branchlist branch.BranchList
	branchlist = append(branchlist, v1)
	branchlist = append(branchlist, v2)
	return branchlist.Less(0, 1)
}

func Sort(list []string) []string {
	correctList := correctVersionList(list)
	var branchlist branch.BranchList
	branchlist = append(branchlist, correctList...)
	sort.SliceStable(correctList, func(i, j int) bool {
		return branchlist.Less(i, j)
	})
	return list
}

func readList(localPath string) ([]string, error) {
	var versions []string
	file, err := os.Open(filepath.Clean(localPath))
	if err != nil {
		return versions, err
	}
	defer func() {
		if err := file.Close(); err != nil {
			logger.Warningf("error closing file: %v", err)
		}
	}()

	content, err := ioutil.ReadAll(file)
	if err != nil {
		return versions, err
	}
	list := strings.Split(string(content), "\n")
	for _, v := range list {
		if len(strings.TrimSpace(v)) == 0 {
			continue
		}
		versions = append(versions, v)
	}

	return versions, nil
}

func Load(localPath string) (ListManager, error) {
	var err error
	listManager.Path = localPath
	if util.IsExists(localPath) {
		listManager.versionList, err = readList(localPath)
		if err != nil {
			listManager.versionList = listManager.versionList[0:0]
			return listManager, err
		}
	} else {
		listManager.versionList = listManager.versionList[0:0]
	}
	return listManager, nil
}

func (list *ListManager) Append(version string) {
	list.versionList = append(list.versionList, version)
}

func (list *ListManager) Delete(version string) {
	res := make([]string, 0, len(list.versionList))
	for _, v := range list.versionList {
		if v != version {
			res = append(res, v)
		}
	}
	list.versionList = res
}
