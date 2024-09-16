// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"errors"
	"fmt"
	"time"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/network1/nm"
	nmdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.networkmanager"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/utils"
)

type apSecType uint32

const (
	apSecNone apSecType = iota
	apSecWep
	apSecPsk
	apSecEap
	apSecSae
)
const scanWifiDelayTime = 10 * time.Second
const channelAutoChangeThreshold = 65

// frequency range
const (
	frequency5GUpperlimit = 5825
	frequency5GLowerlimit = 4915
	frequency2GUpperlimit = 2484
	frequency2GLowerlimit = 2412
)

func (v apSecType) String() string {
	switch v {
	case apSecNone:
		return "none"
	case apSecWep:
		return "wep"
	case apSecPsk:
		return "wpa-psk"
	case apSecSae:
		return "sae"
	case apSecEap:
		return "wpa-eap"
	default:
		return fmt.Sprintf("<invalid apSecType %d>", v)
	}
}

type accessPoint struct {
	nmAp    nmdbus.AccessPoint
	devPath dbus.ObjectPath

	Ssid         string
	Secured      bool
	SecuredInEap bool
	Strength     uint8
	Path         dbus.ObjectPath
	Frequency    uint32
	// add hidden property
	Hidden  bool
	Flags   uint32
	KeyMgmt string // 直接表明推荐的 keymgmt，不要让前后端两套逻辑
}

func (m *Manager) newAccessPoint(devPath, apPath dbus.ObjectPath) (ap *accessPoint, err error) {
	nmAp, err := nmNewAccessPoint(apPath)
	if err != nil {
		return
	}

	ap = &accessPoint{
		nmAp:    nmAp,
		devPath: devPath,
		Path:    apPath,
	}
	ap.updateProps()
	if len(ap.Ssid) == 0 {
		err = fmt.Errorf("ignore hidden access point")
		return
	}

	// add hidden
	if m.isHidden(ap.Ssid) {
		ap.Hidden = true
	}

	// connect property changed signals
	ap.nmAp.InitSignalExt(m.sysSigLoop, true)
	_, err = ap.nmAp.ConnectSignalPropertiesChanged(func(properties map[string]dbus.Variant) {
		m.accessPointsLock.Lock()
		defer m.accessPointsLock.Unlock()
		if !m.isAccessPointExists(devPath, apPath) {
			return
		}

		if ap.updateProps() {
			m.PropsMu.Lock()
			m.updatePropWirelessAccessPoints()
			m.PropsMu.Unlock()
		}

	})
	if err != nil {
		logger.Warning("failed to monitor changing properties of AccessPoint", err)
	}

	apJSON, _ := marshalJSON(ap)
	err1 := m.service.Emit(m, "AccessPointAdded", string(devPath), apJSON)
	if err1 != nil {
		logger.Warning("failed to emit signal:", err1)
	}

	return
}

func (m *Manager) destroyAccessPoint(ap *accessPoint) {
	// emit AccessPointRemoved signal
	apJSON, _ := marshalJSON(ap)
	err := m.service.Emit(m, "AccessPointRemoved", string(ap.devPath), apJSON)
	if err != nil {
		logger.Warning("failed to emit signal:", err)
	}
	nmDestroyAccessPoint(ap.nmAp)
}

func (a *accessPoint) updateProps() bool {
	ssid, err := a.nmAp.Ssid().Get(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	typ, err := getApSecType(a.nmAp)
	if err != nil {
		logger.Warning(err)
		return false
	}
	strength, err := a.nmAp.Strength().Get(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	frequency, err := a.nmAp.Frequency().Get(0)
	if err != nil {
		logger.Warning(err)
		return false
	}
	flags, err := a.nmAp.Flags().Get(0)
	if err != nil {
		logger.Warning(err)
		return false
	}

	a.Ssid = decodeSsid(ssid)
	a.Secured = typ != apSecNone
	a.SecuredInEap = typ == apSecEap
	a.Strength = strength
	a.Frequency = frequency
	a.Flags = flags
	a.KeyMgmt = getKeyMgmtFromAP(a.nmAp)

	return true
}

func getKeyMgmtFromAP(ap nmdbus.AccessPoint) string {
	keymgmt := "none"

	apflags, err := ap.Flags().Get(0)
	if err != nil {
		logger.Warning("get flags failed, err:", err)
	}
	wpaFlags, err := ap.WpaFlags().Get(0)
	if err != nil {
		logger.Warning("get wpa flags failed, err:", err)
	}
	rsnFlags, err := ap.RsnFlags().Get(0)
	if err != nil {
		logger.Warning("get rsn flags failed, err:", err)
	}

	// WEP, Dynamic WEP, or LEAP
	if (apflags&nm.NM_802_11_AP_FLAGS_PRIVACY != 0) &&
		(wpaFlags == nm.NM_802_11_AP_SEC_NONE) &&
		(rsnFlags == nm.NM_802_11_AP_SEC_NONE) {
		return "wep"
	}

	// WPA/RSN
	if wpaFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_PSK != 0 ||
		rsnFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_PSK != 0 {
		keymgmt = "wpa-psk"
	}

	// 优先sae
	if rsnFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_SAE != 0 {
		keymgmt = "sae"
	}

	if wpaFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_802_1X != 0 ||
		rsnFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_802_1X != 0 {
		keymgmt = "wpa-eap"
	}

	if wpaFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 != 0 ||
		rsnFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 != 0 {
		keymgmt = "wpa-eap-suite-b-192"
	}

	return keymgmt
}

func getApSecType(ap nmdbus.AccessPoint) (apSecType, error) {
	flags, err := ap.Flags().Get(0)
	if err != nil {
		logger.Debugf("get flags failed, err: %v", err)
		return apSecNone, err
	}
	wpaFlags, err := ap.WpaFlags().Get(0)
	if err != nil {
		logger.Debugf("get wpa flags failed, err: %v", err)
		return apSecNone, err
	}
	rsnFlags, err := ap.RsnFlags().Get(0)
	if err != nil {
		logger.Debugf("get rsn flags failed, err: %v", err)
		return apSecNone, err
	}
	return doParseApSecType(flags, wpaFlags, rsnFlags), nil
}

func doParseApSecType(flags, wpaFlags, rsnFlags uint32) apSecType {
	r := apSecNone

	if (flags&nm.NM_802_11_AP_FLAGS_PRIVACY != 0) && (wpaFlags == nm.NM_802_11_AP_SEC_NONE) && (rsnFlags == nm.NM_802_11_AP_SEC_NONE) {
		r = apSecWep
	}
	if wpaFlags != nm.NM_802_11_AP_SEC_NONE {
		r = apSecPsk
	}
	if rsnFlags != nm.NM_802_11_AP_SEC_NONE {
		r = apSecPsk
	}
	if (wpaFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_802_1X != 0) || (rsnFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_802_1X != 0) {
		r = apSecEap
	}
	// prefer sae
	if wpaFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_SAE != 0 || rsnFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_SAE != 0 {
		r = apSecSae
	}
	if (wpaFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 != 0) || (rsnFlags&nm.NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192 != 0) {
		r = apSecEap
	}
	return r
}

func (m *Manager) isAccessPointActivated(devPath dbus.ObjectPath, ssid string) bool {
	for _, path := range nmGetActiveConnections() {
		aconn := m.newActiveConnection(path)
		if aconn.typ == nm.NM_SETTING_WIRELESS_SETTING_NAME && isDBusPathInArray(devPath, aconn.Devices) {
			if ssid == string(nmGetWirelessConnectionSsidByUuid(aconn.Uuid)) {
				return true
			}
		}
	}
	return false
}

func (m *Manager) clearAccessPoints() {
	m.accessPointsLock.Lock()
	defer m.accessPointsLock.Unlock()
	for _, aps := range m.accessPoints {
		for _, ap := range aps {
			m.destroyAccessPoint(ap)
		}
	}
	m.accessPoints = make(map[dbus.ObjectPath][]*accessPoint)
}

func (m *Manager) initAccessPoints(devPath dbus.ObjectPath, apPaths []dbus.ObjectPath) {
	accessPoints := make([]*accessPoint, 0, len(apPaths))
	for _, apPath := range apPaths {
		ap, err := m.newAccessPoint(devPath, apPath)
		if err != nil {
			continue
		}
		//logger.Debug("add access point", devPath, apPath)
		accessPoints = append(accessPoints, ap)
	}

	m.accessPointsLock.Lock()
	m.accessPoints[devPath] = accessPoints
	m.accessPointsLock.Unlock()
}

func (m *Manager) isHidden(ssid string) bool {
	m.connectionsLock.Lock()
	wirelessCon := m.connections[connectionWireless]
	m.connectionsLock.Unlock()
	for _, conn := range wirelessCon {
		if conn.Ssid == ssid && conn.Hidden {
			logger.Debugf("access point %s is hidden ", ssid)
			return true
		}
	}
	return false
}

func (m *Manager) addAccessPoint(devPath, apPath dbus.ObjectPath) {
	if m.isAccessPointExists(devPath, apPath) {
		return
	}
	ap, err := m.newAccessPoint(devPath, apPath)
	if err != nil {
		return
	}
	//logger.Debug("add access point", devPath, apPath)
	m.accessPoints[devPath] = append(m.accessPoints[devPath], ap)
}

func (m *Manager) removeAccessPoint(devPath, apPath dbus.ObjectPath) {
	i := m.getAccessPointIndex(devPath, apPath)
	if i < 0 {
		return
	}
	m.accessPoints[devPath] = m.doRemoveAccessPoint(m.accessPoints[devPath], i)
}

func (m *Manager) doRemoveAccessPoint(aps []*accessPoint, i int) []*accessPoint {
	m.destroyAccessPoint(aps[i])
	copy(aps[i:], aps[i+1:])
	aps[len(aps)-1] = nil
	aps = aps[:len(aps)-1]
	return aps
}

func (m *Manager) isAccessPointExists(devPath, apPath dbus.ObjectPath) bool {
	i := m.getAccessPointIndex(devPath, apPath)
	return i >= 0
}

func (m *Manager) getAccessPointIndex(devPath, apPath dbus.ObjectPath) int {
	for i, ap := range m.accessPoints[devPath] {
		if ap.Path == apPath {
			return i
		}
	}
	return -1
}

// GetAccessPoints return all access points object which marshaled by json.
func (m *Manager) GetAccessPoints(path dbus.ObjectPath) (apsJSON string, busErr *dbus.Error) {
	m.accessPointsLock.Lock()
	defer m.accessPointsLock.Unlock()
	accessPoints := m.accessPoints[path]
	apsJSON, err := marshalJSON(accessPoints)
	busErr = dbusutil.ToError(err)
	return
}

func (m *Manager) ActivateAccessPoint(uuid string, apPath, devPath dbus.ObjectPath) (connection dbus.ObjectPath,
	busErr *dbus.Error) {
	var err error
	cpath, err := m.activateAccessPoint(uuid, apPath, devPath, false)
	if err != nil {
		logger.Warning("failed to activate access point:", err)
		return "/", dbusutil.ToError(err)
	}
	return cpath, nil
}

func (m *Manager) fixApKeyMgmtChange(uuid string, keymgmt string, saved bool, devPath dbus.ObjectPath) (needUserEdit bool, err error) {
	var cpath dbus.ObjectPath
	cpath, err = nmGetConnectionByUuid(uuid)
	if err != nil {
		return
	}

	// 已成功连接过的网络需要更新配置
	isAvailable := false
	connections, err := m.listDeviceConnections(devPath)
	if err != nil {
		logger.Warning(err)
	} else {
		for _, path := range connections {
			if path == cpath {
				isAvailable = true
				break
			}
		}
	}

	var conn nmdbus.ConnectionSettings
	conn, err = nmNewSettingsConnection(cpath)
	if err != nil {
		return
	}
	var connData connectionData
	connData, err = conn.GetSettings(0)
	if err != nil {
		return
	}

	current := getSettingWirelessSecurityKeyMgmt(connData)
	if keymgmt == current {
		return
	}
	logger.Infof("keymgmt change from %v to %v", current, keymgmt)

	if keymgmt == "wpa-eap" || keymgmt == "wpa-eap-suite-b-192" {
		needUserEdit = true
	} else {
		err = logicSetSettingVkWirelessSecurityKeyMgmt(connData, keymgmt)
		if err != nil {
			logger.Warning("failed to set VKWirelessSecutiryKeyMgmt")
			return
		}
	}

	// fix ipv6 addresses and routes data structure, interface{}
	if isSettingIP6ConfigAddressesExists(connData) {
		setSettingIP6ConfigAddresses(connData, getSettingIP6ConfigAddresses(connData))
	}
	if isSettingIP6ConfigRoutesExists(connData) {
		setSettingIP6ConfigRoutes(connData, getSettingIP6ConfigRoutes(connData))
	}

	unsaved, err := conn.Unsaved().Get(0)
	if err != nil {
		unsaved = false
		logger.Warning(err)
	}
	if saved || (isAvailable && !unsaved) {
		err = conn.Update(0, connData)
	} else {
		err = conn.UpdateUnsaved(0, connData)
	}
	return
}

func fixApSecTypeChange(uuid string, secType apSecType) (needUserEdit bool, err error) {
	var cpath dbus.ObjectPath
	cpath, err = nmGetConnectionByUuid(uuid)
	if err != nil {
		return
	}

	var conn nmdbus.ConnectionSettings
	conn, err = nmNewSettingsConnection(cpath)
	if err != nil {
		return
	}
	var connData connectionData
	connData, err = conn.GetSettings(0)
	if err != nil {
		return
	}

	secTypeOld, err := getApSecTypeFromConnData(connData)
	if err != nil {
		logger.Warning("failed to get apSecType from connData")
		return false, nil
	}

	if secTypeOld == secType {
		return
	}
	logger.Debug("apSecType change to", secType)

	switch secType {
	case apSecNone:
		err = logicSetSettingVkWirelessSecurityKeyMgmt(connData, "none")
	case apSecWep:
		err = logicSetSettingVkWirelessSecurityKeyMgmt(connData, "wep")
	case apSecPsk:
		err = logicSetSettingVkWirelessSecurityKeyMgmt(connData, "wpa-psk")
	case apSecSae:
		err = logicSetSettingVkWirelessSecurityKeyMgmt(connData, "sae")
	case apSecEap:
		needUserEdit = true
		return
	}
	if err != nil {
		logger.Debug("failed to set VKWirelessSecutiryKeyMgmt")
		return
	}

	// fix ipv6 addresses and routes data structure, interface{}
	if isSettingIP6ConfigAddressesExists(connData) {
		setSettingIP6ConfigAddresses(connData, getSettingIP6ConfigAddresses(connData))
	}
	if isSettingIP6ConfigRoutesExists(connData) {
		setSettingIP6ConfigRoutes(connData, getSettingIP6ConfigRoutes(connData))
	}

	err = conn.Update(0, connData)
	return
}

// ActivateAccessPoint add and activate connection for access point.
func (m *Manager) activateAccessPoint(uuid string, apPath, devPath dbus.ObjectPath, saved bool) (cpath dbus.ObjectPath, err error) {
	logger.Debugf("ActivateAccessPoint: uuid=%s, apPath=%s, devPath=%s", uuid, apPath, devPath)

	cpath = "/"
	var nmAp nmdbus.AccessPoint
	nmAp, err = nmNewAccessPoint(apPath)
	if err != nil {
		return
	}
	keymgmt := getKeyMgmtFromAP(nmAp)
	if uuid != "" {
		var needUserEdit bool
		needUserEdit, err = m.fixApKeyMgmtChange(uuid, keymgmt, saved, devPath)
		if err != nil {
			return
		}
		if needUserEdit {
			err = errors.New("need user edit")
			return
		}
		cpath, err = m.activateConnection(uuid, devPath)
		if err != nil {
			return
		}
	} else {
		// if there is no connection for current access point, create one
		uuid = utils.GenUuid()
		var ssid []byte
		ssid, err = nmAp.Ssid().Get(0)
		if err != nil {
			logger.Warning("failed to get Ap Ssid:", err)
			return
		}

		// need to set macAddress
		var hwAddr string
		hwAddr, err = nmGeneralGetDeviceHwAddr(devPath, true)
		if err != nil {
			logger.Warning("failed to get mac", err)
		}
		data := newWirelessConnectionData(decodeSsid(ssid), uuid, ssid, keymgmt, hwAddr)
		// check if need add hidden
		if m.isHidden(string(ssid)) {
			setSettingWirelessHidden(data, true)
		}
		if saved {
			cpath, _, err = nmAddAndActivateConnection(data, devPath, true)
		} else {
			cpath, err = nmAddConnectionUnsave(data)
			if err == nil {
				_, err = nmActivateConnection(cpath, devPath)
			}
		}
		if err != nil {
			return
		}
	}
	return
}

func (m *Manager) DebugChangeAPChannel(band string) *dbus.Error {
	if band != "a" && band != "bg" {
		return dbusutil.ToError(errors.New("band input error"))
	}
	err := m.RequestWirelessScan()
	if err != nil {
		logger.Warning("RequestWirelessScan: ", err)
		return dbusutil.ToError(err)
	}
	m.debugChangeAPBand = band
	return nil
}

func (m *Manager) findAPByBand(ssid string, accessPoints []*accessPoint, band string) (apNow *accessPoint) {
	logger.Debug("findAPByBand:", ssid, band)
	apNow = nil
	var strengthMax uint8 = 0
	for _, ap := range accessPoints {
		if band == "" {
			if ap.Ssid == ssid && ap.Strength > strengthMax {
				apNow = ap
				strengthMax = ap.Strength
			}
		} else {
			for _, ap := range accessPoints {
				if ap.Ssid == ssid && band == "a" &&
					ap.Frequency >= frequency5GLowerlimit &&
					ap.Frequency <= frequency5GUpperlimit {
					apNow = ap
					break
				} else if ap.Ssid == ssid && band == "bg" &&
					ap.Frequency >= frequency2GLowerlimit &&
					ap.Frequency <= frequency2GUpperlimit {
					apNow = ap
					break
				}
			}
		}
	}
	return
}

func (m *Manager) getBandByFrequency(freq uint32) (band string) {
	if freq >= frequency5GLowerlimit &&
		freq <= frequency5GUpperlimit {
		band = "a"
	} else if freq >= frequency2GLowerlimit &&
		freq <= frequency2GUpperlimit {
		band = "bg"
	} else {
		band = "unknown"
	}
	return
}

func (m *Manager) checkAPStrength() {
	band := m.debugChangeAPBand
	m.debugChangeAPBand = ""
	m.checkAPStrengthTimer = nil
	logger.Debug("checkAPStrength:")
	if devices, ok := m.devices[deviceWifi]; ok {
		for _, dev := range devices {
			apPath, _ := dev.nmDev.Wireless().ActiveAccessPoint().Get(0)
			nmAp, err := nmNewAccessPoint(apPath)
			if err != nil {
				continue
			}

			frequency, _ := nmAp.Frequency().Get(0)
			strength, _ := nmAp.Strength().Get(0)
			ssid, _ := nmAp.Ssid().Get(0)

			aPath, err := dev.nmDev.Device().ActiveConnection().Get(0)
			if err != nil || !isObjPathValid(aPath) {
				continue
			}
			aConn, err := nmNewActiveConnection(aPath)
			if err != nil {
				logger.Error(err)
				continue
			}

			state, err := aConn.State().Get(0)
			if err != nil {
				logger.Error(err)
				continue
			}
			//当热点还没有连接成功时,不需要切换
			if state != nm.NM_ACTIVE_CONNECTION_STATE_ACTIVATED {
				logger.Debug("do not need change if connection not be activated")
				continue
			}

			if band == "" {
				//当前信号比较好，无需切换
				if strength > channelAutoChangeThreshold && frequency >= frequency5GLowerlimit &&
					frequency <= frequency5GUpperlimit {
					continue
				}
			}

			apNow := m.findAPByBand(decodeSsid(ssid), m.accessPoints[dev.Path], band)
			if apNow == nil {
				logger.Debug("not found AP ")
				continue
			}
			if apNow.Path == apPath {
				logger.Debug("no need to change AP")
				continue
			}
			logger.Debug("changeAPChanel, apPath:", apPath)
			if band == "" {
				// 信号强度改变小于1格，不发生切换
				if apNow.Strength < strength+20 {
					continue
				}
				if apNow.Frequency >= frequency5GLowerlimit &&
					apNow.Frequency <= frequency5GUpperlimit {
					band = "a"
				} else {
					band = "bg"
				}
			}
			if band == m.getBandByFrequency(frequency) {
				logger.Debug("no need to change AP")
				continue
			}

			connPath, err := aConn.Connection().Get(0)
			if err != nil {
				logger.Error(err)
				continue
			}
			conn := m.getConnection(connPath)
			err = m.updateConnectionBand(conn, band)
			if err != nil {
				logger.Error(err)
				continue
			}
			_, err = m.activateAccessPoint(conn.Uuid, apNow.Path, dev.Path, false)
			if err != nil {
				logger.Error(err)
				continue
			}
		}
	}
}
