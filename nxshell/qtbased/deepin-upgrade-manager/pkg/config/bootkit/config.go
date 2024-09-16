// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package config

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path/filepath"
)

type KitConfig struct {
	ConfigVersion       string `json:"config_version"`
	Data                string `json:"data_path"`
	Config              string `json:"config_dir"`
	MaxVersionRetention int    `json:"max_version_retention"`
}

type kitConfig *KitConfig

type BootConfig struct {
	filename string

	Kit kitConfig
}

func (c *BootConfig) BootPrepare() error {
	err := os.MkdirAll(c.Kit.Config, 0644)
	if err != nil {
		return err
	}
	return nil
}

func BootloadFile(infos *BootConfig, dirpath string) error {
	infos.filename = dirpath
	content, err := ioutil.ReadFile(filepath.Clean(dirpath))
	if err != nil {
		return nil
	}
	err = json.Unmarshal(content, &infos.Kit)
	if err != nil {
		return nil
	}
	return nil
}

func BootLoadConfig(dirpath string) (*BootConfig, error) {
	var info BootConfig
	err := BootloadFile(&info, dirpath)
	if err != nil {
		return nil, err
	}
	info.filename = dirpath

	return &info, nil
}
