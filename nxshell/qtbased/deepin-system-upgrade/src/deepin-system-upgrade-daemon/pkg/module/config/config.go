// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package config

import (
	"errors"
	"io/ioutil"

	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"gopkg.in/yaml.v2"
)

var logger = log.NewLogger("deepin-system-upgrade/version")
var _Conf = umarshalConfigYaml(YamlConfigPath)

// DBus path and DBus interface
const (
	dbusPath      = "/org/deepin/SystemUpgrade1/ConfigManager"
	dbusInterface = "org.deepin.SystemUpgrade1.ConfigManager"
)

// Configuration files carried by the upgrade tool
const (
	YamlConfigPath = "/etc/deepin-system-upgrade/config.yaml"
	YamlBackupPath = "/etc/deepin-system-upgrade/backup.yaml"
)

type BackupConfig struct {
	BackupTarget BackupTarget `yaml:"target"`
}

type BackupTarget struct {
	BackupList []string `yaml:"backup_list"`
	HoldList   []string `yaml:"hold_list"`
}

type Config struct {
	Version     string     `yaml:"version"`
	Transfer    []Transfer `yaml:"transfer"`
	Source      []Source   `yaml:"source"`
	Target      []Target   `yaml:"target"`
	DebInstall  []string   `yaml:"debinstall"`
	BackupApps  bool       `yaml:"backupapps"`
	FullMigrate bool       `yaml:"fullmigrate"`
}

type Transfer struct {
	Minversion []MinVersion `yaml:"minversion"`
}

type MinVersion struct {
	Distro     string `yaml:"distro"`
	Subversion string `yaml:"subversion"`
}

type Source struct {
	Url       string `yaml:"url"`
	Type      string `yaml:"type"`
	Integrity bool   `yaml:"integrity"`
}

type Target struct {
	MergeDir string   `yaml:"merge_dirs"`
	Filter   []string `yaml:"filter"`
}

type ConfigManager struct {
	service *dbusutil.Service

	methods *struct {
		GetSource       func() `out:"sources"`
		GetTarget       func() `out:"target"`
		GetIsBackupApps func() `out:"backup"`
	}
}

func (*ConfigManager) GetInterfaceName() string {
	return dbusInterface
}

func newManager(service *dbusutil.Service) *ConfigManager {
	m := &ConfigManager{
		service: service,
	}
	return m
}

func (*ConfigManager) GetTarget() ([]Target, *dbus.Error) {
	if _Conf == nil {
		return nil, nil
	}
	return _Conf.Target, nil
}

func (*ConfigManager) GetSource() ([]Source, *dbus.Error) {
	if _Conf == nil {
		return nil, nil
	}
	return _Conf.Source, nil
}

func (*ConfigManager) GetIsBackupApps() (bool, *dbus.Error) {
	if _Conf == nil {
		return false, nil
	}
	return _Conf.BackupApps, nil
}

func (*ConfigManager) GetVersion() string {
	if _Conf == nil {
		return ""
	}
	return _Conf.Version
}

func (*ConfigManager) GetTransfer() []Transfer {
	if _Conf == nil {
		return nil
	}
	return _Conf.Transfer
}

func GetIsFullMigrate() (bool, error) {
	if _Conf == nil {
		return false, errors.New("_Conf interface is nil")
	}
	return _Conf.FullMigrate, nil
}

func ExportConfigManager(service *dbusutil.Service) error {
	m := newManager(service)
	return service.Export(dbusPath, m)
}

func umarshalBackupConfigYaml(filepath string) (*BackupConfig, error) {
	file, err := ioutil.ReadFile(filepath)
	if err != nil {
		logger.Warning("failed to read file:", err)
		return nil, err
	}
	var Config BackupConfig
	err = yaml.Unmarshal(file, &Config)
	if err != nil {
		logger.Warning("failed to unmarshal config:", err)
		return nil, err
	}
	return &Config, nil
}

func umarshalConfigYaml(filepath string) *Config {
	file, err := ioutil.ReadFile(filepath)
	if err != nil {
		logger.Warning("failed to read file:", err)
		return nil
	}
	var Config Config
	err = yaml.Unmarshal(file, &Config)
	if err != nil {
		logger.Warning("failed to unmarshal config:", err)
		return nil
	}
	return &Config
}

func GetTargetList() []Target {
	if _Conf == nil {
		return nil
	}
	return _Conf.Target
}

func GetPackageList() []string {
	if _Conf == nil {
		return nil
	}
	return _Conf.DebInstall
}

func GetBackupTarget() (BackupTarget, error) {
	conf, err := umarshalBackupConfigYaml(YamlBackupPath)
	return conf.BackupTarget, err
}
