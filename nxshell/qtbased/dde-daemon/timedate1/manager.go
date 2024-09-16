// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

import (
	"os"
	"os/user"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	ddbus "github.com/linuxdeepin/dde-daemon/dbus"
	"github.com/linuxdeepin/dde-daemon/session/common"
	accounts "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.accounts1"
	timedated "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.timedate1"
	timedate1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.timedate1"
	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

const (
	timeDateSchema             = "com.deepin.dde.datetime"
	settingsKey24Hour          = "is-24hour"
	settingsKeyTimezoneList    = "user-timezone-list"
	settingsKeyDSTOffset       = "dst-offset"
	settingsKeyWeekdayFormat   = "weekday-format"
	settingsKeyShortDateFormat = "short-date-format"
	settingsKeyLongDateFormat  = "long-date-format"
	settingsKeyShortTimeFormat = "short-time-format"
	settingsKeyLongTimeFormat  = "long-time-format"
	settingsKeyWeekBegins      = "week-begins"

	dbusServiceName = "org.deepin.dde.Timedate1"
	dbusPath        = "/org/deepin/dde/Timedate1"
	dbusInterface   = dbusServiceName
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
	Timezone  string
	NTPServer string

	// dbusutil-gen: ignore-below
	// Use 24 hour format to display time
	Use24HourFormat gsprop.Bool `prop:"access:rw"`
	// DST offset
	DSTOffset gsprop.Int `prop:"access:rw"`
	// User added timezone list
	UserTimezones gsprop.Strv

	// weekday shows format
	WeekdayFormat gsprop.Int `prop:"access:rw"`

	// short date shows format
	ShortDateFormat gsprop.Int `prop:"access:rw"`

	// long date shows format
	LongDateFormat gsprop.Int `prop:"access:rw"`

	// short time shows format
	ShortTimeFormat gsprop.Int `prop:"access:rw"`

	// long time shows format
	LongTimeFormat gsprop.Int `prop:"access:rw"`

	WeekBegins gsprop.Int `prop:"access:rw"`

	settings *gio.Settings
	td       timedate1.Timedate
	setter   timedated.Timedate
	userObj  accounts.User

	//nolint
	signals *struct {
		TimeUpdate struct {
		}
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
	m.setter = timedated.NewTimedate(sysBus)

	m.settings = gio.NewSettings(timeDateSchema)
	m.Use24HourFormat.Bind(m.settings, settingsKey24Hour)
	m.DSTOffset.Bind(m.settings, settingsKeyDSTOffset)
	m.UserTimezones.Bind(m.settings, settingsKeyTimezoneList)

	m.WeekdayFormat.Bind(m.settings, settingsKeyWeekdayFormat)
	m.ShortDateFormat.Bind(m.settings, settingsKeyShortDateFormat)
	m.LongDateFormat.Bind(m.settings, settingsKeyLongDateFormat)
	m.ShortTimeFormat.Bind(m.settings, settingsKeyShortTimeFormat)
	m.LongTimeFormat.Bind(m.settings, settingsKeyLongTimeFormat)
	m.WeekBegins.Bind(m.settings, settingsKeyWeekBegins)

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
	m.setPropTimezone(timezone)

	m.PropsMu.Unlock()

	newList, hasNil := filterNilString(m.UserTimezones.Get())
	if hasNil {
		m.UserTimezones.Set(newList)
	}
	err = m.AddUserTimezone(m.Timezone)
	if err != nil {
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
		}
	}

	sysBus := m.systemSigLoop.Conn()
	m.initUserObj(sysBus)
	m.handleGSettingsChanged()
	m.systemSigLoop.Start()
	m.listenPropChanged()
}

func (m *Manager) destroy() {
	m.settings.Unref()
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

	err = common.ActivateSysDaemonService("org.deepin.dde.Accounts1")
	if err != nil {
		logger.Warning(err)
	}

	m.userObj, err = ddbus.NewUserByUid(systemConn, cur.Uid)
	if err != nil {
		logger.Warning("failed to new user object:", err)
		return
	}

	// sync use 24 hour format
	use24hourFormat := m.settings.GetBoolean(settingsKey24Hour)
	err = m.userObj.SetUse24HourFormat(0, use24hourFormat)
	if err != nil {
		logger.Warning(err)
	}

	weekdayFormat := m.settings.GetInt(settingsKeyWeekdayFormat)
	err = m.userObj.SetWeekdayFormat(0, weekdayFormat)
	if err != nil {
		logger.Warning(err)
	}

	shortDateFormat := m.settings.GetInt(settingsKeyShortDateFormat)
	err = m.userObj.SetShortDateFormat(0, shortDateFormat)
	if err != nil {
		logger.Warning(err)
	}

	longDateFormat := m.settings.GetInt(settingsKeyLongDateFormat)
	err = m.userObj.SetLongDateFormat(0, longDateFormat)
	if err != nil {
		logger.Warning(err)
	}

	shortTimeFormat := m.settings.GetInt(settingsKeyShortTimeFormat)
	err = m.userObj.SetShortTimeFormat(0, shortTimeFormat)
	if err != nil {
		logger.Warning(err)
	}

	longTimeFormat := m.settings.GetInt(settingsKeyLongTimeFormat)
	err = m.userObj.SetLongDateFormat(0, longTimeFormat)
	if err != nil {
		logger.Warning(err)
	}

	weekBegins := m.settings.GetInt(settingsKeyWeekBegins)
	err = m.userObj.SetWeekBegins(0, weekBegins)
	if err != nil {
		logger.Warning(err)
	}
}
