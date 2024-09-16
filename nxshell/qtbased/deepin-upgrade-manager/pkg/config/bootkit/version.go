// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package config

import (
	"deepin-upgrade-manager/pkg/module/generator"
	"deepin-upgrade-manager/pkg/module/util"
	"encoding/json"
	"errors"
	"sort"
	"strings"
)

type VersionConfig struct {
	Version     string `json:"version"`
	Kernel      string `json:"kernel"`
	Initrd      string `json:"initrd"`
	Scheme      string `json:"scheme"`
	DisplayInfo string `json:"display"`
	UUID        string `json:"uuid"`
}

type VersionListConf []*VersionConfig

type VersionListConfig struct {
	VersionList VersionListConf `json:"version_list"`
}

type VersionListInfo []*VersionListConfig

func (v VersionListConfig) checkConfig() error {
	for _, v := range v.VersionList {
		if len(v.Initrd) == 0 || len(v.Version) == 0 || len(v.Kernel) == 0 || len(v.Scheme) == 0 {
			return errors.New("failure to check configuration, configuration is not in conformity with the rules")
		}
	}
	return nil
}

func (list VersionListConf) Less(i, j int) bool {
	return generator.Less(list[i].Version, list[j].Version)

}

func (v VersionListConf) Sort() VersionListConf {
	list := v
	sort.SliceStable(list, func(i, j int) bool {
		return list.Less(i, j)
	})
	return list
}

func loadVersionConfig(cmd string) (*VersionListConfig, error) {
	var versionConfig VersionListConfig
	context := strings.Fields(cmd)
	if len(context) < 1 {
		return &versionConfig, errors.New("command does not conform to the rules")
	}
	var arg []string
	action := context[0]
	if len(context) > 1 {
		arg = context[1:]
	}
	content, _ := util.ExecCommandWithOut(action, arg)
	err := json.Unmarshal([]byte(content), &versionConfig)
	if err != nil {
		return &versionConfig, err
	}
	err = versionConfig.checkConfig()
	if err != nil {
		return &versionConfig, err
	}

	return &versionConfig, nil
}

func LoadVersionConfig(list ToolConfigList) (VersionListInfo, error) {
	var listInfo VersionListInfo
	for _, v := range list {
		config, err := loadVersionConfig(v.Submenu.InfoGenerator)
		if err != nil {
			continue
		}
		listInfo = append(listInfo, config)
	}
	return listInfo, nil
}
