// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"strconv"
	"strings"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/network1/nm"
	"github.com/linuxdeepin/go-lib/dbusutil"
	. "github.com/linuxdeepin/go-lib/gettext"
)

type activeConnection struct {
	path      dbus.ObjectPath
	typ       string
	vpnFailed bool
	vpnType   string
	vpnState  uint32

	Devices        []dbus.ObjectPath
	conn           dbus.ObjectPath
	Id             string
	Uuid           string
	State          uint32
	Vpn            bool
	SpecificObject dbus.ObjectPath
}

var frequencyChannelMap = map[uint32]int32{
	2412: 1, 2417: 2, 2422: 3, 2427: 4, 2432: 5, 2437: 6, 2442: 7,
	2447: 8, 2452: 9, 2457: 10, 2462: 11, 2467: 12, 2472: 13, 2484: 14,
	5035: 7, 5040: 8, 5045: 9, 5055: 11, 5060: 12, 5080: 16, 5170: 34,
	5180: 36, 5190: 38, 5200: 40, 5220: 44, 5230: 44, 5240: 48, 5260: 52, 5280: 56, 5300: 60,
	5320: 64, 5500: 100, 5520: 104, 5540: 108, 5560: 112, 5580: 116, 5600: 120,
	5620: 124, 5640: 128, 5660: 132, 5680: 136, 5700: 140, 5745: 149, 5765: 153,
	5785: 157, 5805: 161, 5825: 165,
	4915: 183, 4920: 184, 4925: 185, 4935: 187, 4940: 188, 4945: 189, 4960: 192, 4980: 196,
}

type activeConnectionInfo struct {
	IsPrimaryConnection bool
	Device              dbus.ObjectPath
	SettingPath         dbus.ObjectPath
	SpecificObject      dbus.ObjectPath
	ConnectionType      string
	Protocol            string
	ConnectionName      string
	ConnectionUuid      string
	MobileNetworkType   string
	Security            string
	DeviceType          string
	DeviceInterface     string
	HwAddress           string
	Speed               string
	Ip4                 ip4ConnectionInfoDeprecated
	Ip6                 ip6ConnectionInfoDeprecated
	IPv4                ipv4Info
	IPv6                ipv6Info
	Hotspot             hotspotConnectionInfo
}

type addressDataItem struct {
	Address string
	Prefix  uint32
}

type ipv4Info struct {
	Addresses   []addressDataItem
	Gateway     string
	Nameservers []string
}

type ipv6Info struct {
	Addresses   []addressDataItem
	Gateway     string
	Nameservers []string
}

type ip4ConnectionInfoDeprecated struct {
	Address  string
	Mask     string
	Gateways []string
	Dnses    []string
}
type ip6ConnectionInfoDeprecated struct {
	Address  string
	Prefix   string
	Gateways []string
	Dnses    []string
}
type hotspotConnectionInfo struct {
	Ssid    string
	Band    string
	Channel int32 // wireless channel
}

func (i ipv4Info) toDeprecatedStruct() ip4ConnectionInfoDeprecated {
	ip4ConnectionInfoDeprecatedInfo := ip4ConnectionInfoDeprecated{
		Gateways: []string{i.Gateway},
		Dnses:    i.Nameservers,
	}
	if len(i.Addresses) != 0 {
		ip4ConnectionInfoDeprecatedInfo.Address = i.Addresses[0].Address
		ip4ConnectionInfoDeprecatedInfo.Mask = convertIpv4PrefixToNetMask(i.Addresses[0].Prefix)
	}
	return ip4ConnectionInfoDeprecatedInfo
}

func (i ipv6Info) toDeprecatedStruct() ip6ConnectionInfoDeprecated {
	ip6ConnectionInfoDeprecatedInfo := ip6ConnectionInfoDeprecated{
		Gateways: []string{i.Gateway},
		Dnses:    i.Nameservers,
	}
	if len(i.Addresses) != 0 {
		ip6ConnectionInfoDeprecatedInfo.Address = i.Addresses[0].Address
		ip6ConnectionInfoDeprecatedInfo.Prefix = strconv.FormatUint(uint64(i.Addresses[0].Prefix), 10)
	}
	return ip6ConnectionInfoDeprecatedInfo
}

func (m *Manager) initActiveConnectionManage() {
	m.initActiveConnections()

	senderNm := "org.freedesktop.NetworkManager"
	interfaceActiveConnection := "org.freedesktop.NetworkManager.Connection.Active"
	interfaceVpnConnection := "org.freedesktop.NetworkManager.VPN.Connection"
	interfaceDBusProps := "org.freedesktop.DBus.Properties"
	memberPropsChanged := "PropertiesChanged"
	memberVpnStateChanged := "VpnStateChanged"

	err := dbusutil.NewMatchRuleBuilder().
		Type("signal").
		Sender(senderNm).
		Interface(interfaceDBusProps).
		Member(memberPropsChanged).
		ArgNamespace(0, interfaceActiveConnection).Build().
		AddTo(m.sysSigLoop.Conn())
	if err != nil {
		logger.Warning(err)
	}

	err = dbusutil.NewMatchRuleBuilder().
		Type("signal").
		Sender(senderNm).
		Interface(interfaceDBusProps).
		Member(memberPropsChanged).
		ArgNamespace(0, "org.freedesktop.NetworkManager").Build().
		AddTo(m.sysSigLoop.Conn())
	if err != nil {
		logger.Warning(err)
	}

	err = dbusutil.NewMatchRuleBuilder().
		Type("signal").
		Sender(senderNm).
		Interface(interfaceVpnConnection).
		Member(memberVpnStateChanged).Build().
		AddTo(m.sysSigLoop.Conn())
	if err != nil {
		logger.Warning(err)
	}

	sysSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: interfaceDBusProps + "." + memberPropsChanged,
	}, func(sig *dbus.Signal) {
		if strings.HasPrefix(string(sig.Path),
			"/org/freedesktop/NetworkManager/ActiveConnection/") &&
			len(sig.Body) == 3 {

			ifc, ok := sig.Body[0].(string)
			if !ok {
				return
			}

			if ifc != interfaceActiveConnection {
				return
			}

			props, ok := sig.Body[1].(map[string]dbus.Variant)
			if !ok {
				return
			}

			specificPathVar, ok := props["SpecificPath"]
			if ok {
				specificPath, ok := specificPathVar.Value().(dbus.ObjectPath)
				if ok {
					logger.Debugf("active connection %s SpecificPath changed %v", sig.Path, specificPath)
					m.updateActiveConnSpecificPath(sig.Path, specificPath)
				}
			}

			var state uint32
			var stateChanged bool
			stateVar, ok := props["State"]
			if ok {
				state, stateChanged = stateVar.Value().(uint32)
				if stateChanged {
					logger.Debugf("active connection %s state changed %v", sig.Path, state)
					m.updateActiveConnState(sig.Path, state)

				}
			}

			if stateChanged && state == nm.NM_ACTIVE_CONNECTION_STATE_ACTIVATED {
				go m.checkConnectivity()
			}
		}
		if strings.HasPrefix(string(sig.Path),
			"/org/freedesktop/NetworkManager/IP") && len(sig.Body) == 3 {
			ifc, ok := sig.Body[0].(string)
			if !ok {
				logger.Warning("interface must string type")
				return
			}

			switch ifc {
			case "org.freedesktop.NetworkManager.IP4Config":
				fallthrough
			case "org.freedesktop.NetworkManager.IP6Config":
				{
					// ipconfig changed
					go m.updateActiveConnectionInfo()
				}
			}
		}
	})

	// handle notification for vpn connections
	sysSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: interfaceVpnConnection + "." + memberVpnStateChanged,
	}, func(sig *dbus.Signal) {
		if strings.HasPrefix(string(sig.Path),
			"/org/freedesktop/NetworkManager/ActiveConnection/") &&
			len(sig.Body) >= 2 {

			state, ok := sig.Body[0].(uint32)
			if !ok {
				return
			}
			reason, ok := sig.Body[1].(uint32)
			if !ok {
				return
			}
			logger.Debug(sig.Path, "vpn state changed", state, reason)
			m.doHandleVpnNotification(sig.Path, state, reason)
		}
	})
}

func (m *Manager) initActiveConnections() {
	m.activeConnectionsLock.Lock()
	defer m.activeConnectionsLock.Unlock()
	m.activeConnections = make(map[dbus.ObjectPath]*activeConnection)
	for _, path := range nmGetActiveConnections() {
		m.activeConnections[path] = m.newActiveConnection(path)
	}
	m.updatePropActiveConnections()
}

func (m *Manager) doHandleVpnNotification(apath dbus.ObjectPath, state, reason uint32) {
	m.activeConnectionsLock.Lock()
	defer m.activeConnectionsLock.Unlock()

	// get the corresponding active connection
	aConn, ok := m.activeConnections[apath]
	if !ok {
		return
	}

	// if is already state, ignore
	if aConn.vpnState == state {
		return
	}
	// save current state
	aConn.vpnState = state

	// notification for vpn
	switch state {
	case nm.NM_VPN_CONNECTION_STATE_ACTIVATED:
		// NetworkManager may has bug, VPN activated state is emitted unexpectedly when vpn is disconnected
		// vpn connected should not notified when reason is disconnected
		if reason == nm.NM_VPN_CONNECTION_STATE_REASON_USER_DISCONNECTED {
			return
		}
		notifyVpnConnected(aConn.Id)
	case nm.NM_VPN_CONNECTION_STATE_DISCONNECTED:
		if aConn.vpnFailed {
			aConn.vpnFailed = false
		} else {
			notifyVpnDisconnected(aConn.Id)
		}
	case nm.NM_VPN_CONNECTION_STATE_FAILED:
		notifyVpnFailed(aConn.Id, reason)
		aConn.vpnFailed = true
	}
}

func (m *Manager) updateActiveConnSpecificPath(apath dbus.ObjectPath, specificPath dbus.ObjectPath) {
	m.activeConnectionsLock.Lock()
	defer m.activeConnectionsLock.Unlock()

	aConn, ok := m.activeConnections[apath]
	if !ok {
		return
	}
	aConn.SpecificObject = specificPath

	m.updatePropActiveConnections()
}

func (m *Manager) updateActiveConnState(apath dbus.ObjectPath, state uint32) {
	m.activeConnectionsLock.Lock()
	defer m.activeConnectionsLock.Unlock()

	aConn, ok := m.activeConnections[apath]
	if !ok {
		return
	}
	aConn.State = state

	m.updatePropActiveConnections()
}

func (m *Manager) newActiveConnection(path dbus.ObjectPath) (aconn *activeConnection) {
	aconn = &activeConnection{path: path}
	nmAConn, err := nmNewActiveConnection(path)
	if err != nil {
		return
	}

	aconn.conn, _ = nmAConn.Connection().Get(0)
	aconn.State, _ = nmAConn.State().Get(0)
	aconn.Devices, _ = nmAConn.Devices().Get(0)
	aconn.typ, _ = nmAConn.Type().Get(0)
	aconn.Uuid, _ = nmAConn.Uuid().Get(0)
	aconn.Vpn, _ = nmAConn.Vpn().Get(0)
	if cpath, err := nmGetConnectionByUuid(aconn.Uuid); err == nil {
		aconn.Id = nmGetConnectionId(cpath)
		aconn.vpnType = nmGetConnectionVpnType(cpath)
	}
	aconn.SpecificObject, _ = nmAConn.SpecificObject().Get(0)

	return
}

func (m *Manager) clearActiveConnections() {
	m.activeConnectionsLock.Lock()
	defer m.activeConnectionsLock.Unlock()
	m.activeConnections = make(map[dbus.ObjectPath]*activeConnection)
	m.updatePropActiveConnections()
}

func (m *Manager) GetActiveConnectionInfo() (acinfosJSON string, busErr *dbus.Error) {
	var acinfos []activeConnectionInfo
	// get activated devices' connection information
	for _, devPath := range nmGetDevices() {
		if isDeviceStateActivated(nmGetDeviceState(devPath)) {
			if info, err := m.doGetActiveConnectionInfo(nmGetDeviceActiveConnection(devPath), devPath); err == nil {
				acinfos = append(acinfos, info)
			}
		}
	}
	// get activated vpn connection information
	for _, apath := range nmGetVpnActiveConnections() {
		if nmAConn, err := nmNewActiveConnection(apath); err == nil {
			if devs, _ := nmAConn.Devices().Get(0); len(devs) > 0 {
				devPath := devs[0]
				if info, err := m.doGetActiveConnectionInfo(apath, devPath); err == nil {
					acinfos = append(acinfos, info)
				}
			}
		}
	}
	acinfosJSON, err := marshalJSON(acinfos)
	busErr = dbusutil.ToError(err)
	m.acinfosJSON = acinfosJSON
	return
}

func (m *Manager) doGetActiveConnectionInfo(apath, devPath dbus.ObjectPath) (activeConnectionInfo, error) {
	var connType, connName, mobileNetworkType, security, devType, devIfc, hwAddress, speed string
	var hotspotInfo hotspotConnectionInfo
	var acInfo activeConnectionInfo
	var err error

	// active connection
	nmAConn, err := nmNewActiveConnection(apath)
	if err != nil {
		return acInfo, err
	}

	nmAConnConnection, _ := nmAConn.Connection().Get(0)
	nmConn, err := nmNewSettingsConnection(nmAConnConnection)
	if err != nil {
		return acInfo, err
	}

	// device
	nmDev, err := nmNewDevice(devPath)
	if err != nil {
		return acInfo, err
	}

	specificObject, err := nmAConn.SpecificObject().Get(0)
	if err != nil {
		return acInfo, err
	}

	deviceType, _ := nmDev.Device().DeviceType().Get(0)
	devType = getCustomDeviceType(deviceType)
	devIfc, _ = nmDev.Device().Interface().Get(0)
	if devType == deviceModem {
		devUdi, _ := nmDev.Device().Udi().Get(0)
		mobileNetworkType = mmGetModemMobileNetworkType(dbus.ObjectPath(devUdi))
	}

	// connection data
	hwAddress, err = nmGeneralGetDeviceHwAddr(devPath, false)
	if err != nil {
		hwAddress = ""
	}
	speed = nmGeneralGetDeviceSpeed(devPath)

	cdata, err := nmConn.GetSettings(0)
	if err != nil {
		return acInfo, err
	}

	connName = getSettingConnectionId(cdata)
	connType = getCustomConnectionType(cdata)

	if connType == connectionWirelessHotspot || connType == connectionWireless {
		apPath, _ := nmDev.Wireless().ActiveAccessPoint().Get(0)
		nmAp, err := nmNewAccessPoint(apPath)
		if err != nil {
			return acInfo, err
		}

		ssid, _ := nmAp.Ssid().Get(0)
		hotspotInfo.Ssid = decodeSsid(ssid)
		frequency, _ := nmAp.Frequency().Get(0)

		if frequency >= 4915 && frequency <= 5825 {
			hotspotInfo.Band = "a"
		} else if frequency >= 2412 && frequency <= 2484 {
			hotspotInfo.Band = "bg"
		} else {
			hotspotInfo.Band = "unknown"
		}

		hotspotInfo.Channel = frequencyChannelMap[frequency]
	}

	// security
	use8021xSecurity := false
	// get protocol from data
	protocol := getSettingConnectionType(cdata)
	switch protocol {
	case nm.NM_SETTING_WIRED_SETTING_NAME:
		if getSettingVk8021xEnable(cdata) {
			use8021xSecurity = true
		} else {
			security = Tr("None")
		}
	case nm.NM_SETTING_WIRELESS_SETTING_NAME:
		switch getSettingVkWirelessSecurityKeyMgmt(cdata) {
		case "none":
			security = Tr("None")
		case "wep":
			security = Tr("WEP 40/128-bit Key")
		case "wpa-psk":
			security = Tr("WPA/WPA2 Personal")
		case "sae":
			security = Tr("WPA3 Personal")
		case "wpa-eap":
			use8021xSecurity = true
		}
	}
	if use8021xSecurity {
		switch getSettingVk8021xEap(cdata) {
		case "tls":
			security = "EAP/" + Tr("TLS")
		case "md5":
			security = "EAP/" + Tr("MD5")
		case "leap":
			security = "EAP/" + Tr("LEAP")
		case "fast":
			security = "EAP/" + Tr("FAST")
		case "ttls":
			security = "EAP/" + Tr("Tunneled TLS")
		case "peap":
			security = "EAP/" + Tr("Protected EAP")
		}
	}

	// ipv4
	var ip4Data ipv4Info
	if ip4Path, _ := nmAConn.Ip4Config().Get(0); isNmObjectPathValid(ip4Path) {
		ip4Data = nmGetIp4ConfigInfo(ip4Path)
	}

	// ipv6
	var ip6Data ipv6Info
	if ip6Path, _ := nmAConn.Ip6Config().Get(0); isNmObjectPathValid(ip6Path) {
		ip6Data = nmGetIp6ConfigInfo(ip6Path)
	}

	nmAConnUuid, _ := nmAConn.Uuid().Get(0)
	acInfo = activeConnectionInfo{
		IsPrimaryConnection: nmGetPrimaryConnection() == apath,
		Device:              devPath,
		SettingPath:         nmConn.Path_(),
		SpecificObject:      specificObject,
		ConnectionType:      connType,
		Protocol:            protocol,
		ConnectionName:      connName,
		ConnectionUuid:      nmAConnUuid,
		MobileNetworkType:   mobileNetworkType,
		Security:            security,
		DeviceType:          devType,
		DeviceInterface:     devIfc,
		HwAddress:           hwAddress,
		Speed:               speed,
		Ip4:                 ip4Data.toDeprecatedStruct(),
		Ip6:                 ip6Data.toDeprecatedStruct(),
		IPv4:                ip4Data,
		IPv6:                ip6Data,
		Hotspot:             hotspotInfo,
	}

	return acInfo, err
}

func (m *Manager) updateActiveConnectionInfo() {
	var acinfos []activeConnectionInfo
	// get activated devices' connection information
	for _, devPath := range nmGetDevices() {
		if isDeviceStateActivated(nmGetDeviceState(devPath)) {
			if info, err := m.doGetActiveConnectionInfo(nmGetDeviceActiveConnection(devPath), devPath); err == nil {
				acinfos = append(acinfos, info)
			}
		}
	}
	// get activated vpn connection information
	for _, apath := range nmGetVpnActiveConnections() {
		if nmAConn, err := nmNewActiveConnection(apath); err == nil {
			if devs, _ := nmAConn.Devices().Get(0); len(devs) > 0 {
				devPath := devs[0]
				if info, err := m.doGetActiveConnectionInfo(apath, devPath); err == nil {
					acinfos = append(acinfos, info)
				}
			}
		}
	}
	acinfosJSON, _ := marshalJSON(acinfos)
	if acinfosJSON != m.acinfosJSON {
		logger.Debug("ActiveConnectionInfoChanged...")
		m.service.Emit(m, "ActiveConnectionInfoChanged")
	}
}
