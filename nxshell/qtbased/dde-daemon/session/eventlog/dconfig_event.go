// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package eventlog

import (
	"encoding/json"
	"errors"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	lastore "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.lastore1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

type dconfigLogCollector struct {
	sessionService  *dbusutil.Service
	systemService   *dbusutil.Service
	writeEventLogFn writeEventLogFunc
	sysSigLoop      *dbusutil.SignalLoop //后续方便监听信号
	lastoreObj      lastore.Lastore
	info            *configInfo
	infoPropMu      sync.Mutex
}

type configInfo struct {
	Tid                 TidTyp
	AutoCheckUpdate     bool
	AutoDownloadUpdate  bool
	AutoInstallUpdate   bool
	UpdatesNotification bool
	DeveloperMode       bool
}

const (
	ConfigTid TidTyp = 1000600000
)

func init() {
	register("dconfig", newDconfigLogCollector())
}

func newDconfigLogCollector() *dconfigLogCollector {
	return &dconfigLogCollector{
		info: &configInfo{
			Tid: ConfigTid,
		},
	}
}

func (c *dconfigLogCollector) Init(service *dbusutil.Service, fn writeEventLogFunc) error {
	logger.Info("config collector init")
	if service == nil || fn == nil {
		return errors.New("failed to init dconfigLogCollector: error args")
	}
	var err error
	c.sessionService = service
	c.writeEventLogFn = fn
	c.systemService, err = dbusutil.NewSystemService()
	if err != nil {
		return err
	}
	c.lastoreObj = lastore.NewLastore(c.systemService.Conn())
	c.sysSigLoop = dbusutil.NewSignalLoop(c.systemService.Conn(), 10)
	c.lastoreObj.InitSignalExt(c.sysSigLoop, true)
	c.sysSigLoop.Start()
	return nil
}

func (c *dconfigLogCollector) Collect() error {
	time.AfterFunc(10*time.Minute, func() {
		c.updateLastoreConfig(c.systemService)
		c.updateSyncConfig(c.systemService)
		c.infoPropMu.Lock()
		defer c.infoPropMu.Unlock()
		c.writeDConfigLog(c.info)
	})
	return nil
}

func (c *dconfigLogCollector) Stop() error {
	if c.lastoreObj != nil {
		c.lastoreObj.RemoveAllHandlers()
	}
	if c.sysSigLoop != nil {
		c.sysSigLoop.Stop()
	}
	return nil
}

func (c *dconfigLogCollector) updateLastoreConfig(service *dbusutil.Service) (err error) {
	c.infoPropMu.Lock()
	defer c.infoPropMu.Unlock()
	c.info.AutoCheckUpdate, err = c.lastoreObj.Updater().AutoCheckUpdates().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	c.info.AutoDownloadUpdate, err = c.lastoreObj.Updater().AutoDownloadUpdates().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	c.info.AutoInstallUpdate, err = c.getLastoreBoolProp(service, "AutoInstallUpdates")
	if err != nil {
		logger.Warning(err)
	}
	c.info.UpdatesNotification, err = c.getLastoreBoolProp(service, "UpdateNotify")
	if err != nil {
		logger.Warning(err)
	}

	return nil
}

func (c *dconfigLogCollector) updateSyncConfig(service *dbusutil.Service) error {
	c.infoPropMu.Lock()
	defer c.infoPropMu.Unlock()
	syncObj := service.Conn().Object("com.deepin.sync.Helper", "/com/deepin/sync/Helper")
	var ret bool
	err := syncObj.Call("com.deepin.sync.Helper.IsDeveloperMode", 0).Store(&ret)
	if err != nil {
		logger.Warning(err)
		return err
	}
	c.info.DeveloperMode = ret
	return nil
}

func (c *dconfigLogCollector) getLastoreBoolProp(service *dbusutil.Service, propName string) (bool, error) {
	lastoreObj := service.Conn().Object("com.deepin.lastore", "/com/deepin/lastore")
	var ret dbus.Variant
	err := lastoreObj.Call("org.freedesktop.DBus.Properties.Get", 0, "com.deepin.lastore.Updater", propName).Store(&ret)
	if err != nil {
		logger.Warning(err)
		return false, err
	}
	if ret.Signature().String() != "b" {
		return false, errors.New("not excepted value type")
	}
	return ret.Value().(bool), nil
}

func (c *dconfigLogCollector) writeDConfigLog(info *configInfo) error {
	content, err := json.Marshal(info)
	if err != nil {
		return err
	}
	if c.writeEventLogFn != nil {
		c.writeEventLogFn(string(content))
	}

	return nil
}
