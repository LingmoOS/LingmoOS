// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

import "github.com/linuxdeepin/go-lib/gsettings"

func (m *Manager) listenPropChanged() {
	m.td.InitSignalExt(m.systemSigLoop, true)
	err := m.td.CanNTP().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		logger.Debug("property CanTTP changed to", value)

		m.PropsMu.Lock()
		m.setPropCanNTP(value)
		m.PropsMu.Unlock()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = m.td.NTP().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		logger.Debug("property NTP changed to", value)

		m.PropsMu.Lock()
		m.setPropNTP(value)
		m.PropsMu.Unlock()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = m.td.LocalRTC().ConnectChanged(func(hasValue bool, value bool) {
		if !hasValue {
			return
		}
		logger.Debug("property LocalRTC changed to", value)

		m.PropsMu.Lock()
		m.setPropLocalRTC(value)
		m.PropsMu.Unlock()
	})
	if err != nil {
		logger.Warning(err)
	}

	err = m.td.Timezone().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}
		logger.Debug("property Timezone changed to", value)
		m.PropsMu.Lock()
		m.setPropTimezone(value)
		m.PropsMu.Unlock()

		err := m.AddUserTimezone(m.Timezone)
		if err != nil {
			logger.Warning("AddUserTimezone error:", err)
		}
	})
	if err != nil {
		logger.Warning(err)
	}

	m.setter.InitSignalExt(m.systemSigLoop, true)
	err = m.setter.NTPServer().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}

		m.PropsMu.Lock()
		m.setPropNTPServer(value)
		m.PropsMu.Unlock()
	})
	if err != nil {
		logger.Warning(err)
	}
}

func (m *Manager) handleGSettingsChanged() {
	gsettings.ConnectChanged(timeDateSchema, settingsKey24Hour, func(key string) {
		value := m.settings.GetBoolean(settingsKey24Hour)
		err := m.userObj.SetUse24HourFormat(0, value)
		if err != nil {
			logger.Warning(err)
		}
	})

	gsettings.ConnectChanged(timeDateSchema, settingsKeyWeekdayFormat, func(key string) {
		value := m.settings.GetInt(settingsKeyWeekdayFormat)
		err := m.userObj.SetWeekdayFormat(0, value)
		if err != nil {
			logger.Warning(err)
		}
	})

	gsettings.ConnectChanged(timeDateSchema, settingsKeyShortDateFormat, func(key string) {
		value := m.settings.GetInt(settingsKeyShortDateFormat)
		err := m.userObj.SetShortDateFormat(0, value)
		if err != nil {
			logger.Warning(err)
		}
	})

	gsettings.ConnectChanged(timeDateSchema, settingsKeyLongDateFormat, func(key string) {
		value := m.settings.GetInt(settingsKeyLongDateFormat)
		err := m.userObj.SetLongDateFormat(0, value)
		if err != nil {
			logger.Warning(err)
		}
	})

	gsettings.ConnectChanged(timeDateSchema, settingsKeyShortTimeFormat, func(key string) {
		value := m.settings.GetInt(settingsKeyShortTimeFormat)
		err := m.userObj.SetShortTimeFormat(0, value)
		if err != nil {
			logger.Warning(err)
		}
	})

	gsettings.ConnectChanged(timeDateSchema, settingsKeyLongTimeFormat, func(key string) {
		value := m.settings.GetInt(settingsKeyLongTimeFormat)
		err := m.userObj.SetLongTimeFormat(0, value)
		if err != nil {
			logger.Warning(err)
		}
	})
	gsettings.ConnectChanged(timeDateSchema, settingsKeyWeekBegins, func(key string) {
		value := m.settings.GetInt(settingsKeyWeekBegins)
		err := m.userObj.SetWeekBegins(0, value)
		if err != nil {
			logger.Warning(err)
		}
	})
}
