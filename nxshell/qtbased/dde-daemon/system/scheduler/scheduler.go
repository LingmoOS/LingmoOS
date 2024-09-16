// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package scheduler

import (
	"os"
	"path/filepath"
	"strconv"
	"syscall"
	"time"

	"github.com/linuxdeepin/go-lib/procfs"
)

// 遍历所有进程, 设置优先级
func updateProcessesPriority(cfg *config) error {
	fileInfos, err := readDir("/proc")
	if err != nil {
		return err
	}

	for _, fileInfo := range fileInfos {
		name := fileInfo.Name()
		pid, err := strconv.Atoi(name)
		if err != nil {
			continue
		}

		setProcessPriority(cfg, pid)
	}
	return nil
}

// 设置进程优先级
func setProcessPriority(cfg *config, pid int) {
	exe, err := getProcessExe(pid)
	if err != nil {
		// 有些无法获取 exe, 比如内核线程 kworker/2:1-events
		return
	}
	pCfg := cfg.getPriority(exe)
	if pCfg == nil {
		// 无配置
		return
	}
	// 仅在有配置时设置优先级
	err = setProcessCpuPriority(pid, pCfg.CPU)
	if err != nil {
		logger.Warningf("set priority for process %d (exe: %v) failed: %v", pid, exe, err)
	}
}

// 获取进程可执行文件路径
func getProcessExe(pid int) (string, error) {
	exe, err := procfs.Process(pid).Exe()
	return exe, err
}

// 设置进程的 cpu 优先级（nice），包括所有线程
func setProcessCpuPriority(pid int, priority int) error {
	tasks, err := getProcessTasks(pid)
	if err != nil {
		return err
	}
	for _, taskId := range tasks {
		err = setCpuPriority(taskId, priority)
		if err != nil {
			return err
		}
	}
	return nil
}

// 获取进程的所有线程 id, 包括自身。
func getProcessTasks(pid int) ([]int, error) {
	fileInfos, err := readDir(filepath.Join("/proc", strconv.Itoa(pid), "task"))
	if err != nil {
		return nil, err
	}
	result := make([]int, 0, len(fileInfos))
	for _, fileInfo := range fileInfos {
		name := fileInfo.Name()
		taskId, err := strconv.Atoi(name)
		if err != nil {
			return nil, err
		}
		result = append(result, taskId)
	}
	return result, nil
}

// 读取目录，但是不排序
func readDir(dirname string) ([]os.FileInfo, error) {
	f, err := os.Open(dirname)
	if err != nil {
		return nil, err
	}
	list, err := f.Readdir(-1)
	_ = f.Close()
	if err != nil {
		return nil, err
	}
	return list, nil
}

// 设置线程的 cpu 优先级 nice 值， priority 范围是 19（低） ～ -20（高）。
func setCpuPriority(taskId int, priority int) error {
	err := syscall.Setpriority(syscall.PRIO_PROCESS, taskId, priority)
	return err
}

// 处理进程事件的周期间隔
const handleProcEventsIntervalSec = 5

// 更新所有进程优先级的周期间隔
const updateAllIntervalSec = 90

// 模块入口
func start() error {
	cfg, err := loadConfig()
	if err != nil {
		logger.Warning("load config failed:", err)
		return nil
	}
	logger.Debug("load config file:", cfg.filename)

	if !cfg.Enabled {
		logger.Info("scheduler module is disabled")
		return nil
	}

	var pm *procMonitor
	if cfg.ProcMonitorEnabled {
		pm = newProcMonitor(func() {
			// 定时器回调函数
			pids := pm.getAlivePids()
			for _, pid := range pids {
				setProcessPriority(cfg, int(pid))
			}
		})
		go func() {
			err := pm.listenProcEvents()
			if err != nil {
				logger.Warning(err)
			}
		}()
	}

	err = updateProcessesPriority(cfg)
	if err != nil {
		logger.Warning("updateProcessesPriority err:", err)
	}
	ticker := time.NewTicker(time.Second * updateAllIntervalSec)
	go func() {
		for range ticker.C {
			err := updateProcessesPriority(cfg)
			if err != nil {
				logger.Warning("updateProcessesPriority err:", err)
			}
		}
	}()

	return nil
}
