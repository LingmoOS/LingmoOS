// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package config

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"path/filepath"
)

type SubmenuConfig struct {
	InfoGenerator string `json:"version_list"`
}

type InirdConfig struct {
	ScriptDir string `json:"script_path"`
}

type ToolConfig struct {
	Submenu SubmenuConfig `json:"submenu"`
	Initrd  InirdConfig   `json:"initrd"`

	ConfigFileName string
}

type ToolConfigList []*ToolConfig

func (c *ToolConfig) CheckConfig() error {
	if len(c.Initrd.ScriptDir) == 0 || len(c.Submenu.InfoGenerator) == 0 {
		return errors.New("failure to check configuration, configuration is not in conformity with the rules")
	}
	return nil
}

func LoadToolConfig(dir string) (ToolConfigList, error) {
	var list ToolConfigList
	fiList, err := ioutil.ReadDir(dir)

	if err != nil {
		return list, err
	}
	for _, fi := range fiList {
		var toolConfig *ToolConfig
		if fi.IsDir() {
			continue
		}
		content, err := ioutil.ReadFile(filepath.Clean(filepath.Join(dir, fi.Name())))
		if err != nil {
			continue
		}
		err = json.Unmarshal(content, &toolConfig)
		if err != nil {
			continue
		}
		toolConfig.ConfigFileName = filepath.Join(dir, fi.Name())
		list = append(list, toolConfig)
	}
	return list, nil
}
