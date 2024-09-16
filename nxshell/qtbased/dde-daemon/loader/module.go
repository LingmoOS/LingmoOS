// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package loader

import (
	"fmt"
	"sync"

	"github.com/godbus/dbus/v5"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

type Module interface {
	Name() string
	IsEnable() bool
	Enable(bool) error
	GetDependencies() []string
	SetLogLevel(log.Priority)
	LogLevel() log.Priority
	WaitEnable() // TODO: should this function return when modules enable failed?
	ModuleImpl
}

type Modules map[string]Module

type ModuleImpl interface {
	Start() error // please keep Start sync, please return err, err log will be done by loader
	Stop() error
}

type ModuleBase struct {
	impl    ModuleImpl
	enabled bool
	name    string
	log     *log.Logger
	wg      sync.WaitGroup
}

const (
	dsettingsAppID           = "org.deepin.dde.daemon"
	dsettingsResource        = "org.deepin.dde.daemon.logger"
	dsettingsKeyEnabled      = "enabled"
	dsettingsKeyDefaultLevel = "defaultLevel"
)

var (
	sysSigLoop     *dbusutil.SignalLoop
	sysSigLoopOnce sync.Once
)

func getLogPriority(level string) log.Priority {
	switch level {
	case "fatal":
		return log.LevelFatal
	case "panic":
		return log.LevelPanic
	case "error":
		return log.LevelError
	case "warning":
		return log.LevelWarning
	case "info":
		return log.LevelInfo
	case "debug":
		return log.LevelDebug
	}

	return log.LevelDisable
}

func getLogLevel(priority log.Priority) string {
	switch priority {
	case log.LevelFatal:
		return "fatal"
	case log.LevelPanic:
		return "panic"
	case log.LevelError:
		return "error"
	case log.LevelWarning:
		return "warning"
	case log.LevelInfo:
		return "info"
	case log.LevelDebug:
		return "debug"
	}

	return ""
}

func NewModuleBase(name string, impl ModuleImpl, logger *log.Logger) *ModuleBase {
	m := &ModuleBase{
		name: name,
		impl: impl,
		log:  logger,
	}

	// 此为等待「enabled」的 WaitGroup，故在 enable 完成之前，需要一直为等待状态。
	// 其他依赖当前模块的模块启动时，可能还没有调用过当前模块的 Enable，所以不能放在 Enable 中。
	m.wg.Add(1)

	return m
}

func (d *ModuleBase) setupDSGLogLeveL() {
	sysSigLoopOnce.Do(func() {
		conn, err := dbus.SystemBus()
		if err != nil {
			d.log.Warning(err)
			return
		}
		sysSigLoop = dbusutil.NewSignalLoop(conn, 10)
		sysSigLoop.Start()
	})

	if sysSigLoop == nil {
		return
	}

	ds := ConfigManager.NewConfigManager(sysSigLoop.Conn())
	dsPath, err := ds.AcquireManager(0, dsettingsAppID, dsettingsResource, "")
	if err != nil {
		d.log.Warning(err)
		return
	}

	dsManager, err := ConfigManager.NewManager(sysSigLoop.Conn(), dsPath)
	if err != nil {
		d.log.Warning(err)
		return
	}

	oldPriority := d.log.GetLogLevel()

	fn := func(reset bool) {
		if v, err := dsManager.Value(0, dsettingsKeyEnabled); err != nil {
			return
		} else if !v.Value().(bool) {
			if reset {
				d.log.Info("reset log level to", getLogLevel(oldPriority))
				d.log.SetLogLevel(oldPriority)
			}
			return
		}

		v, err := dsManager.Value(0, d.name)
		if err != nil {
			d.log.Warning(err)
			return
		}
		level := v.Value().(string)
		if level == "" {
			v, err := dsManager.Value(0, dsettingsKeyDefaultLevel)
			if err != nil {
				d.log.Warning(err)
				return
			}
			level = v.Value().(string)
		}
		d.log.Infof("set %s log level %s", d.name, level)
		if v := getLogPriority(level); v != log.LevelDisable {
			d.log.SetLogLevel(v)
		}
	}

	fn(false)

	dsManager.InitSignalExt(sysSigLoop, true)
	dsManager.ConnectValueChanged(func(key string) {
		if key == d.name || key == dsettingsKeyEnabled || key == dsettingsKeyDefaultLevel {
			fn(key == dsettingsKeyEnabled)
		}
	})
}

func (d *ModuleBase) doEnable(enable bool) error {
	if d.impl != nil {
		fn := d.impl.Stop
		if enable {
			fn = d.impl.Start
		}

		if err := fn(); err != nil {
			return err
		}

		if enable {
			d.setupDSGLogLeveL()
			d.wg.Done()
		}
	}
	d.enabled = enable
	return nil
}

func (d *ModuleBase) Enable(enable bool) error {
	if d.enabled == enable {
		return fmt.Errorf("%s daemon is already started", d.name)
	}
	return d.doEnable(enable)
}

func (d *ModuleBase) IsEnable() bool {
	return d.enabled
}

func (d *ModuleBase) WaitEnable() {
	d.wg.Wait()
}

func (d *ModuleBase) Name() string {
	return d.name
}

func (d *ModuleBase) SetLogLevel(pri log.Priority) {
	d.log.SetLogLevel(pri)
}

func (d *ModuleBase) LogLevel() log.Priority {
	return d.log.GetLogLevel()
}
