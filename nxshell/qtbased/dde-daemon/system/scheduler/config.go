// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package scheduler

import (
	"encoding/json"
	"io/ioutil"
	"path/filepath"
)

type config struct {
	filename           string
	Processes          map[string]*priorityCfg `json:"processes"`
	Enabled            bool                    `json:"enabled"`
	ProcMonitorEnabled bool                    `json:"procMonitorEnabled"`
}

type priorityCfg struct {
	CPU int `json:"cpu"`
}

func (c *config) getPriority(exe string) *priorityCfg {
	if pCfg, ok := c.Processes[exe]; ok {
		return pCfg
	}
	name := filepath.Base(exe)
	if pCfg, ok := c.Processes[name]; ok {
		return pCfg
	}
	return nil
}

func loadConfigAux(filename string) (*config, error) {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var cfg config
	err = json.Unmarshal(content, &cfg)
	if err != nil {
		return nil, err
	}
	cfg.filename = filename
	return &cfg, nil
}

func loadConfig() (*config, error) {
	paths := []string{
		"/etc/deepin/scheduler/config.json",       // 用户
		"/usr/share/deepin/scheduler/config.json", // 软件包
	}
	var lastErr error
	for _, p := range paths {
		cfg, err := loadConfigAux(p)
		if err != nil {
			lastErr = err
			continue
		}
		return cfg, nil
	}
	return nil, lastErr
}
