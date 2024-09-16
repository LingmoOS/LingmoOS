// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package airplane_mode

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync"
)

const (
	configFile = "/var/lib/dde-daemon/airplane_mode/config.json"
)

// Config indicate each module config state
// when config is not set in the beginning, block is false as default
type Config struct {
	// config store all rfkill module config
	config map[rfkillType]bool

	mu sync.Mutex
}

// NewConfig create config obj
func NewConfig() *Config {
	cfg := &Config{
		config: make(map[rfkillType]bool),
	}
	return cfg
}

// LoadConfig load config from file
func (cfg *Config) LoadConfig() error {
	// read file
	buf, err := ioutil.ReadFile(configFile)
	if err != nil {
		return err
	}
	// marshal file to state
	err = json.Unmarshal(buf, &cfg.config)
	if err != nil {
		return err
	}
	return nil
}

// SaveConfig save config to file
func (cfg *Config) SaveConfig() error {
	// marshal config to buf
	buf, err := json.Marshal(&cfg.config)
	if err != nil {
		return err
	}
	// make dir
	err = os.MkdirAll(filepath.Dir(configFile), 0755)
	if err != nil {
		return err
	}
	// write config to file
	err = ioutil.WriteFile(configFile, buf, 0644)
	if err != nil {
		return err
	}
	return nil
}

// SetBlocked set ref config state
func (cfg *Config) SetBlocked(module rfkillType, blocked bool) {
	cfg.mu.Lock()
	defer cfg.mu.Unlock()
	cfg.config[module] = blocked
}

// GetBlocked get ref config state
// if config is not stored, rfkill is unblocked as default
func (cfg *Config) GetBlocked(module rfkillType) bool {
	cfg.mu.Lock()
	defer cfg.mu.Unlock()
	// get blocked state, if not exist, is blocked
	blocked, ok := cfg.config[module]
	if !ok {
		return false
	}
	return blocked
}
