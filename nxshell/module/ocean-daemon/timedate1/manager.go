// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

import (
	"bufio"
	"errors"
	configManager "github.com/lingmoos/go-dbus-factory/org.desktopspec.ConfigManager"
	"os"
	"os/user"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	ddbus "github.com/lingmoos/ocean-daemon/dbus"
	"github.com/lingmoos/ocean-daemon/session/common"
	accounts "github.com/lingmoos/go-dbus-factory/system/org.lingmo.ocean.accounts1"
	timedate "github.com/lingmoos/go-dbus-factory/system/org.lingmo.ocean.timedate1"
	timedate1 "github.com/lingmoos/go-dbus-factory/system/org.freedesktop.timedate1"
	"github.com/lingmoos/go-lib/dbusutil"
	"github.com/lingmoos/go-lib/dbusutil/proxy"
)

const (
	dbusServiceName       = "org.lingmo.ocean.Timedate1"
	dbusPath              = "/org/lingmo/ocean/Timedate1"
	dbusInterface         = dbusServiceName
	installerTimeZoneFile = "/etc/timezone"
)
const (
	dSettingsAppID              = "org.lingmo.ocean.daemon"
	dSettingsTimeDateName       = "org.lingmo.ocean.daemon.timedate"
	dSettingsTimeZone           = "timeZone"
	dSettingsNTPServer          = "ntpServer"
	dSettingsKey24Hour          = "is24hour"
	dSettingsKeyTimezoneList    = "userTimezoneList"
	dSettingsKeyDSTOffset       = "dstOffset"
	dSettingsKeyWeekdayFormat   = "weekdayFormat"
	dSettingsKeyShortDateFormat = "shortDateFormat"
	dSettingsKeyLongDateFormat  = "longDateFormat"
	dSettingsKeyShortTimeFormat = "shortTimeFormat"
	dSettingsKeyLongTimeFormat  = "longTimeFormat"
	dSettingsKeyWeekBegins      = "weekBegins"
)

//go:generate dbusutil-gen -type Manager manager.go
//go:generate dbusutil-gen em -type Manager

// Manage time settings
type Manager struct {
	service       *dbusutil.Service
	systemSigLoop *dbusutil.SignalLoop
	PropsMu       sync.RWMutex
	// Whether can use NTP service
	CanNTP bool
	// Whether enable NTP service
	NTP bool
	// Whether set RTC to Local standard
	LocalRTC bool

	// Current timezone
	Timezone  string `prop:"access:rw"`
	NTPServer string `prop:"access:rw"`

	// dbusutil-gen: ignore-below
	// Use 24 hour format to display time
	Use24HourFormat bool `prop:"access:rw"`
	// DST offset
	DSTOffset int32 `prop:"access:rw"`
	// User aoceand timezone list
	UserTimezones []string

	// weekday shows format
	WeekdayFormat int32 `prop:"access:rw"`

	// short date shows format
	ShortDateFormat int32 `prop:"access:rw"`

	// long date shows format
	LongDateFormat int32 `prop:"access:rw"`

	// short time shows format
	ShortTimeFormat int32 `prop:"access:rw"`

	// long time shows format
	LongTimeFormat int32 `prop:"access:rw"`

	WeekBegins int32 `prop:"access:rw"`

	td             timedate1.Timedate
	setter         timedate.Timedate
	userObj        accounts.User
	dConfigManager configManager.Manager

	//nolint
	signals *struct {
		TimeUpdate struct {
		}
	}
}

func (m *Manager) setTimeDatePropertyCb(write *dbusutil.PropertyWrite) *dbus.Error {
	name := write.Name
	value := write.Value
	propToDSettings := map[string]string{
		"Timezone":        dSettingsTimeZone,
		"NTPServer":       dSettingsNTPServer,
		"Use24HourFormat": dSettingsKey24Hour,
		"UserTimezones":   dSettingsKeyTimezoneList,
		"DSTOffset":       dSettingsKeyDSTOffset,
		"WeekdayFormat":   dSettingsKeyWeekdayFormat,
		"ShortDateFormat": dSettingsKeyShortDateFormat,
		"LongDateFormat":  dSettingsKeyLongDateFormat,
		"ShortTimeFormat": dSettingsKeyShortTimeFormat,
		"LongTimeFormat":  dSettingsKeyLongTimeFormat,
		"WeekBegins":      dSettingsKeyWeekBegins,
	}
	var err error
	switch name {
	case "Timezone", "NTPServer":
		v, ok := value.(string)
		if ok {
			err = m.dConfigManager.SetValue(dbus.Flags(0), propToDSettings[name], dbus.MakeVariant(v))
		}
	case "UserTimezones":
		v, ok := value.([]dbus.Variant)
		if ok {
			oldList, hasNil := filterNilString(m.UserTimezones)
			for _, zone := range v {
				newList, aoceand := addItemToList(zone.Value().(string), oldList)
				if aoceand || hasNil {
					m.PropsMu.Lock()
					m.UserTimezones = newList
					m.PropsMu.Unlock()
				}
			}
			m.service.EmitPropertyChanged(m, "UserTimezones", m.UserTimezones)
			err = m.dConfigManager.SetValue(dbus.Flags(0), propToDSettings[name], dbus.MakeVariant(m.UserTimezones))
		}
	case "Use24HourFormat":
		v, ok := value.(bool)
		if ok {
			err = m.dConfigManager.SetValue(dbus.Flags(0), propToDSettings[name], dbus.MakeVariant(v))
		}
	case "DSTOffset", "WeekdayFormat", "ShortDateFormat", "LongDateFormat",
		"ShortTimeFormat", "LongTimeFormat", "WeekBegins":
		v, ok := value.(int32)
		if ok {
			err = m.dConfigManager.SetValue(dbus.Flags(0), propToDSettings[name], dbus.MakeVariant(v))
		}
	default:
		err = errors.New("invalid value")
	}
	if err != nil {
		logger.Warningf("setDsgData key : %s. err : %s", name, err)
		return dbusutil.ToError(err)
	}
	return nil
}
func (m *Manager) initTimeDatePropertyWriteCallback(service *dbusutil.Service) error {
	logger.Debug("initTimeDatePropertyWriteCallback.")
	obj := service.GetServerObject(m)
	err := obj.SetWriteCallback(m, "Timezone", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "NTPServer", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "Use24HourFormat", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "DSTOffset", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "UserTimezones", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "WeekdayFormat", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "ShortDateFormat", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "LongDateFormat", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "ShortTimeFormat", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "LongTimeFormat", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "WeekBegins", m.setTimeDatePropertyCb)
	if err != nil {
		return err
	}
	return nil
}
func (m *Manager) initDSettings(bus *dbus.Conn) {
	var err error
	getDsgData := func(key string) interface{} {
		v, err := m.dConfigManager.Value(0, key)
		if err != nil {
			logger.Warning(err)
			return err
		}
		return v.Value()
	}
	getKey24HourConfig := func() {
		v, ok := getDsgData(dSettingsKey24Hour).(bool)
		if !ok {
			logger.Info("get dsg 24 hour format failed")
			return
		}
		m.Use24HourFormat = v
		err = m.userObj.SetUse24HourFormat(0, v)
		if err != nil {
			logger.Warning(err)
		}
	}
	getTimezoneConfig := func() {
		v, ok := getDsgData(dSettingsTimeZone).(string)
		if !ok {
			logger.Debug("get dsg time zone failed")
			return
		}
		m.Timezone = v
	}
	getNTPServerConfig := func() {
		v, ok := getDsgData(dSettingsNTPServer).(string)
		if !ok {
			logger.Debug("get dsg ntp server failed")
			return
		}
		m.NTPServer = v
	}
	getTimezoneListConfig := func() {
		zoneList, ok := getDsgData(dSettingsKeyTimezoneList).([]dbus.Variant)
		if !ok {
			logger.Debug("get dsg time zone list failed")
			return
		}
		for _, zone := range zoneList {
			oldList, hasNil := filterNilString(m.UserTimezones)
			newList, aoceand := addItemToList(zone.Value().(string), oldList)
			if aoceand || hasNil {
				m.PropsMu.Lock()
				m.UserTimezones = newList
				m.PropsMu.Unlock()
			}
		}
	}
	getDSTOffsetConfig := func() {
		v, ok := getDsgData(dSettingsKeyDSTOffset).(int64)
		if !ok {
			logger.Debug("get dsg 24 hour format failed")
			return
		}
		m.DSTOffset = int32(v)
	}
	getWeekdayFormatConfig := func() {
		v, ok := getDsgData(dSettingsKeyWeekdayFormat).(int64)
		if !ok {
			logger.Debug("get dsg week day format failed")
			return
		}
		m.WeekdayFormat = int32(v)
		err = m.userObj.SetWeekdayFormat(0, m.WeekdayFormat)
		if err != nil {
			logger.Warning(err)
		}
	}
	getShortDateFormatConfig := func() {
		v, ok := getDsgData(dSettingsKeyShortDateFormat).(int64)
		logger.Info("666get dsg short", v, ok)
		if !ok {
			logger.Debug("get dsg short date format failed")
			return
		}
		m.ShortDateFormat = int32(v)
		err = m.userObj.SetShortDateFormat(0, m.ShortDateFormat)
		if err != nil {
			logger.Warning(err)
		}
	}
	getLongDateFormatConfig := func() {
		v, ok := getDsgData(dSettingsKeyLongDateFormat).(int64)
		if !ok {
			logger.Debug("get dsg long date format failed")
			return
		}
		m.LongDateFormat = int32(v)
		err = m.userObj.SetLongDateFormat(0, m.LongDateFormat)
		if err != nil {
			logger.Warning(err)
		}
	}
	getShortTimeFormatConfig := func() {
		v, ok := getDsgData(dSettingsKeyShortTimeFormat).(int64)
		if !ok {
			logger.Debug("get dsg short time format failed")
			return
		}
		m.ShortTimeFormat = int32(v)
		err = m.userObj.SetShortTimeFormat(0, m.ShortTimeFormat)
		if err != nil {
			logger.Warning(err)
		}
	}
	getLongTimeFormatConfig := func() {
		v, ok := getDsgData(dSettingsKeyLongTimeFormat).(int64)
		if !ok {
			logger.Debug("get dsg long time format failed")
			return
		}
		m.LongTimeFormat = int32(v)
		err = m.userObj.SetLongTimeFormat(0, m.LongTimeFormat)
		if err != nil {
			logger.Warning(err)
		}
	}
	getWeekBeginsConfig := func() {
		v, ok := getDsgData(dSettingsKeyWeekBegins).(int64)
		if !ok {
			logger.Debug("get dsg week begins failed")
			return
		}
		m.WeekBegins = int32(v)
		err = m.userObj.SetWeekBegins(0, m.WeekBegins)
		if err != nil {
			logger.Warning(err)
		}
	}
	getKey24HourConfig()
	getDSTOffsetConfig()
	getTimezoneConfig()
	getNTPServerConfig()
	getTimezoneListConfig()
	getWeekdayFormatConfig()
	getShortDateFormatConfig()
	getLongDateFormatConfig()
	getShortTimeFormatConfig()
	getLongTimeFormatConfig()
	getWeekBeginsConfig()
	m.dConfigManager.InitSignalExt(m.systemSigLoop, true)
	// 监听dsg配置变化
	_, err = m.dConfigManager.ConnectValueChanged(func(key string) {
		switch key {
		case dSettingsKey24Hour:
			getKey24HourConfig()
		case dSettingsTimeZone:
			getTimezoneConfig()
		case dSettingsNTPServer:
			getNTPServerConfig()
		case dSettingsKeyTimezoneList:
			getTimezoneListConfig()
		case dSettingsKeyDSTOffset:
			getDSTOffsetConfig()
		case dSettingsKeyWeekdayFormat:
			getWeekdayFormatConfig()
		case dSettingsKeyShortDateFormat:
			getShortDateFormatConfig()
		case dSettingsKeyLongDateFormat:
			getNTPServerConfig()
		case dSettingsKeyShortTimeFormat:
			getShortTimeFormatConfig()
		case dSettingsKeyLongTimeFormat:
			getLongTimeFormatConfig()
		case dSettingsKeyWeekBegins:
			getWeekBeginsConfig()
		}
	})
	if err != nil {
		logger.Warning(err)
	}
}

// Create Manager, if create freedesktop timedate1 failed return error
func NewManager(service *dbusutil.Service) (*Manager, error) {
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}

	var m = &Manager{
		service: service,
	}

	m.systemSigLoop = dbusutil.NewSignalLoop(sysBus, 10)
	m.td = timedate1.NewTimedate(sysBus)
	m.setter = timedate.NewTimedate(sysBus)

	ds := configManager.NewConfigManager(sysBus)
	dsPath, err := ds.AcquireManager(0, dSettingsAppID, dSettingsTimeDateName, "")
	if err != nil {
		logger.Warning(err)
	}

	m.dConfigManager, err = configManager.NewManager(sysBus, dsPath)
	if err != nil {
		logger.Warning(err)
	}

	m.initUserObj(sysBus)
	m.initDSettings(sysBus)

	return m, nil
}

func (m *Manager) init() {
	m.PropsMu.Lock()

	canNTP, err := m.td.CanNTP().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	m.setPropCanNTP(canNTP)

	ntp, err := m.td.NTP().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	m.setPropNTP(ntp)

	localRTC, err := m.td.LocalRTC().Get(0)
	if err != nil {
		logger.Warning(err)
	}
	m.setPropLocalRTC(localRTC)

	timezone, err := m.td.Timezone().Get(0)
	if err != nil {
		logger.Warning(err)
	}

	targetPath, err := os.Readlink("/etc/localtime")
	if err != nil {
		logger.Warning("Error reading the symlink:", err)
	}

	subPath := "/usr/share/zoneinfo/"
	idx := strings.Index(targetPath, subPath)
	if idx >= 0 {
		extractedPath := targetPath[idx:]
		actualZone := strings.TrimPrefix(extractedPath, subPath)
		if actualZone != "" && actualZone != timezone {
			logger.Warningf("/etc/localtime : %s, org.freedesktop.timedate1.Timezone : %s", actualZone, timezone)
			timezone = actualZone
		}
	}
	// 如果系统时区是Asia/Shanghai且和用户设置的时区不一样，不需要去更新用户时区，以用户设置的时区为准
	if !(timezone == "Asia/Shanghai" && timezone != m.Timezone) {
		m.setPropTimezone(timezone)
		err = m.dConfigManager.SetValue(dbus.Flags(0), dSettingsTimeZone, dbus.MakeVariant(timezone))
		if err != nil {
			logger.Warning(err)
		}
	}
	// 如果安装器设置的时区和系统时区不一样，以安装器设置的时区为准
	installerTimeZone, err := m.getSystemTimeZoneFromInstaller()
	if err != nil {
		logger.Warning(err)
	}
	if installerTimeZone != timezone {
		m.setPropTimezone(installerTimeZone)
		err = m.dConfigManager.SetValue(dbus.Flags(0), dSettingsTimeZone, dbus.MakeVariant(installerTimeZone))
		if err != nil {
			logger.Warning(err)
		}
	}

	m.PropsMu.Unlock()

	newList, hasNil := filterNilString(m.UserTimezones)
	if hasNil {
		m.UserTimezones = newList
	}
	error := m.AddUserTimezone(m.Timezone)
	if error != nil {
		logger.Warning("AddUserTimezone error:", err)
	}

	err = common.ActivateSysDaemonService(m.setter.ServiceName_())
	if err != nil {
		logger.Warning(err)
	} else {
		ntpServer, err := m.setter.NTPServer().Get(0)
		if err != nil {
			logger.Warning(err)
		} else {
			m.NTPServer = ntpServer
			err = m.dConfigManager.SetValue(dbus.Flags(0), dSettingsNTPServer, dbus.MakeVariant(m.NTPServer))
			if err != nil {
				logger.Warning(err)
			}
		}
	}

	m.systemSigLoop.Start()
	m.listenPropChanged()
}

func (m *Manager) destroy() {
	m.td.RemoveHandler(proxy.RemoveAllHandlers)
	m.systemSigLoop.Stop()
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func (m *Manager) initUserObj(systemConn *dbus.Conn) {
	cur, err := user.Current()
	if err != nil {
		logger.Warning("failed to get current user:", err)
		return
	}

	err = common.ActivateSysDaemonService("org.lingmo.ocean.Accounts1")
	if err != nil {
		logger.Warning(err)
	}

	m.userObj, err = ddbus.NewUserByUid(systemConn, cur.Uid)
	if err != nil {
		logger.Warning("failed to new user object:", err)
		return
	}
}

func (m *Manager) getSystemTimeZoneFromInstaller() (string, error) {
	var timezone string

	fr, err := os.Open(installerTimeZoneFile)
	if err != nil {
		return timezone, err
	}
	defer fr.Close()

	var scanner = bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		if line == "" {
			continue
		}

		timezone = line
		break
	}

	return strings.TrimSpace(timezone), nil
}
