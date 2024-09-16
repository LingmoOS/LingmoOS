// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"net/http"
	"os"
	"os/exec"
	"sync"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/common/dsync"
	"github.com/linuxdeepin/dde-daemon/network1/nm"
	"github.com/linuxdeepin/dde-daemon/network1/proxychains"
	"github.com/linuxdeepin/dde-daemon/session/common"
	configManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	sessionmanager "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.sessionmanager1"
	secrets "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.secrets"
	airplanemode "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.airplanemode1"
	ipwatchd "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.ipwatchd1"
	sysNetwork "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.network1"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	nmdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.networkmanager"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
	"github.com/linuxdeepin/go-lib/keyfile"
	"github.com/linuxdeepin/go-lib/strv"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const (
	dbusServiceName = "org.deepin.dde.Network1"
	dbusPath        = "/org/deepin/dde/Network1"
	dbusInterface   = dbusServiceName
)

const (
	daemonConfigPath                   = "org.deepin.dde.daemon"
	networkConfigPath                  = "org.deepin.dde.daemon.network"
	dsettingsProtalAuthEnable          = "protalAuthEnable"
	dsettingsResetWifiOSDEnableTimeout = "resetWifiOSDEnableTimeout"
	dsettingsDisableFailureNotify      = "disableFailureNotify"

	networkCoreDsgConfigPath    = "/usr/share/dsg/configs/org.deepin.dde.network/org.deepin.dde.network.json"
	networkCoreConfigPath       = "org.deepin.dde.network"
	ddeNetworkCoreConfigPath    = networkCoreConfigPath
	dsettingsLoadServiceFromNM  = "LoadServiceFromNM"
	dsettingsEnableConnectivity = "enableConnectivity"
)

const checkRepeatTime = 1 * time.Second

type connectionData map[string]map[string]dbus.Variant

var globalSessionActive bool

//go:generate dbusutil-gen em -type Manager,SecretAgent

// Manager is the main DBus object for network module.
type Manager struct {
	sysSigLoop         *dbusutil.SignalLoop
	service            *dbusutil.Service
	sysNetwork         sysNetwork.Network
	airplane           airplanemode.AirplaneMode
	sysIPWatchD        ipwatchd.IPWatchD
	nmObjManager       nmdbus.ObjectManager
	PropsMu            sync.RWMutex
	sessionManager     sessionmanager.SessionManager
	currentSessionPath dbus.ObjectPath
	currentSession     login1.Session

	// update by manager.go
	State            uint32 // global networking state
	connectivityLock sync.Mutex
	Connectivity     uint32

	NetworkingEnabled bool `prop:"access:rw"` // airplane mode for NetworkManager
	VpnEnabled        bool `prop:"access:rw"`

	// hidden properties
	wirelessEnabled bool
	wwanEnabled     bool
	wiredEnabled    bool

	delayEnableVpn bool
	delayVpnLock   sync.Mutex

	// update by manager_devices.go
	devicesLock sync.Mutex
	devices     map[string][]*device
	Devices     string // array of device objects and marshaled by json

	accessPointsLock sync.Mutex
	accessPoints     map[dbus.ObjectPath][]*accessPoint

	// update by manager_connections.go
	connectionsLock sync.Mutex
	connections     map[string]connectionSlice
	Connections     string // array of connection information and marshaled by json

	// update by manager_active.go
	activeConnectionsLock sync.Mutex
	activeConnections     map[dbus.ObjectPath]*activeConnection
	ActiveConnections     string // array of connections that activated and marshaled by json

	secretAgent        *SecretAgent
	stateHandler       *stateHandler
	proxyChainsManager *proxychains.Manager

	sessionSigLoop *dbusutil.SignalLoop
	syncConfig     *dsync.Config

	portalLastDetectionTime time.Time

	WirelessAccessPoints    string `prop:"access:r"` //用于读取AP
	debugChangeAPBand       string //调用接口切换ap频段
	checkAPStrengthTimer    *time.Timer
	protalAuthBrowserOpened bool // PORTAL认证中状态

	acinfosJSON string

	// to identify if vpn support multi connections
	multiVpn map[string]bool

	connectionSettingsLock sync.Mutex

	// dsg config : org.deepin.dde.daemon.network
	protalAuthEnable          bool
	wifiOSDEnable             bool
	disableFailureNotify      bool
	resetWifiOSDEnableTimeout uint32
	resetWifiOSDEnableTimer   *time.Timer
	delayShowWifiOSD          *time.Timer

	// dsg config : org.deepin.dde.network : LoadServiceFromNM
	loadServiceFromNM       bool
	enableLocalConnectivity bool

	//nolint
	signals *struct {
		AccessPointAdded, AccessPointRemoved, AccessPointPropertiesChanged struct {
			devPath, apJSON string
		}
		DeviceEnabled struct {
			devPath string
			enabled bool
		}
		ActiveConnectionInfoChanged struct {
		}
		IPConflict struct {
			ip  string
			mac string
		}
		ProxyMethodChanged struct {
			method string
		}
	}
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

// initialize slice code manually to make i18n works
func initSlices() {
	initProxyGsettings()
	initNmStateReasons()
}

func NewManager(service *dbusutil.Service) (m *Manager) {
	m = &Manager{
		service: service,
	}

	return
}

func (m *Manager) init() {
	logger.Info("initialize network")

	systemBus, err := dbus.SystemBus()
	if err != nil {
		return
	}

	m.multiVpn = make(map[string]bool)

	sessionBus := m.service.Conn()
	m.sessionSigLoop = dbusutil.NewSignalLoop(sessionBus, 10)
	m.sessionSigLoop.Start()

	m.sysSigLoop = sysSigLoop
	m.initDbusObjects()

	disableNotify()
	defer enableNotify()

	m.sessionManager = sessionmanager.NewSessionManager(sessionBus)
	m.currentSessionPath, err = m.sessionManager.CurrentSessionPath().Get(0)
	if err != nil {
		logger.Warning("get sessionManager CurrentSessionPath failed:", err)
	}
	m.currentSession, err = login1.NewSession(systemBus, m.currentSessionPath)
	if err != nil {
		logger.Error("Failed to connect self session:", err)
		return
	}

	sysService, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Warning(err)
		return
	}

	// TODO(jouyouyun): improve in future
	// Sometimes the 'org.freedesktop.secrets' is not exists, this would block the 'init' function, so move to goroutine
	go func() {
		secServiceObj := secrets.NewService(sessionBus)
		sa, err := newSecretAgent(secServiceObj, m)
		if err != nil {
			logger.Warning(err)
			return
		}
		m.secretAgent = sa

		logger.Debug("unique name on system bus:", systemBus.Names()[0])
		err = sysService.Export("/org/freedesktop/NetworkManager/SecretAgent", sa)
		if err != nil {
			logger.Warning(err)
			return
		}

		// register secret agent
		nmAgentManager := nmdbus.NewAgentManager(systemBus)
		err = nmAgentManager.Register(0, "org.deepin.dde.Network1.SecretAgent")
		if err != nil {
			logger.Debug("failed to register secret agent:", err)
		} else {
			logger.Debug("register secret agent ok")
		}
	}()

	// 初始化配置
	m.resetWifiOSDEnableTimeout = 300
	ds := configManager.NewConfigManager(m.sysSigLoop.Conn())
	configManagerPath, err := ds.AcquireManager(0, daemonConfigPath, networkConfigPath, "")
	if err == nil {
		networkConfigManager, err := configManager.NewManager(m.sysSigLoop.Conn(), configManagerPath)
		if err == nil {
			getProtalAuthEnable := func() {
				v, err := networkConfigManager.Value(0, dsettingsProtalAuthEnable)
				if err != nil {
					logger.Warning(err)
					return
				}
				m.protalAuthEnable = v.Value().(bool)
			}

			getDisableFailureNotify := func() {
				v, err := networkConfigManager.Value(0, dsettingsDisableFailureNotify)
				if err != nil {
					logger.Warning(err)
					return
				}
				m.disableFailureNotify = v.Value().(bool)
			}

			getResetWifiOSDEnableTimeout := func() {
				v, err := networkConfigManager.Value(0, dsettingsResetWifiOSDEnableTimeout)
				if err != nil {
					logger.Warning(err)
					return
				}
				switch vv := v.Value().(type) {
				case float64:
					m.resetWifiOSDEnableTimeout = uint32(vv)
				case int64:
					m.resetWifiOSDEnableTimeout = uint32(vv)
				default:
					logger.Warning("type is wrong!")
				}
			}

			getProtalAuthEnable()
			getResetWifiOSDEnableTimeout()
			getDisableFailureNotify()

			networkConfigManager.InitSignalExt(m.sysSigLoop, true)
			_, err = networkConfigManager.ConnectValueChanged(func(key string) {
				if key == dsettingsProtalAuthEnable {
					getProtalAuthEnable()
				} else if key == dsettingsResetWifiOSDEnableTimeout {
					getResetWifiOSDEnableTimeout()
				} else if key == dsettingsDisableFailureNotify {
					getDisableFailureNotify()
				}
			})
			if err != nil {
				logger.Warning(err)
			}
		} else {
			logger.Warning(err)
		}
	} else {
		logger.Warning(err)
	}

	m.loadServiceFromNM = m.getLoadServiceFromNM(ds)
	logger.Info("[init], DConfig data of LoadServiceFromNM : ", m.loadServiceFromNM)
	m.loadEnableConnectivity(ds)
	// 初始化配置
	m.wifiOSDEnable = true
	m.resetWifiOSDEnableTimer = time.AfterFunc(time.Duration(m.resetWifiOSDEnableTimeout)*time.Millisecond, func() {
		logger.Debug("reset wifi OSD enable")
		m.wifiOSDEnable = true
	})
	m.resetWifiOSDEnableTimer.Stop()

	globalSessionActive = m.isSessionActive()
	logger.Debugf("current session activated state: %v", globalSessionActive)

	// initialize device and connection handlers
	m.sysNetwork = sysNetwork.NewNetwork(systemBus)
	m.airplane = airplanemode.NewAirplaneMode(systemBus)
	m.loadMultiVpn()
	m.initConnectionManage()
	m.initDeviceManage()
	m.initActiveConnectionManage()
	m.initNMObjManager(systemBus)
	m.stateHandler = newStateHandler(m.sysSigLoop, m)
	m.initSysNetwork(systemBus)
	m.initIPConflictManager(systemBus)

	// monitor enable state
	m.airplane.InitSignalExt(m.sysSigLoop, true)

	// airplane osd
	err = m.airplane.Enabled().ConnectChanged(func(hasValue bool, value bool) {
		// has value
		if !hasValue {
			return
		}

		// 显示飞行模式OSD时不显示WIFI连接OSD,200毫秒后恢复显示WIFI的OSD
		m.wifiOSDEnable = false
		if m.delayShowWifiOSD != nil {
			m.delayShowWifiOSD.Stop()
		}
		m.resetWifiOSDEnableTimer.Stop()
		m.resetWifiOSDEnableTimer.Reset(time.Duration(m.resetWifiOSDEnableTimeout) * time.Millisecond)

		// if enabled is true, airplane is on
		if value {
			showOSD("AirplaneModeOn")
			// if enabled is false, airplane is off
		} else {
			showOSD("AirplaneModeOff")
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	// wlan osd
	err = m.airplane.WifiEnabled().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}

		// 停止上次的定时器
		if m.delayShowWifiOSD != nil {
			m.delayShowWifiOSD.Stop()
		}

		// 如果刚刚显示了飞行模式的OSD则直接退出不显示WIFI的OSD
		if !m.wifiOSDEnable {
			return
		}

		// 等待150毫秒接收airplane.Enabled改变信号
		m.delayShowWifiOSD = time.AfterFunc(time.Duration(m.resetWifiOSDEnableTimeout-50)*time.Millisecond, func() {
			// 禁用WIFI网络OSD时退出
			if !m.wifiOSDEnable {
				return
			}

			// if enabled is true, wifi rfkill block is true
			// so wlan is off
			if value {
				showOSD("WLANOff")
				// if enabled is false, wifi is off
			} else {
				showOSD("WLANOn")
			}
		})
	})
	if err != nil {
		logger.Warning(err)
	}

	// update property "State"
	err = nmManager.PropState().ConnectChanged(func(hasValue bool, value uint32) {
		m.updatePropState()
		// get network state
		avail, err := isNetworkAvailable()
		if err != nil {
			logger.Warningf("get network state failed, err: %v", err)
			return
		}
		// check network state
		if !avail {
			return
		}
		// check if current pri
		typ, err := nmManager.PrimaryConnectionType().Get(0)
		if err != nil {
			logger.Warningf("get primary type failed, err: %v", err)
			return
		}
		// check if primary type is already vpn
		if typ == nm.NM_SETTING_VPN_SETTING_NAME {
			logger.Debug("current primary typ is already vpn, dont need to reactive once")
			return
		}
		logger.Debugf("current primary typ is %v, prop changed: %v need to reactive vpn", typ, value)
		// get delay vpn state
		delay := m.getDelayEnableVpn()
		// if vpn enable is true, but network disconnect last time, try to auto connect vpn.
		// delay is marked as true when trying to enable vpn state but network cant be available,
		// so need to retry enable vpn and try to auto connect vpn.
		if !delay && !m.VpnEnabled {
			return
		}
		m.setVpnEnable(true)
	})
	if err != nil {
		logger.Warning(err)
	}
	m.updatePropState()

	// update property Connectivity
	_ = nmManager.Connectivity().ConnectChanged(func(hasValue bool, value uint32) {
		logger.Debug("connectivity state changed ", hasValue, value)
		if hasValue && value == nm.NM_CONNECTIVITY_PORTAL && m.protalAuthEnable && !m.enableLocalConnectivity {
			go m.doPortalAuthentication()
		}
		m.setPropConnectivity(value)
	})
	// get connectivity
	connectivity, err := nmManager.Connectivity().Get(0)
	if err != nil {
		logger.Warningf("get connectivity failed, err: %v", err)
	}
	m.setPropConnectivity(connectivity)
	go func() {
		time.Sleep(3 * time.Second)
		m.checkConnectivity()
	}()

	// 调整nmDev的状态
	m.adjustDeviceStatus()
	// move to power module
	// connect computer suspend signal
	// _, err = loginManager.ConnectPrepareForSleep(func(active bool) {
	// 	if active {
	// 		// suspend
	// 		disableNotify()
	// 	} else {
	// 		// restore
	// 		enableNotify()

	// 		_ = m.RequestWirelessScan()
	// 	}
	// })
	// if err != nil {
	// 	logger.Warning(err)
	// }

	m.syncConfig = dsync.NewConfig("network", &syncConfig{m: m},
		m.sessionSigLoop, dbusPath, logger)
}

func (m *Manager) loadEnableConnectivity(ds configManager.ConfigManager) {
	networkCoreConfigManagerPath, err := ds.AcquireManager(0, networkCoreConfigPath, ddeNetworkCoreConfigPath, "")
	if err != nil {
		logger.Warning(err)
		return
	}

	networkCoreConfigManager, err := configManager.NewManager(m.sysSigLoop.Conn(), networkCoreConfigManagerPath)
	if err != nil {
		logger.Warning(err)
		return
	}

	getDEnableLocalConnectivity := func() bool {
		v, err := networkCoreConfigManager.Value(0, dsettingsEnableConnectivity)
		if err != nil {
			logger.Warning(err)
			return false
		}
		return v.Value().(bool)
	}

	m.enableLocalConnectivity = getDEnableLocalConnectivity()
	logger.Info("DConfig data of enableConnectivity : ", m.enableLocalConnectivity)
	networkCoreConfigManager.InitSignalExt(m.sysSigLoop, true)
	_, err = networkCoreConfigManager.ConnectValueChanged(func(key string) {
		if key == dsettingsEnableConnectivity {
			m.enableLocalConnectivity = getDEnableLocalConnectivity()
			logger.Info("DConfig data changed of enableConnectivity : ", m.enableLocalConnectivity)
		}
	})

	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) getLoadServiceFromNM(ds configManager.ConfigManager) (ret bool) {
	if dutils.IsFileExist(networkCoreDsgConfigPath) {
		configManagerPath, err := ds.AcquireManager(0, networkCoreConfigPath, ddeNetworkCoreConfigPath, "")
		if err != nil {
			logger.Warning(err)
			return
		}
		ddeNetworkCoreConfigManager, err := configManager.NewManager(m.sysSigLoop.Conn(), configManagerPath)
		if err != nil {
			logger.Warning(err)
			return
		}
		getDSettingsLoadServiceFromNM := func() bool {
			v, err := ddeNetworkCoreConfigManager.Value(0, dsettingsLoadServiceFromNM)
			if err != nil {
				logger.Warning(err)
				return false
			}
			return v.Value().(bool)
		}

		ret = getDSettingsLoadServiceFromNM()
	} else {
		logger.Warning("[init] DConfig file not exist : /usr/share/dsg/configs/org.deepin.dde.network/org.deepin.dde.network.json")
	}

	return
}

func (m *Manager) destroy() {
	logger.Info("destroy network")
	m.multiVpn = nil
	m.sessionSigLoop.Stop()
	m.syncConfig.Destroy()
	m.nmObjManager.RemoveHandler(proxy.RemoveAllHandlers)
	m.sysNetwork.RemoveHandler(proxy.RemoveAllHandlers)
	destroyDbusObjects()
	destroyStateHandler(m.stateHandler)
	m.clearDevices()
	m.clearAccessPoints()
	m.clearConnections()
	m.clearActiveConnections()

	// reset dbus properties
	m.setPropNetworkingEnabled(false)
	m.updatePropState()

	if m.checkAPStrengthTimer != nil {
		m.checkAPStrengthTimer.Stop()
		m.checkAPStrengthTimer = nil
	}
}

func watchNetworkManagerRestart(m *Manager) {
	_, err := dbusDaemon.ConnectNameOwnerChanged(func(name, oldOwner, newOwner string) {
		if name == "org.freedesktop.NetworkManager" {
			// if a new dbus session was installed, the name and newOwner
			// will be no empty, if a dbus session was uninstalled, the
			// name and oldOwner will be not empty
			if len(newOwner) != 0 {
				// network-manager is starting
				logger.Info("network-manager is starting")
				time.Sleep(1 * time.Second)
				m.init()
			} else {
				// network-manager stopped
				logger.Info("network-manager stopped")
				m.destroy()
			}
		}
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) initSysNetwork(sysBus *dbus.Conn) {
	m.sysNetwork.InitSignalExt(m.sysSigLoop, true)
	err := common.ActivateSysDaemonService(m.sysNetwork.ServiceName_())
	if err != nil {
		logger.Warning(err)
	}

	_, err = m.sysNetwork.ConnectDeviceEnabled(func(devPath dbus.ObjectPath, enabled bool) {
		err := m.service.Emit(manager, "DeviceEnabled", string(devPath), enabled)
		if err != nil {
			logger.Warning(err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	vpnEnabled, err := m.sysNetwork.VpnEnabled().Get(0)
	if err != nil {
		logger.Warning(err)
	} else {
		// set vpn enable
		m.setVpnEnable(vpnEnabled)
	}
	err = m.sysNetwork.VpnEnabled().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}

		m.PropsMu.Lock()
		m.setPropVpnEnabled(value)
		m.PropsMu.Unlock()
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) initNMObjManager(systemBus *dbus.Conn) {
	objManager := nmdbus.NewObjectManager(systemBus)
	m.nmObjManager = objManager
	objManager.InitSignalExt(m.sysSigLoop, true)
	_, err := objManager.ConnectInterfacesAdded(func(objectPath dbus.ObjectPath,
		interfacesAndProperties map[string]map[string]dbus.Variant) {
		_, ok := interfacesAndProperties["org.freedesktop.NetworkManager.Connection.Active"]
		if ok {
			// add active connection
			m.activeConnectionsLock.Lock()
			defer m.activeConnectionsLock.Unlock()

			logger.Debug("add active connection", objectPath)
			aConn := m.newActiveConnection(objectPath)
			m.activeConnections[objectPath] = aConn
			m.updatePropActiveConnections()
		}
	})
	if err != nil {
		logger.Warning(err)
	}
	_, err = objManager.ConnectInterfacesRemoved(func(objectPath dbus.ObjectPath, interfaces []string) {
		if strv.Strv(interfaces).Contains("org.freedesktop.NetworkManager.Connection.Active") {
			// remove active connection
			m.activeConnectionsLock.Lock()
			defer m.activeConnectionsLock.Unlock()

			logger.Debug("remove active connection", objectPath)
			delete(m.activeConnections, objectPath)
			m.updatePropActiveConnections()
		}
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) doPortalAuthentication() {
	err := exec.Command("pgrep", "startdde").Run()
	if err != nil {
		return
	}

	sincePortalDetection := time.Since(m.portalLastDetectionTime)
	// 处于认证中状态无需再次打开认证窗口
	if sincePortalDetection < checkRepeatTime || m.protalAuthBrowserOpened {
		return
	}

	// http client to get url
	client := &http.Client{
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return http.ErrUseLastResponse
		},
	}
	// get url
	detectUrl := "http://detectportal.deepin.com"
	res, err := client.Get(detectUrl)
	if err != nil {
		logger.Warningf("get remote http failed ,err: %v", err)
		return
	}
	// get portal addr from response
	portal, err := getRedirectFromResponse(res, detectUrl)
	if err != nil {
		logger.Warningf("get redirect hosts failed, err: %v", err)
		return
	}
	logger.Debugf("portal addr is %v", portal)
	err = exec.Command(`xdg-open`, portal).Run()
	if err != nil {
		logger.Warningf("xdg open windows failed, err: %v", err)
		return
	}
	m.portalLastDetectionTime = time.Now()
	m.protalAuthBrowserOpened = true
}

// auto connect vpn
func (m *Manager) autoConnectVpn() {
	// get vpn list from NetworkManager/Settings
	uuidList, err := getAutoConnectConnUuidListByConnType("vpn")
	if err != nil {
		logger.Warningf("get vpn conn uuid list failed, err: %v", err)
		return
	}
	logger.Debugf("all auto connect vpn is %v", uuidList)
	// auto connect vpn list
	for _, uuid := range uuidList {
		_, err := m.activateConnection(uuid, "/")
		if err != nil {
			logger.Warningf("activate connection vpn failed, err: %v", err)
		}
	}
}

// set vpn enable
func (m *Manager) setVpnEnable(vpnEnabled bool) {
	// if vpn enable is true, check if network is available.
	if vpnEnabled {
		// get network available state
		avail, err := isNetworkAvailable()
		if err != nil {
			logger.Warning(err)
			return
		}
		// check if network is available
		if avail {
			logger.Debug("network available is true")
			// if network available is true and enable is true,
			// set vpn enable and emit signal immediately.
			m.setPropVpnEnabled(true)
			// reset delay vpn enable
			m.setDelayEnableVpn(false)
			// auto connect vpn
			m.autoConnectVpn()
		} else {
			logger.Debug("network available is false")
			// mark delayEnableVpn as true
			m.setDelayEnableVpn(true)
		}
	} else {
		logger.Debug("set vpn enable false")
		// reset delay enable vpn as false
		m.setDelayEnableVpn(false)
	}
}

// load if vpn support multi connections
func (m *Manager) loadMultiVpn() {
	// all vpn plugins dir
	pathSl := []string{os.Getenv("NM_VPN_PLUGIN_DIR"), "/usr/lib/NetworkManager/VPN", "/etc/NetworkManager/VPN"}

	// read file
	kf := keyfile.NewKeyFile()
	for _, path := range pathSl {
		// dont care about read error
		if err := kf.LoadFromFile(path); err != nil {
			continue
		}
		// get service name, service must exist
		service, err := kf.GetString("VPN Connection", "service")
		if err != nil {
			logger.Warningf("cant read service from file %s, err: %v", path, err)
			continue
		}
		// if service exist already, should ignore
		if _, ok := m.multiVpn[service]; ok {
			continue
		}
		// get if support vpn multi connections, key may not exist
		exist, err := kf.GetBool("VPN Connection", "supports-multiple-connections")
		if err != nil {
			continue
		}
		// store
		m.multiVpn[service] = exist
	}
}

// set delay enable vpn
func (m *Manager) setDelayEnableVpn(enable bool) {
	m.delayVpnLock.Lock()
	m.delayEnableVpn = enable
	m.delayVpnLock.Unlock()
}

// get delay enable vpn
func (m *Manager) getDelayEnableVpn() bool {
	m.delayVpnLock.Lock()
	enable := m.delayEnableVpn
	m.delayVpnLock.Unlock()
	return enable
}

// checkConnectivity This function may block for a long time，
// is recommended for use in Goroutine
func (m *Manager) checkConnectivity() {
	connectivity, err := nmManager.CheckConnectivity(0)
	if err != nil {
		logger.Warning(err)
		return
	}
	if connectivity == nm.NM_CONNECTIVITY_PORTAL && m.protalAuthEnable && !m.enableLocalConnectivity {
		m.doPortalAuthentication()
	}
}
