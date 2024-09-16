// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package calltrace

import (
	"time"

	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/dde-daemon/loader"
)

var (
	logger = log.NewLogger("daemon/calltrace")
)

type Daemon struct {
	ct   *Manager
	quit chan bool
	*loader.ModuleBase
}

func init() {
	loader.Register(NewDaemon())
}

func NewDaemon() *Daemon {
	var d = new(Daemon)
	d.ModuleBase = loader.NewModuleBase("calltrace", d, logger)
	return d
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

// Start launch calltrace module
func (d *Daemon) Start() error {
	if d.quit != nil {
		return nil
	}

	d.quit = make(chan bool)
	go d.loop()
	return nil
}

// Stop terminate calltrace module
func (d *Daemon) Stop() error {
	if d.quit == nil {
		return nil
	}

	d.quit <- true
	if d.ct != nil {
		d.ct.SetAutoDestroy(1)
	}
	logger.Info("--------Terminate calltrace loop")
	return nil
}

func (d *Daemon) loop() {
	s := gio.NewSettings("com.deepin.dde.calltrace")
	cpuPercentage := s.GetInt("cpu-percentage")
	memUsage := s.GetInt("mem-usage")
	duration := s.GetInt("duration")
	s.Unref()

	logger.Info("--------Start calltrace loop")
	d.handleProcessStat(cpuPercentage, memUsage, duration)
	ticker := time.NewTicker(time.Second * 30)
	for {
		select {
		case _, ok := <-ticker.C:
			if !ok {
				logger.Error("Invalid ticker event, exit loop!")
				return
			}
			d.handleProcessStat(cpuPercentage, memUsage, duration)
		case <-d.quit:
			ticker.Stop()
			close(d.quit)
			d.quit = nil
			return
		}
	}
}

func (d *Daemon) handleProcessStat(cpuPercentage, memUsage, duration int32) {
	cpu, _ := getCPUPercentage()
	mem, _ := getMemoryUsage()
	logger.Debugf("-----------Handle process stat, cpu: %#v, mem: %#v, ct: %p", cpu, mem, d.ct)
	if cpu > float64(cpuPercentage) || mem > int64(memUsage)*1024 {
		if d.ct == nil {
			d.ct, _ = NewManager(uint32(duration))
		}
	} else {
		if d.ct == nil {
			return
		}
		d.ct.SetAutoDestroy(1)
	}
}
