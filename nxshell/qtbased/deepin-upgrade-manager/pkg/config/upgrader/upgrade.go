// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package config

import (
	"deepin-upgrade-manager/pkg/module/util"
	"encoding/json"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync"
)

const (
	curConfigVersion = "1.0"
)

type Server struct {
	Distribution string `json:"distribution"`
	Host         string `json:"host"`
	UpgradeRoute string `json:"upgrade_route"`
	VersionRoute string `json:"version_route"`
}

// UpgradeConfig upgrade config structure
type UpgradeConfig struct {
	ConfigVersion string `json:"config_version"`
	RepoDir       string `json:"repo_dir"`
	SnapshotDir   string `json:"snapshot_dir"`
	CacheDir      string `json:"cache_dir"`
	VendorDir     string `json:"vendor_dir"`
	OSDir         string `json:"os_dir"`
	EtcDir        string `json:"etc_dir"`
	ActiveVersion string `json:"active_version"`
	Compression   string `json:"compression"`
	SignAlg       string `json:"sign_alg"`

	Server *Server `json:"server"`

	filename string
	locker   sync.RWMutex
}

func (conf *UpgradeConfig) Save() error {
	conf.locker.RLock()
	data, err := json.Marshal(conf)
	conf.locker.RUnlock()
	if err != nil {
		return err
	}

	dst := conf.filename + "." + util.MakeRandomString(util.MinRandomLen)
	err = ioutil.WriteFile(dst, data, 0600)
	if err != nil {
		return nil
	}
	_, err = util.Move(conf.filename, dst, true)
	return err
}

func LoadUpgradeConfig(filename string) (*UpgradeConfig, error) {
	var conf = UpgradeConfig{
		ConfigVersion: curConfigVersion,
		filename:      filename,
	}
	err := loadFile(&conf, filename)
	if err != nil {
		return &conf, err
	}

	_ = os.MkdirAll(conf.SnapshotDir, 0750)
	_ = os.MkdirAll(conf.CacheDir, 0750)
	return &conf, nil
}

func loadFile(info interface{}, filename string) error {
	content, err := ioutil.ReadFile(filepath.Clean(filename))
	if err != nil {
		return err
	}
	return json.Unmarshal(content, info)
}
