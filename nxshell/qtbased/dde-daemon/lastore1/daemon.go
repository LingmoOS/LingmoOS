// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package lastore

import (
	"encoding/json"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/loader"
	ConfigManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	notifications "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.dde.notification"
	abrecovery "github.com/linuxdeepin/go-dbus-factory/system/com.deepin.abrecovery"
	lastore "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.lastore1"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/log"
)

const (
	dbusPath        = "/org/deepin/dde/LastoreSessionHelper1"
	dbusServiceName = "org.deepin.dde.LastoreSessionHelper1"
)

const (
	dSettingsAppID                  = "org.deepin.lastore"
	dSettingsLastoreName            = "org.deepin.lastore"
	dSettingsKeyUpgradeStatus       = "upgrade-status"
	dSettingsKeyLastoreDaemonStatus = "lastore-daemon-status"
)

const (
	UpgradeReady   = "ready"
	UpgradeRunning = "running"
	UpgradeFailed  = "failed"

	runningUpgradeBackend = 1 << 2 // 是否处于更新状态

)

type UpgradeReasonType string

const (
	NoError                      UpgradeReasonType = "NoError"
	ErrorUnknown                 UpgradeReasonType = "ErrorUnknown"
	ErrorFetchFailed             UpgradeReasonType = "fetchFailed"
	ErrorDpkgError               UpgradeReasonType = "dpkgError"
	ErrorPkgNotFound             UpgradeReasonType = "pkgNotFound"
	ErrorUnmetDependencies       UpgradeReasonType = "unmetDependencies"
	ErrorNoInstallationCandidate UpgradeReasonType = "noInstallationCandidate"
	ErrorInsufficientSpace       UpgradeReasonType = "insufficientSpace"
	ErrorUnauthenticatedPackages UpgradeReasonType = "unauthenticatedPackages"
	ErrorOperationNotPermitted   UpgradeReasonType = "operationNotPermitted"
	ErrorIndexDownloadFailed     UpgradeReasonType = "IndexDownloadFailed"
	ErrorIO                      UpgradeReasonType = "ioError"
	ErrorDamagePackage           UpgradeReasonType = "damagePackage" // 包损坏,需要删除后重新下载或者安装
)

const (
	NotifyExpireTimeoutDefault = -1
	NotifyExpireTimeoutNoHide  = 0
)

const distUpgradeJobPath = "/com/deepin/lastore/Jobdist_upgrade"

var logger = log.NewLogger("daemon/lastore")

func init() {
	loader.Register(newDaemon())
}

type Daemon struct {
	lastore *Lastore
	agent   *Agent
	*loader.ModuleBase
}

func newDaemon() *Daemon {
	daemon := new(Daemon)
	daemon.ModuleBase = loader.NewModuleBase("lastore", daemon, logger)
	return daemon
}

func (*Daemon) GetDependencies() []string {
	return []string{}
}

func (d *Daemon) Start() error {
	var lastoreOnce sync.Once
	service := loader.GetService()
	sysBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning(err)
		return err
	}
	sysDBusDaemon := ofdbus.NewDBus(sysBus)
	systemSigLoop := dbusutil.NewSignalLoop(sysBus, 10)
	systemSigLoop.Start()
	initLastore := func() error {
		lastoreObj, err := newLastore(service)
		if err != nil {
			logger.Warning(err)
			return err
		}
		d.lastore = lastoreObj
		err = service.Export(dbusPath, lastoreObj, lastoreObj.syncConfig)
		if err != nil {
			logger.Warning(err)
			return err
		}

		err = service.RequestName(dbusServiceName)
		if err != nil {
			logger.Warning(err)
			return err
		}
		err = lastoreObj.syncConfig.Register()
		if err != nil {
			logger.Warning("Failed to register sync service:", err)
		}
		defer func() {
			sysDBusDaemon.RemoveAllHandlers()
			systemSigLoop.Stop()
		}()
		agent, err := newAgent(lastoreObj)
		if err != nil {
			logger.Warning(err)
			return err
		}
		d.agent = agent
		return agent.init()

	}
	core := lastore.NewLastore(sysBus)
	// 处理更新失败/中断的记录
	go func() {
		// 获取lastore-daemon记录的更新状态
		ds := ConfigManager.NewConfigManager(sysBus)
		dsPath, err := ds.AcquireManager(0, dSettingsAppID, dSettingsLastoreName, "")
		if err != nil {
			logger.Warning(err)
			return
		}
		dsLastoreManager, err := ConfigManager.NewManager(sysBus, dsPath)
		if err != nil {
			logger.Warning(err)
			return
		}
		v, err := dsLastoreManager.Value(0, dSettingsKeyUpgradeStatus)
		if err != nil {
			logger.Warning(err)
			return
		} else {
			jobList, _ := core.Manager().JobList().Get(0)
			// 如果正常系统更新,那么不需要处理(更新中切换用户场景)
			for _, path := range jobList {
				if path == distUpgradeJobPath {
					logger.Info("running dist-upgrade,don't need handle status")
					return
				}
			}
			upgradeStatus := struct {
				Status     string
				ReasonCode UpgradeReasonType
			}{}
			statusContent := v.Value().(string)
			err = json.Unmarshal([]byte(statusContent), &upgradeStatus)
			if err != nil {
				logger.Warning(err)
				return
			}
			logger.Infof("lastore status:%+v", upgradeStatus)
			// 记录处理异常更新的通知
			osd := notifications.NewNotification(service.Conn())
			abObj := abrecovery.NewABRecovery(sysBus)
			valid, err := abObj.ConfigValid().Get(0) // config失效时,无法回滚,提示重新更新
			var msg string
			switch upgradeStatus.Status {
			// running状态,更新被中断
			case UpgradeRunning:
				if err != nil || !valid {
					msg = gettext.Tr("Updates failed: it was interrupted.")
				} else {
					msg = gettext.Tr("Updates failed: it was interrupted. Please roll back to the old version and try again.")
				}
			case UpgradeFailed:
				switch upgradeStatus.ReasonCode {
				case ErrorDpkgError:
					if err != nil || !valid {
						msg = gettext.Tr("Updates failed: DPKG error.")
					} else {
						msg = gettext.Tr("Updates failed: DPKG error. Please roll back to the old version and try again.")
					}
				case ErrorInsufficientSpace:
					if err != nil || !valid {
						msg = gettext.Tr("Updates failed: insufficient disk space.")
					} else {
						msg = gettext.Tr("Updates failed: insufficient disk space. Please roll back to the old version and try again.")
					}
				case ErrorUnknown, ErrorPkgNotFound, ErrorNoInstallationCandidate, ErrorIO, ErrorDamagePackage:
					if err != nil || !valid {
						msg = gettext.Tr("Updates failed")
					} else {
						msg = gettext.Tr("Updates failed. Please roll back to the old version and try again.")
					}
				}
			}
			logger.Infof("notify msg is:%v", msg)
			if len(msg) != 0 {
				_, err = osd.Notify(0, "dde-control-center", 0, "preferences-system", "", msg, nil, nil, NotifyExpireTimeoutNoHide)
				if err != nil {
					logger.Warning(err)
				}
			}
			// 通知发完之后,恢复配置文件的内容
			upgradeStatus.Status = UpgradeReady
			upgradeStatus.ReasonCode = NoError
			v, err := json.Marshal(upgradeStatus)
			if err != nil {
				logger.Warning(err)
			} else {
				err = dsLastoreManager.SetValue(0, dSettingsKeyUpgradeStatus, dbus.MakeVariant(v))
				if err != nil {
					logger.Warning(err)
				}
			}
		}
	}()
	time.AfterFunc(10*time.Minute, func() {
		lastoreOnce.Do(func() {
			err := initLastore()
			if err != nil {
				logger.Warning(err)
			}
		})
	})

	sysDBusDaemon.InitSignalExt(systemSigLoop, true)
	_, err = sysDBusDaemon.ConnectNameOwnerChanged(func(name, oldOwner, newOwner string) {
		if name == core.ServiceName_() && newOwner != "" {
			lastoreOnce.Do(func() {
				err := initLastore()
				if err != nil {
					logger.Warning(err)
				}
			})
		}
	})
	if err != nil {
		logger.Warning(err)
	}
	hasOwner, err := sysDBusDaemon.NameHasOwner(0, core.ServiceName_())
	if err != nil {
		logger.Warning(err)
	} else if hasOwner {
		lastoreOnce.Do(func() {
			err := initLastore()
			if err != nil {
				logger.Warning(err)
			}
		})
	}
	return nil
}

func (d *Daemon) Stop() error {
	if d.lastore != nil {
		service := loader.GetService()
		err := service.ReleaseName(dbusServiceName)
		if err != nil {
			logger.Warning(err)
		}
		d.lastore.destroy()
		err = service.StopExport(d.lastore)
		if err != nil {
			logger.Warning(err)
		}

		d.lastore = nil
	}

	if d.agent != nil {
		service := loader.GetService()
		d.agent.destroy()
		err := service.StopExport(d.agent)
		if err != nil {
			logger.Warning(err)
		}
		d.agent = nil
	}
	return nil
}
