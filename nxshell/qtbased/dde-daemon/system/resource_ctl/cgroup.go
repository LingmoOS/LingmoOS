// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package resource_control

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"syscall"
	"time"

	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/log"
)

const (
	configPath = "/etc/deepin/daemon/resource-control.json"
)

func toSystemdPath(path string) string {
	return "/sys/fs/cgroup/systemd/" + path
}

func toCpuPath(path string) string {
	return "/sys/fs/cgroup/cpu/" + path
}

func toMemPath(path string) string {
	return "/sys/fs/cgroup/memory/" + path
}

type AppConfig struct {
	Memory *struct {
		Limit uint
	} `json:"Memory"`
	Cpu *struct {
		Limit uint
		Share uint
	} `json:"Cpu"`
}

type config map[string]AppConfig

func loadConfig() (cfg config, err error) {
	defer func() {
		if err != nil {
			err = fmt.Errorf("failed to load config: %w", err)
		}
	}()

	var content []byte

	content, err = ioutil.ReadFile(configPath)
	if err != nil {
		return
	}

	cfg = config{}
	err = json.Unmarshal(content, &cfg)

	return
}

func getTasksFromFile(path string) (tasks [][]byte, err error) {
	defer func() {
		if err != nil {
			err = fmt.Errorf("failed to get tasks from %s: %w", path, err)
		}
	}()

	var tasksFile *os.File

	tasksFile, err = os.Open(path)
	if err != nil {
		return
	}

	var content []byte

	content, err = ioutil.ReadAll(tasksFile)
	if err != nil {
		return
	}

	tasks = bytes.Split(content, []byte("\n"))

	return
}

func cleanupCgroup(path string) (shouldRemove bool) {
	shouldRemove = false
	var err error
	var pids [][]byte
	pids, err = getTasksFromFile(filepath.Join(toSystemdPath(path), "tasks"))

	if err != nil || pids == nil || len(pids) == 0 {
		logger.Debugf("failed to get cpu tasks of %v", path)
		shouldRemove = true
		syscall.Rmdir(toCpuPath(path))
		syscall.Rmdir(toMemPath(path))
	}

	return
}

var logger = log.NewLogger("daemon/system/resource_control")

func init() {
	loader.Register(newDaemon(logger))
}

type Daemon struct {
	*loader.ModuleBase
}

func newDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("resource_control", daemon, logger)
	return daemon
}

func (d *Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {

	cfg, err := loadConfig()
	if err != nil {
		logger.Infof("error: %v", err)
	}
	if cfg == nil || len(cfg) == 0 {
		logger.Infof("resource_ctl disabled")
		return nil
	}

	go func() {
		reg, err := regexp.Compile("^/sys/fs/cgroup/systemd/(.*app-dde-(.+)-.+\\.scope)$")
		if err != nil {
			panic(err)
		}
		list := map[string]struct{}{}

		for range time.NewTicker(time.Second * 3).C {
			for k := range list {
				if cleanupCgroup(k) {
					delete(list, k)
				}
			}

			if err = filepath.Walk("/sys/fs/cgroup/systemd",
				func(path string, info os.FileInfo, err error) error {
					// FIXME(chenlinxuan): should skip some dirs

					if err != nil {
						return err
					}

					matches := reg.FindStringSubmatch(path)
					if len(matches) == 0 {
						return nil
					}

					if appCfg, ok := cfg[matches[2]]; !ok {
						return nil
					} else {
						path = matches[1]
						setupCgroup(path, appCfg)
						list[path] = struct{}{}
						return nil
					}
				},
			); err != nil {
				logger.Infof("failed to walk %v: resource_ctl disable", err)
				return
			}
		}
	}()

	return nil
}

func setupCgroup(path string, cfg AppConfig) {
	var err error

	var pids [][]byte
	pids, err = getTasksFromFile(filepath.Join(toSystemdPath(path), "tasks"))
	if err != nil {
		logger.Errorf("failed to get tasks of %v: %v", path, err)
		return
	}

	if cfg.Cpu != nil {
		cpuPath := toCpuPath(path)
		if err := os.MkdirAll(cpuPath, 0755); err != nil {
			logger.Errorf("failed to create cpu cgroup for %v: %v", path, err)
			return
		}

		if cfg.Cpu.Limit != 0 {
			if file, err := os.OpenFile(filepath.Join(cpuPath, "cpu.cfs_quota_us"), os.O_WRONLY, 0644); err != nil {
				logger.Errorf("failed to open cpu.cfs_quota_us: %v", err)
				return
			} else {
				defer file.Close()
				if _, err := file.Write([]byte(fmt.Sprintf("%v", cfg.Cpu.Limit))); err != nil {
					logger.Errorf("failed to set cpu.cfs_quota_us: %v", err)
					return
				}
			}
		}

		if cfg.Cpu.Share != 0 {
			if file, err := os.OpenFile(filepath.Join(cpuPath, "cpu.shares"), os.O_WRONLY, 0644); err != nil {
				logger.Errorf("failed to open cpu.shares: %v", err)
				return
			} else {
				defer file.Close()
				if _, err := file.Write([]byte(fmt.Sprintf("%v", cfg.Cpu.Share))); err != nil {
					logger.Errorf("failed to set cpu.shares: %v", err)
					return
				}
			}
		}

		if file, err := os.OpenFile(filepath.Join(cpuPath, "tasks"), os.O_WRONLY, 0644); err == nil {
			for _, pid := range pids {
				_, err = file.Write(pid)
				if err != nil {
					logger.Errorf("failed to set cgroup of task %v", err)
				}
			}
		} else {
			logger.Errorf("failed to open %v: %v", cpuPath, err)
		}
	}

	if cfg.Memory != nil {

		memPath := toMemPath(path)

		if err := os.MkdirAll(memPath, 0755); err != nil {
			logger.Errorf("failed to create memory cgroup for %v: %v", path, err)
			return
		}

		if cfg.Memory.Limit != 0 {
			if file, err := os.OpenFile(filepath.Join(memPath, "memory.limit_in_bytes"), os.O_WRONLY, 0644); err != nil {
				logger.Errorf("failed to open memory.limit_in_bytes")
				return
			} else {
				if _, err := file.Write([]byte(fmt.Sprintf("%v\n", cfg.Memory.Limit))); err != nil {
					logger.Errorf("failed to set memory.limit_in_bytes: %v", err)
					return
				}
			}
		}

		if file, err := os.OpenFile(filepath.Join(memPath, "tasks"), os.O_WRONLY, 0644); err == nil {
			for _, pid := range pids {
				_, err = file.Write(pid)
				if err != nil {
					logger.Errorf("failed to set cgroup of task %v", err)
				}
			}
		} else {
			logger.Errorf("failed to open %v: %v", memPath, err)
		}
	}
}

func (d *Daemon) Stop() error {
	// TODO:
	return nil
}
