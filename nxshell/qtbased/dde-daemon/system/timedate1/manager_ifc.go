// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedated

import (
	"os"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/timedate1/zoneinfo"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

// SetTime set the current time and date,
// pass a value of microseconds since 1 Jan 1970 UTC
func (m *Manager) SetTime(sender dbus.Sender, usec int64, relative bool, message string) *dbus.Error {
	// TODO: check usec validity
	err := m.core.SetTime(0, usec, relative, false)
	return dbusutil.ToError(err)
}

// SetTimezone set the system time zone, the value from /usr/share/zoneinfo/zone.tab
func (m *Manager) SetTimezone(sender dbus.Sender, timezone, message string) *dbus.Error {
	ok, err := zoneinfo.IsZoneValid(timezone)
	if err != nil {
		return dbusutil.ToError(err)
	}
	if !ok {
		logger.Warning("invalid zone:", timezone)
		return dbusutil.ToError(zoneinfo.ErrZoneInvalid)
	}

	err = m.checkAuthorization("SetTimezone", message, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}

	currentTimezone, err := m.core.Timezone().Get(0)
	if err != nil {
		return dbusutil.ToError(err)
	}

	if currentTimezone == timezone {
		return nil
	}
	err = m.core.SetTimezone(0, timezone, false)
	return dbusutil.ToError(err)
}

// SetLocalRTC to control whether the RTC is the local time or UTC.
func (m *Manager) SetLocalRTC(sender dbus.Sender, enabled bool, fixSystem bool, message string) *dbus.Error {
	err := m.checkAuthorization("SetLocalRTC", message, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}

	currentLocalRTCEnabled, err := m.core.LocalRTC().Get(0)
	if err != nil {
		return dbusutil.ToError(err)
	}

	if currentLocalRTCEnabled == enabled {
		return nil
	}
	err = m.core.SetLocalRTC(0, enabled, fixSystem, false)
	return dbusutil.ToError(err)
}

// SetNTP to control whether the system clock is synchronized with the network
func (m *Manager) SetNTP(sender dbus.Sender, enabled bool, message string) *dbus.Error {
	currentNTPEnabled, err := m.core.NTP().Get(0)
	if err != nil {
		return dbusutil.ToError(err)
	}

	if currentNTPEnabled == enabled {
		return nil
	}

	err = m.core.SetNTP(0, enabled, false)
	if err != nil {
		return dbusutil.ToError(err)
	}

	// 关闭NTP时删除clock文件，使systemd以RTC时间为准
	if !enabled {
		err := os.Remove("/var/lib/systemd/timesync/clock")
		if err != nil {
			if !os.IsNotExist(err) {
				logger.Warning("delete [/var/lib/systemd/timesync/clock] err:", err)
			}
		}
	}

	return dbusutil.ToError(err)
}

func (m *Manager) SetNTPServer(sender dbus.Sender, server, message string) *dbus.Error {
	err := m.checkAuthorization("SetNTPServer", message, sender)
	if err != nil {
		return dbusutil.ToError(err)
	}

	err = m.setNTPServer(server)
	if err != nil {
		logger.Warning(err)
	}
	err = m.setDsgNTPServer(server)
	if err != nil {
		logger.Warning(err)
	}

	ntp, err := m.core.NTP().Get(0)
	if err != nil {
		logger.Warning(err)
	} else if ntp {
		// ntp enabled
		go func() {
			_, err := m.systemd.RestartUnit(0, timesyncdService, "replace")
			if err != nil {
				logger.Warning("failed to restart systemd timesyncd service:", err)
			}
		}()
	}
	return nil
}
