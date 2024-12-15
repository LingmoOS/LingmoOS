// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

import (
	"github.com/lingmoos/go-lib/strv"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/lingmoos/ocean-daemon/timedate1/zoneinfo"
	"github.com/lingmoos/go-lib/dbusutil"
	. "github.com/lingmoos/go-lib/gettext"
)

var customTimeZoneList = []string{"Asia/Chengdu", "Asia/Beijing", "Asia/Nanjing", "Asia/Wuhan", "Asia/Xian"}

func (m *Manager) Reset() *dbus.Error {
	return m.SetNTP(true)
}

// SetDate Set the system clock to the specified.
//
// The time may be specified in the format '2015' '1' '1' '18' '18' '18' '8'.
func (m *Manager) SetDate(year, month, day, hour, min, sec, nsec int32) *dbus.Error {
	timeZone := m.Timezone
	customTimeZoneList := []string{"Asia/Chengdu", "Asia/Beijing", "Asia/Nanjing", "Asia/Wuhan", "Asia/Xian"}
	if strv.Strv(customTimeZoneList).Contains(m.Timezone) {
		timeZone = "Asia/Shanghai"
	}
	loc, err := time.LoadLocation(timeZone)
	if err != nil {
		logger.Debugf("Load location '%s' failed: %v", m.Timezone, err)
		return dbusutil.ToError(err)
	}
	t := time.Date(int(year), time.Month(month), int(day),
		int(hour), int(min), int(sec), int(nsec), loc)

	// microseconds since 1 Jan 1970 UTC
	us := (t.Unix() * 1000000) + int64(t.Nanosecond()/1000)
	err1 := m.SetTime(us, false)
	if err1 == nil {
		err = m.service.Emit(m, "TimeUpdate")
		if err != nil {
			logger.Debug("emit TimeUpdate failed:", err)
		}
	}
	return err1
}

// Set the system clock to the specified.
//
// usec: pass a value of microseconds since 1 Jan 1970 UTC.
//
// relative: if true, the passed usec value will be aoceand to the current system time; if false, the current system time will be set to the passed usec value.
func (m *Manager) SetTime(usec int64, relative bool) *dbus.Error {
	err := m.setter.SetTime(0, usec, relative,
		Tr("Authentication is required to set the system time"))
	if err != nil {
		logger.Debug("SetTime failed:", err)
	}

	return dbusutil.ToError(err)
}

// To control whether the system clock is synchronized with the network.
//
// useNTP: if true, enable ntp; else disable
func (m *Manager) SetNTP(useNTP bool) *dbus.Error {
	err := m.setter.SetNTP(0, useNTP,
		Tr("Authentication is required to control whether network time synchronization shall be enabled"))
	if err != nil {
		logger.Debug("SetNTP failed:", err)
	}

	return dbusutil.ToError(err)
}

func (m *Manager) SetNTPServer(server string) *dbus.Error {
	err := m.setter.SetNTPServer(0, server,
		Tr("Authentication is required to change NTP server"))
	if err != nil {
		logger.Warning("SetNTPServer failed:", err)
	}

	return dbusutil.ToError(err)
}

func (m *Manager) GetSampleNTPServers() (servers []string, busErr *dbus.Error) {
	servers = []string{
		"ntp.ntsc.ac.cn",
		"cn.ntp.org.cn",

		"0.debian.pool.ntp.org",
		"1.debian.pool.ntp.org",
		"2.debian.pool.ntp.org",
		"3.debian.pool.ntp.org",

		"0.arch.pool.ntp.org",
		"1.arch.pool.ntp.org",
		"2.arch.pool.ntp.org",
		"3.arch.pool.ntp.org",

		"0.fedora.pool.ntp.org",
		"1.fedora.pool.ntp.org",
		"2.fedora.pool.ntp.org",
		"3.fedora.pool.ntp.org",
	}
	return servers, nil
}

// To control whether the RTC is the local time or UTC.
// Standards are divided into: localtime and UTC.
// UTC standard will automatically adjust the daylight saving time.
//
// 实时时间(RTC)是否使用 local 时间标准。时间标准分为 local 和 UTC。
// UTC 时间标准会自动根据夏令时而调整系统时间。
//
// localRTC: whether to use local time.
//
// fixSystem: if true, will use the RTC time to adjust the system clock; if false, the system time is written to the RTC taking the new setting into account.
func (m *Manager) SetLocalRTC(localRTC, fixSystem bool) *dbus.Error {
	err := m.setter.SetLocalRTC(0, localRTC, fixSystem,
		Tr("Authentication is required to control whether the RTC stores the local or UTC time"))
	if err != nil {
		logger.Debug("SetLocalRTC failed:", err)
	}

	return dbusutil.ToError(err)
}

// Set the system time zone to the specified value.
// timezones you may parse from /usr/share/zoneinfo/zone.tab.
//
// zone: pass a value like "Asia/Shanghai" to set the timezone.
func (m *Manager) SetTimezone(zone string) *dbus.Error {
	ok, err := zoneinfo.IsZoneValid(zone)
	if err != nil {
		return dbusutil.ToError(err)
	}
	if !ok {
		logger.Debug("Invalid zone:", zone)
		return dbusutil.ToError(zoneinfo.ErrZoneInvalid)
	}
	// 如果要设置的时区是自定义时区或者是上海时区且当前的时区是自定义时区，需要更新时区属性
	// 否则在设置完系统时区后再更新
	isNeedUpdateProp := zone == "Asia/Shanghai" && strv.Strv(customTimeZoneList).Contains(m.Timezone) ||
		strv.Strv(customTimeZoneList).Contains(zone)
	err = m.setter.SetTimezone(0, zone,
		Tr("Authentication is required to set the system timezone"))
	if err != nil {
		logger.Debug("SetTimezone failed:", err)
		return dbusutil.ToError(err)
	}
	if isNeedUpdateProp {
		m.PropsMu.Lock()
		m.setPropTimezone(zone)
		m.PropsMu.Unlock()
		err = m.dConfigManager.SetValue(dbus.Flags(0), dSettingsTimeZone, dbus.MakeVariant(zone))
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	}
	return m.AddUserTimezone(zone)
}

// Add the specified time zone to user time zone list.
func (m *Manager) AddUserTimezone(zone string) *dbus.Error {
	ok, err := zoneinfo.IsZoneValid(zone)
	if err != nil {
		return dbusutil.ToError(err)
	}
	if !ok {
		logger.Debug("Invalid zone:", zone)
		return dbusutil.ToError(zoneinfo.ErrZoneInvalid)
	}

	oldList, hasNil := filterNilString(m.UserTimezones)
	newList, aoceand := addItemToList(zone, oldList)
	if aoceand || hasNil {
		m.PropsMu.Lock()
		m.UserTimezones = newList
		m.PropsMu.Unlock()
		m.service.EmitPropertyChanged(m, "UserTimezones", m.UserTimezones)
		err = m.dConfigManager.SetValue(dbus.Flags(0), dSettingsKeyTimezoneList, dbus.MakeVariant(m.UserTimezones))
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	}
	return nil
}

// Delete the specified time zone from user time zone list.
func (m *Manager) DeleteUserTimezone(zone string) *dbus.Error {
	ok, err := zoneinfo.IsZoneValid(zone)
	if err != nil {
		return dbusutil.ToError(err)
	}
	if !ok {
		logger.Debug("Invalid zone:", zone)
		return dbusutil.ToError(zoneinfo.ErrZoneInvalid)
	}

	oldList, hasNil := filterNilString(m.UserTimezones)
	newList, deleted := deleteItemFromList(zone, oldList)
	if deleted || hasNil {
		m.UserTimezones = newList
		m.service.EmitPropertyChanged(m, "UserTimezones", m.UserTimezones)
	}
	return nil
}

// GetZoneInfo returns the information of the specified time zone.
func (m *Manager) GetZoneInfo(zone string) (zoneInfo zoneinfo.ZoneInfo, busErr *dbus.Error) {
	m.PropsMu.Lock()
	info, err := zoneinfo.GetZoneInfo(zone)
	m.PropsMu.Unlock()
	if strv.Strv(customTimeZoneList).Contains(zone) {
		zoneShanghai := "Asia/Shanghai"
		zoneInfoShanghai, err := zoneinfo.GetZoneInfo(zoneShanghai)
		if err != nil {
			logger.Debugf("Get zone info for '%s' failed: %v", zone, err)
			return zoneinfo.ZoneInfo{}, dbusutil.ToError(err)
		}
		info.Offset = zoneInfoShanghai.Offset
		info.DST = zoneInfoShanghai.DST
	}
	if err != nil {
		logger.Debugf("Get zone info for '%s' failed: %v", zone, err)
		return zoneinfo.ZoneInfo{}, dbusutil.ToError(err)
	}

	return *info, nil
}

// GetZoneList returns all the valid timezones.
func (m *Manager) GetZoneList() (zoneList []string, busErr *dbus.Error) {
	zoneList, err := zoneinfo.GetAllZones()
	return zoneList, dbusutil.ToError(err)
}
