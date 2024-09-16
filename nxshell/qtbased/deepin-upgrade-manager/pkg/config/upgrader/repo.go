// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package config

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strings"
	"sync"
)

const (
	DATA_YAML_PATH   = "ready/data.yaml"
	LOCAL_CONFIG_DIR = "/var/lib/deepin-upgrade-manager/config/"
)

type RepoConfig struct {
	RepoMountPoint string `json:"repo_mount_point"`
	DataOrigin     string `json:"data_origin"`
	Repo           string `json:"repo"`
	SnapshotDir    string `json:"snapshot_dir"`
	ConfigDir      string `json:"config_dir"`
	StageDir       string `json:"stage_dir"`
	AfterRun       string `json:"Afer_Run"`
	PlymouthTheme  string `json:"Plymouth_Theme"`

	SubscribeList []string `json:"subscribe_list"`
	FilterList    []string `json:"filter_list"`
}
type RepoListConfig []*RepoConfig

type Config struct {
	filename string
	dataname string
	locker   sync.RWMutex

	Version       string `json:"config_version"`
	Distribution  string `json:"distribution"`
	ActiveVersion string `json:"active_version"`
	CacheDir      string `json:"cache_dir"`

	AutoCleanup bool           `json:"auto_cleanup"`
	RepoList    RepoListConfig `json:"repo_list"`

	MaxVersionRetention int32 `json:"max_version_retention"`
	MaxRepoRetention    int32 `json:"max_repo_retention"`
}

func (c *Config) Prepare() error {
	for _, repo := range c.RepoList {
		err := os.MkdirAll(repo.StageDir, 0750)
		if err != nil {
			return err
		}
		err = os.MkdirAll(repo.SnapshotDir, 0750)
		if err != nil {
			return err
		}
		err = os.MkdirAll(repo.ConfigDir, 0750)
		if err != nil {
			return err
		}
	}
	repoConfig := filepath.Join(c.RepoList[0].ConfigDir, "config.json")
	if !util.IsExists(repoConfig) {
		util.CopyDir(path.Dir(c.filename), c.RepoList[0].ConfigDir,
			[]string{""}, []string{""}, false)
		c.Save()
		c.dataname = filepath.Join(c.RepoList[0].ConfigDir, DATA_YAML_PATH)
		c.filename = repoConfig
		// local config is max power
		localConfig := filepath.Join(LOCAL_CONFIG_DIR, DATA_YAML_PATH)
		if util.IsExists(localConfig) {
			util.CopyFile(localConfig, c.dataname, false)
		}
	}
	return nil
}

func (c *Config) GetRepoConfig(repoDir string) *RepoConfig {
	for _, v := range c.RepoList {
		if v.Repo == repoDir {
			return v
		}
	}
	return nil
}

func (c *Config) Save() error {
	c.locker.RLock()
	defer c.locker.RUnlock()
	data, err := json.Marshal(c)
	if err != nil {
		return err
	}

	tmpFile := c.filename + "-" + util.MakeRandomString(util.MinRandomLen)
	err = ioutil.WriteFile(tmpFile, data, 0600)
	if err != nil {
		return err
	}

	f, err := os.OpenFile(filepath.Clean(tmpFile), os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0600|0064)
	if err != nil {
		return err
	}
	n, err := f.Write(data)
	if err == nil && n < len(data) {
		err = io.ErrShortWrite
	}
	if err == nil {
		err = f.Sync()
	}
	if err1 := f.Close(); err == nil {
		err = err1
	}

	if err != nil {
		logger.Warning("failed save the repo info, err:", err)
	}

	_, err = util.Move(c.filename, tmpFile, true)

	return err
}

func (c *Config) ChangeRepoMountPoint(mountpoint string) {
	for _, v := range c.RepoList {
		if v.RepoMountPoint == mountpoint {
			continue
		}
		if mountpoint == "/" {
			v.Repo = strings.Replace(v.Repo, v.RepoMountPoint, "", 1)
			v.SnapshotDir = strings.Replace(v.SnapshotDir, v.RepoMountPoint, "", 1)
			v.StageDir = strings.Replace(v.StageDir, v.RepoMountPoint, "", 1)
			v.ConfigDir = strings.Replace(v.ConfigDir, v.RepoMountPoint, "", 1)
		} else {
			v.Repo = strings.Replace(v.Repo, v.RepoMountPoint, mountpoint, 1)
			v.SnapshotDir = strings.Replace(v.SnapshotDir, v.RepoMountPoint, mountpoint, 1)
			v.StageDir = strings.Replace(v.StageDir, v.RepoMountPoint, mountpoint, 1)
			v.ConfigDir = strings.Replace(v.ConfigDir, v.RepoMountPoint, mountpoint, 1)
		}

		v.RepoMountPoint = mountpoint
	}
}

func (c *Config) SetDistribution(version string) {
	if c.Distribution != version {
		c.Distribution = version
	}
}

func (c *Config) SetCacheDir(dir string) {
	c.CacheDir = dir
}

func (c *Config) AppendCommit(dirs []string, isClear bool) {
	if isClear {
		c.RepoList[0].SubscribeList = c.RepoList[0].SubscribeList[:0]
	}
	for _, v := range dirs {
		if len(v) == 0 || util.IsRootSame(c.RepoList[0].SubscribeList, v) {
			continue
		} else {
			c.RepoList[0].SubscribeList = append(c.RepoList[0].SubscribeList, v)
		}
	}
}

func (c *Config) AppendFilter(dirs []string, isClear bool) {
	if isClear {
		c.RepoList[0].FilterList = c.RepoList[0].FilterList[:0]
	}
	for _, v := range dirs {
		if len(v) == 0 || util.IsRootSame(c.RepoList[0].FilterList, v) {
			continue
		} else {
			c.RepoList[0].FilterList = append(c.RepoList[0].FilterList, v)
		}
	}
}

func (c *Config) LoadData(path string) {
	dataCf, err := LoadDataConfig(path)
	if err != nil {
		logger.Warning(err)
	}
	c.RepoList[0].DataOrigin = path

	c.AppendCommit(dataCf.Target.Backup_list, true)
	c.AppendFilter(dataCf.Target.Hold_list, true)
	c.RepoList[0].AfterRun = dataCf.Target.After_run
	c.RepoList[0].PlymouthTheme = dataCf.Target.Plymouth_theme
	const versionManager = "/var/lib/deepin-boot-kit"
	for _, v := range c.RepoList[0].SubscribeList {
		if strings.HasPrefix(c.RepoList[0].RepoMountPoint, v) {
			c.AppendFilter([]string{filepath.Dir(c.RepoList[0].Repo)}, false)
		}
		//  Special handling to prevent the version number error
		if strings.HasPrefix(versionManager, v) {
			c.AppendFilter([]string{versionManager}, false)
		}
	}
}

func (c *Config) LoadReadyData() {
	if !util.IsExists(c.dataname) {
		return
	}
	logger.Debug("load ready config path: ", c.dataname)
	c.LoadData(c.dataname)
	c.Save()
}

func (c *Config) LoadVersionData(version, rootDir string) error {
	if !util.IsExists(c.dataname) || len(version) == 0 {
		return fmt.Errorf("failed load version yaml config, path:%s, version:%s", c.dataname, version)
	}
	versionConfigPath := filepath.Join(rootDir, c.RepoList[0].ConfigDir, version, "data.yaml")
	logger.Debug("load version config path: ", versionConfigPath)
	c.LoadData(versionConfigPath)
	c.Save()
	return nil
}

func (c *Config) SetVersionConfig(version string) {
	if len(version) == 0 {
		return
	}
	versionConfig := filepath.Join(c.RepoList[0].ConfigDir, version)
	os.MkdirAll(versionConfig, 0750)
	util.CopyFile(c.dataname, filepath.Join(versionConfig, "data.yaml"), false)
}

func (c *Config) ReadyDataPath() string {
	path := filepath.Join(c.RepoList[0].ConfigDir, DATA_YAML_PATH)
	if util.IsExists(path) {
		return path
	}
	localConfig := filepath.Join(LOCAL_CONFIG_DIR, DATA_YAML_PATH)
	if util.IsExists(localConfig) {
		return localConfig
	}
	return c.dataname
}

func (c *Config) SetReadyData(datapath string) error {
	if !util.IsExists(datapath) {
		return errors.New("failed set config, file does not exist")
	}
	_, err := LoadDataConfig(datapath)
	if err != nil {
		return err
	}
	repoPath := filepath.Join(c.RepoList[0].ConfigDir, DATA_YAML_PATH)
	if util.IsExists(repoPath) {
		util.CopyFile(datapath, repoPath, false)
		logger.Debugf("set %s to %s", datapath, repoPath)
		return nil
	}
	localPath := filepath.Join(LOCAL_CONFIG_DIR, DATA_YAML_PATH)
	os.MkdirAll(path.Dir(localPath), 0750)
	util.CopyFile(datapath, localPath, false)
	logger.Debugf("set %s to %s", datapath, localPath)
	return nil
}

func (c *Config) ReLoadConfig(rootDir, repoMount string) (*Config, error) {
	repoConfig := filepath.Join(rootDir, c.RepoList[0].ConfigDir, "config.json")
	info := new(Config)
	if util.IsExists(repoConfig) {
		c.Save()
		c.filename = repoConfig
		err := loadFile(&info, c.filename)

		if err != nil {
			return c, err
		}
		c = info
		c.filename = repoConfig
		c.ChangeRepoMountPoint(repoMount)
		c.dataname = filepath.Join(path.Dir(c.filename), DATA_YAML_PATH)
		c.Save()
		return info, nil
	}
	return c, nil
}

func LoadConfig(filename, rootDir string) (*Config, error) {
	var info Config
	err := loadFile(&info, filename)
	if err != nil {
		return nil, err
	}
	repoConfig := filepath.Join(rootDir, info.RepoList[0].ConfigDir, "config.json")
	if util.IsExists(repoConfig) && repoConfig != filename && util.IsValidJson(repoConfig) {
		filename = repoConfig
		err := loadFile(&info, filename)
		if err != nil {
			return nil, err
		}
	}
	info.filename = filename
	info.dataname = filepath.Join(path.Dir(filename), DATA_YAML_PATH)
	logger.Debugf("using config file path: %s, using data file path: %s", filename, info.dataname)
	return &info, nil
}
