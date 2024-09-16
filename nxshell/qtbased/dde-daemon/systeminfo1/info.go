// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/common/cpuinfo"
	"github.com/linuxdeepin/dde-daemon/loader"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	systeminfo "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.systeminfo1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
)

//go:generate dbusutil-gen em -type SystemInfo

const (
	dbusServiceName         = "org.deepin.dde.SystemInfo1"
	dbusPath                = "/org/deepin/dde/SystemInfo1"
	dbusInterface           = dbusServiceName
	dsettingsAppID          = "org.deepin.dde.daemon"
	dsettingsSystemInfoName = "org.deepin.dde.daemon.systeminfo"
	dsettingsIsM900Config   = "IsM900Config"
)

type SystemInfo struct {
	// Current deepin version, ex: "2015 Desktop"
	Version string
	// Distribution ID
	DistroID string
	// Distribution Description
	DistroDesc string
	// Distribution Version
	DistroVer string
	// CPU information
	Processor string
	// Disk capacity
	DiskCap uint64
	// Memory size
	MemoryCap uint64
	// System type: 32bit or 64bit
	SystemType int64
	// CPU max MHz
	CPUMaxMHz float64
	//Current Speed : when cpu max mhz is 0 use
	CurrentSpeed uint64
	// Cpu Hardware
	CPUHardware string
}

type Daemon struct {
	info          *SystemInfo
	PropsMu       sync.RWMutex
	systeminfo    systeminfo.SystemInfo
	sigSystemLoop *dbusutil.SignalLoop
	*loader.ModuleBase

	// 判断是否是M900配置，如果不是则设置CPUHardware为null
	isM900Config bool
}

var logger = log.NewLogger("daemon/systeminfo")

func NewDaemon(logger *log.Logger) *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("systeminfo", daemon, logger)
	return daemon
}

func (d *Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	if d.info != nil {
		return nil
	}
	service := loader.GetService()

	d.info = NewSystemInfo()
	d.isM900Config = d.getDsgIsM900Config()
	logger.Infof("the system M900 config is %t", d.isM900Config)

	d.initSysSystemInfo()
	err := service.Export(dbusPath, d.info)
	if err != nil {
		d.info = nil
		logger.Error(err)
		return err
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		d.info = nil
		logger.Error(err)
		return err
	}
	return nil
}

func (d *Daemon) Stop() error {
	if d.info == nil {
		return nil
	}

	service := loader.GetService()
	_ = service.StopExport(d.info)
	d.info = nil

	return nil
}

func (d *Daemon) initSysSystemInfo() {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return
	}
	d.systeminfo = systeminfo.NewSystemInfo(sysBus)
	d.sigSystemLoop = dbusutil.NewSignalLoop(sysBus, 10)
	d.sigSystemLoop.Start()
	d.systeminfo.InitSignalExt(d.sigSystemLoop, true)

	// 通过 demicode 获取 "CPU 频率", 接收 org.deepin.dde.SystemInfo1 的属性 CurrentSpeed 改变信号
	err = d.systeminfo.CurrentSpeed().ConnectChanged(func(hasValue bool, value uint64) {
		logger.Infof("demicode hasValue : %t, CurrentSpeed : %d", hasValue, value)
		if !hasValue {
			return
		}
		d.PropsMu.Lock()
		d.info.CurrentSpeed = value
		//假如此时 cpu max mhz 还是0, 且 value 不是0, 则给 d.infoCPUMaxMHz 再赋值
		if isFloatEqual(d.info.CPUMaxMHz, 0.0) && value != 0 {
			d.info.CPUMaxMHz = float64(value)
		}
		d.PropsMu.Unlock()
	})

	if err != nil {
		logger.Warning("systeminfo.CurrentSpeed().ConnectChanged err : ", err)
	}

	d.PropsMu.Lock()
	d.info.CurrentSpeed, err = d.systeminfo.CurrentSpeed().Get(0)
	if err != nil {
		logger.Warning("get systeminfo.CurrentSpeed err : ", err)
		d.PropsMu.Unlock()
		return
	}
	if isFloatEqual(d.info.CPUMaxMHz, 0.0) {
		d.info.CPUMaxMHz = float64(d.info.CurrentSpeed)
	}
	d.info.CPUHardware = "null"
	if d.isM900Config {
		cpuinfo, err := cpuinfo.ReadCPUInfo("/proc/cpuinfo")
		if err != nil {
			logger.Warning(err)
		} else if cpuinfo.Hardware != "" {

			d.info.CPUHardware = cpuinfo.Hardware

		}
	}

	d.PropsMu.Unlock()
	logger.Infof("CurrentSpeed: %v CPUMaxMHz: %v CPUHardware: %s", d.info.CurrentSpeed, d.info.CPUMaxMHz, d.info.CPUHardware)
}

func NewSystemInfo() *SystemInfo {
	var info SystemInfo
	tmp, _ := doReadCache(cacheFile)
	if tmp != nil && tmp.isValidity() {
		info = *tmp
		time.AfterFunc(time.Second*10, func() {
			info.init()
			_ = doSaveCache(&info, cacheFile)
		})
		return &info
	}

	info.init()
	go func() {
		_ = doSaveCache(&info, cacheFile)
	}()
	return &info
}

func (info *SystemInfo) init() {
	var err error
	info.Processor, err = GetCPUInfo("/proc/cpuinfo")
	if err != nil {
		logger.Warning("Get cpu info failed:", err)
	}

	info.Version, err = getVersion()
	if err != nil {
		logger.Warning("Get version failed:", err)
	}

	info.DistroID, info.DistroDesc, info.DistroVer, err = getDistro()
	if err != nil {
		logger.Warning("Get distribution failed:", err)
	}

	info.MemoryCap, err = getMemoryFromFile("/proc/meminfo")
	if err != nil {
		logger.Warning("Get memory capacity failed:", err)
	}

	if systemBit() == "64" {
		info.SystemType = 64
	} else {
		info.SystemType = 32
	}

	info.DiskCap, err = getDiskCap()
	if err != nil {
		logger.Warning("Get disk capacity failed:", err)
	}

	lscpuRes, err := runLscpu()
	if err != nil {
		logger.Warning("run lscpu failed:", err)
		return
	} else {
		info.CPUMaxMHz, err = getCPUMaxMHzByLscpu(lscpuRes)
		if err != nil {
			logger.Warning(err)
		} else {
			if isFloatEqual(info.CPUMaxMHz, 0.0) {
				//关联信号,接收system的信号 : line139
				//此时若info.CurrentSpeed不为0, 则可以直接使用备用的currentspeed赋值
				if info.CurrentSpeed != 0 {
					info.CPUMaxMHz = float64(info.CurrentSpeed)
				}
			}
		}

		// 适配兆芯KX-7000
		if lscpuRes[lscpuKeyCPUfamily] == "7" &&
			lscpuRes[lscpuKeyModel] == "107" &&
			lscpuRes[lscpuKeyStepping] == "1" {
			info.Processor, err = getProcessorByLscpuExt(lscpuRes, info.CPUMaxMHz/1000)
			if err != nil {
				logger.Warning(err)
			}
		}

		if info.Processor == "" {
			info.Processor, err = getProcessorByLscpu(lscpuRes)
			if err != nil {
				logger.Warning(err)
			}
		}
	}
}

func (info *SystemInfo) isValidity() bool {
	if info.Processor == "" || info.DiskCap == 0 || info.MemoryCap == 0 {
		return false
	}
	return true
}

func (*SystemInfo) GetInterfaceName() string {
	return dbusInterface
}

func (d *Daemon) getDsgIsM900Config() bool {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning(err)
		return false
	}
	ds := ConfigManager.NewConfigManager(sysBus)
	dsSystemInfoPath, err := ds.AcquireManager(0, dsettingsAppID, dsettingsSystemInfoName, "")
	if err != nil {
		logger.Warning(err)
		return false
	}
	dsSystemInfo, err := ConfigManager.NewManager(sysBus, dsSystemInfoPath)
	if err != nil {
		logger.Warning(err)
		return false
	}

	data, err := dsSystemInfo.Value(0, dsettingsIsM900Config)
	if err != nil {
		logger.Warning(err)
		return false
	}

	if isM900Config, ok := data.Value().(bool); ok {
		return isM900Config
	}

	return false
}
