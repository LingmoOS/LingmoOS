// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"errors"
	"fmt"
	"os"
	"strings"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/session"
	"github.com/linuxdeepin/dde-daemon/calltrace"
	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-gir/glib-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gsettings"
	"github.com/linuxdeepin/go-lib/log"
)

//go:generate dbusutil-gen em -type SessionDaemon

const (
	ProfTypeCPU = "cpu"    //nolint
	ProfTypeMem = "memory" //nolint

	dbusPath        = "/org/deepin/dde/Daemon1"
	dbusServiceName = "org.deepin.dde.Daemon1"
	dbusInterface   = dbusServiceName
	configManagerId = "org.desktopspec.ConfigManager"
)

func runMainLoop() {
	err := gsettings.StartMonitor()
	if err != nil {
		logger.Fatal(err)
	}

	glib.StartLoop()
	logger.Info("Loop has been terminated!")
	os.Exit(0)
}

func getEnableFlag(flag *Flags) loader.EnableFlag {
	enableFlag := loader.EnableFlagIgnoreMissingModule

	if flag.IgnoreMissingModules {
		enableFlag = loader.EnableFlagNone
	}

	if flag.ForceStart {
		enableFlag |= loader.EnableFlagForceStart
	}

	return enableFlag
}

type SessionDaemon struct {
	flags                *Flags
	log                  *log.Logger
	part1EnabledModules  []string
	part1DisabledModules []string
	part2EnabledModules  []string
	part2DisabledModules []string

	configManagerPath dbus.ObjectPath
	systemSigLoop     *dbusutil.SignalLoop
}

func (*SessionDaemon) GetInterfaceName() string {
	return dbusInterface
}

func NewSessionDaemon(logger *log.Logger) *SessionDaemon {
	daemon := &SessionDaemon{
		flags: &Flags{
			IgnoreMissingModules: _options.ignore,
			ForceStart:           _options.force,
		},
		log: logger,
	}
	err := daemon.initDsgConfig()
	if err != nil {
		logger.Warning(err)
		return nil
	}
	daemon.initModules()
	return daemon
}

func (s *SessionDaemon) initDsgConfig() error {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	s.systemSigLoop = dbusutil.NewSignalLoop(systemBus, 100)
	systemConnObj := systemBus.Object(configManagerId, "/")
	err = systemConnObj.Call(configManagerId+".acquireManager", 0, "org.deepin.dde.daemon", "org.deepin.dde.daemon.loader", "").Store(&s.configManagerPath)
	if err != nil {
		logger.Warning(err)
		return err
	}
	err = dbusutil.NewMatchRuleBuilder().Type("signal").
		PathNamespace(string(s.configManagerPath)).
		Interface("org.desktopspec.ConfigManager.Manager").
		Member("valueChanged").Build().AddTo(systemBus)
	if err != nil {
		logger.Warning(err)
		return err
	}

	s.systemSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.desktopspec.ConfigManager.Manager.valueChanged",
	}, func(sig *dbus.Signal) {
		if strings.Contains(string(sig.Name), "org.desktopspec.ConfigManager.Manager.valueChanged") &&
			strings.Contains(string(sig.Path), "org_deepin_dde_daemon_loader") && len(sig.Body) >= 1 {
			key, ok := sig.Body[0].(string)
			if ok {
				// dconfig key names must keep consistent with module names
				moduleLocker.Lock()
				defer moduleLocker.Unlock()
				module := loader.GetModule(key)
				if module == nil {
					logger.Warning("Invalid module name:", key)
					return
				}

				enable := s.getConfigValue(key)
				err := s.checkDependencies(module, enable)
				if err != nil {
					logger.Warning(err)
					return
				}
				logger.Info("valueChanged:", module.Name(), enable)
				err = module.Enable(enable)
				if err != nil {
					logger.Warningf("Enable '%s' failed: %v", key, err)
					return
				}
			}
		}
	})
	s.systemSigLoop.Start()
	return nil
}

func (s *SessionDaemon) getConfigValue(key string) bool {
	systemConn, err := dbus.SystemBus()
	if err != nil {
		logger.Warning(err)
		return false
	}
	systemConnObj := systemConn.Object("org.desktopspec.ConfigManager", s.configManagerPath)
	var val bool
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, key).Store(&val)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return val
}

func (s *SessionDaemon) register(service *dbusutil.Service) error {
	err := service.Export(dbusPath, s)
	if err != nil {
		s.log.Fatal(err)
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}
	return nil
}

func (s *SessionDaemon) initModules() {
	part1ModuleNames := []string{
		"trayicon",
		"x-event-monitor",
	}

	part2ModuleNames := []string{
		"network",
		"audio",
		"screensaver",
		"sessionwatcher",
		"power", // need screensaver and sessionwatcher
		"uadpagent",
		"service-trigger",
		"clipboard",
		"keybinding",
		"inputdevices",
		"gesture",
		"housekeeping",
		"timedate",
		"bluetooth",
		"screenedge",
		"systeminfo",
		"lastore",
		"calltrace",
		"debug",
	}

	allModules := loader.List()
	if len(part1ModuleNames)+len(part2ModuleNames) != len(allModules) {
		panic("module names len not equal")
	}

	for _, moduleName := range part1ModuleNames {
		if s.isModuleDefaultEnabled(moduleName) {
			s.part1EnabledModules = append(s.part1EnabledModules, moduleName)
		} else {
			s.part1DisabledModules = append(s.part1DisabledModules, moduleName)
		}
	}

	for _, moduleName := range part2ModuleNames {
		if s.isModuleDefaultEnabled(moduleName) {
			s.part2EnabledModules = append(s.part2EnabledModules, moduleName)
		} else {
			s.part2DisabledModules = append(s.part2DisabledModules, moduleName)
		}
	}
}

func (s *SessionDaemon) isModuleDefaultEnabled(moduleName string) bool {
	mod := loader.GetModule(moduleName)
	if mod == nil {
		panic(fmt.Errorf("not found module %q", moduleName))
	}
	return s.getConfigValue(moduleName)
}

func (s *SessionDaemon) getAllDefaultEnabledModules() []string {
	result := make([]string, len(s.part1EnabledModules)+len(s.part2EnabledModules))
	n := copy(result, s.part1EnabledModules)
	copy(result[n:], s.part2EnabledModules)
	return result
}

func (s *SessionDaemon) getAllDefaultDisabledModules() []string {
	result := make([]string, len(s.part1DisabledModules)+len(s.part2DisabledModules))
	n := copy(result, s.part1DisabledModules)
	copy(result[n:], s.part2DisabledModules)
	return result
}

func (s *SessionDaemon) execDefaultAction() {
	var err error
	if hasDDECookie {
		// start part1
		err = loader.EnableModules(s.part1EnabledModules, s.part1DisabledModules, 0)
		session.Register()

	} else {
		err = loader.EnableModules(s.getAllDefaultEnabledModules(),
			s.getAllDefaultDisabledModules(), getEnableFlag(s.flags))
	}

	if err != nil {
		fmt.Println(err)
		os.Exit(3)
	}
}

func (s *SessionDaemon) enableModules(enablingModules []string) error {
	disabledModules := filterList(s.getAllDefaultDisabledModules(), enablingModules)
	return loader.EnableModules(enablingModules, disabledModules, getEnableFlag(s.flags))
}

func (s *SessionDaemon) disableModules(disableModules []string) error {
	enablingModules := filterList(s.getAllDefaultEnabledModules(), disableModules)
	return loader.EnableModules(enablingModules, disableModules, getEnableFlag(s.flags))
}

func (s *SessionDaemon) listModule(name string) error {
	if name == "all" {
		for _, module := range loader.List() {
			fmt.Println(module.Name())
		}
		return nil
	}

	module := loader.GetModule(name)
	if module == nil {
		return fmt.Errorf("no such a module named %s", name)
	}

	fmt.Printf("module %v dependencies: %v\n", name, module.GetDependencies())
	return nil
}

func (s *SessionDaemon) CallTrace(times, seconds uint32) *dbus.Error {
	ct, err := calltrace.NewManager(seconds / times)
	if err != nil {
		logger.Warning("Failed to start calltrace:", err)
		return dbusutil.ToError(err)
	}
	ct.SetAutoDestroy(seconds)
	return nil
}

func (s *SessionDaemon) StartPart2() *dbus.Error {
	if !hasDDECookie {
		return dbusutil.ToError(errors.New("env DDE_SESSION_PROCESS_COOKIE_ID is empty"))
	}
	// start part2
	err := loader.EnableModules(s.part2EnabledModules, s.part2DisabledModules, 0)
	return dbusutil.ToError(err)
}

func filterList(origin, condition []string) []string {
	if len(condition) == 0 {
		return origin
	}

	var tmp = make(map[string]struct{})
	for _, v := range condition {
		tmp[v] = struct{}{}
	}

	var ret []string
	for _, v := range origin {
		_, ok := tmp[v]
		if ok {
			continue
		}
		ret = append(ret, v)
	}
	return ret
}
