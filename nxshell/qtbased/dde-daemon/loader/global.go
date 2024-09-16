// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package loader

import (
	"sync"

	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

var loaderInitializer sync.Once
var _loader *Loader

func getLoader() *Loader {
	loaderInitializer.Do(func() {
		_loader = &Loader{
			modules: Modules{},
			log:     log.NewLogger("daemon/loader"),
		}
	})
	return _loader
}

func SetService(s *dbusutil.Service) {
	l := getLoader()
	l.service = s
}

func GetService() *dbusutil.Service {
	return getLoader().service
}

func Register(m Module) {
	loader := getLoader()
	loader.AddModule(m)
}

func List() []Module {
	return getLoader().List()
}

func GetModule(name string) Module {
	return getLoader().GetModule(name)
}

func SetLogLevel(pri log.Priority) {
	getLoader().SetLogLevel(pri)
}

func EnableModules(enablingModules []string, disableModules []string, flag EnableFlag) error {
	return getLoader().EnableModules(enablingModules, disableModules, flag)
}

func ToggleLogDebug(enabled bool) {
	var priority log.Priority = log.LevelInfo
	if enabled {
		priority = log.LevelDebug
	}
	for _, m := range getLoader().modules {
		m.SetLogLevel(priority)
	}
}

func StartAll() {
	allModules := getLoader().List()
	modules := []string{}
	for _, module := range allModules {
		modules = append(modules, module.Name())
	}
	_ = getLoader().EnableModules(modules, []string{}, EnableFlagNone)
}

// TODO: check dependencies
func StopAll() {
	modules := getLoader().List()
	for _, module := range modules {
		_ = module.Enable(false)
	}
}
