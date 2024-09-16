// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

import (
	"time"

	dbus "github.com/godbus/dbus/v5"
	gio "github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
	"github.com/linuxdeepin/go-lib/gsettings"
)

type warnLevelConfig struct {
	UsePercentageForPolicy  bool
	LowTime                 uint64
	DangerTime              uint64
	CriticalTime            uint64
	ActionTime              uint64
	LowPowerNotifyThreshold float64
	remindPercentage        float64
	LowPercentage           float64 // 废弃
	DangerPercentage        float64 // 废弃
	CriticalPercentage      float64 // 废弃
	ActionPercentage        float64 // 废弃
}

func (c *warnLevelConfig) isValid() bool {
	if c.LowTime > c.DangerTime &&
		c.DangerTime > c.CriticalTime &&
		c.CriticalTime > c.ActionTime &&
		c.remindPercentage > c.LowPercentage &&
		c.LowPercentage > c.DangerPercentage &&
		c.DangerPercentage > c.CriticalPercentage &&
		c.CriticalPercentage > c.ActionPercentage {
		return true
	}
	return false
}

type WarnLevelConfigManager struct {
	UsePercentageForPolicy gsprop.Bool `prop:"access:rw"`

	LowTime      gsprop.Int `prop:"access:rw"`
	DangerTime   gsprop.Int `prop:"access:rw"`
	CriticalTime gsprop.Int `prop:"access:rw"`
	ActionTime   gsprop.Int `prop:"access:rw"`

	LowPowerNotifyThreshold gsprop.Int `prop:"access:rw"`
	// LowPercentage、DangerPercentage、CriticalPercentage、ActionPercentage废弃
	// 这4个值不再提供可设置的方法
	LowPercentage      gsprop.Int `prop:"access:rw"` // 废弃
	DangerPercentage   gsprop.Int `prop:"access:rw"` // 废弃
	CriticalPercentage gsprop.Int `prop:"access:rw"` // 废弃
	ActionPercentage   gsprop.Int `prop:"access:rw"`

	settings    *gio.Settings
	changeTimer *time.Timer
	changeCb    func()
}

func NewWarnLevelConfigManager(gs *gio.Settings) *WarnLevelConfigManager {

	m := &WarnLevelConfigManager{
		settings: gs,
	}

	m.UsePercentageForPolicy.Bind(gs, settingKeyUsePercentageForPolicy)
	m.LowTime.Bind(gs, settingKeyLowTime)
	m.DangerTime.Bind(gs, settingKeyDangerTime)
	m.CriticalTime.Bind(gs, settingKeyCriticalTime)
	m.ActionTime.Bind(gs, settingKeyActionTime)

	m.LowPowerNotifyThreshold.Bind(gs, settingKeyLowPowerNotifyThreshold)
	m.LowPercentage.Bind(gs, settingKeyLowPercentage)           // 废弃
	m.DangerPercentage.Bind(gs, settingKeyDangerlPercentage)    // 废弃
	m.CriticalPercentage.Bind(gs, settingKeyCriticalPercentage) // 废弃
	m.ActionPercentage.Bind(gs, settingKeyActionPercentage)

	m.connectSettingsChanged()
	return m
}

func (m *WarnLevelConfigManager) getWarnLevelConfig() *warnLevelConfig {
	return &warnLevelConfig{
		UsePercentageForPolicy: m.UsePercentageForPolicy.Get(),
		LowTime:                uint64(m.LowTime.Get()),
		DangerTime:             uint64(m.DangerTime.Get()),
		CriticalTime:           uint64(m.CriticalTime.Get()),
		ActionTime:             uint64(m.ActionTime.Get()),

		LowPowerNotifyThreshold: float64(m.LowPowerNotifyThreshold.Get()),
		remindPercentage:        float64(25),
		LowPercentage:           float64(20),
		DangerPercentage:        float64(15),
		CriticalPercentage:      float64(10),
		ActionPercentage:        float64(m.ActionPercentage.Get()),
	}
}

func (m *WarnLevelConfigManager) setChangeCallback(fn func()) {
	m.changeCb = fn
}

func (m *WarnLevelConfigManager) delayCheckValid() {
	logger.Debug("delayCheckValid")
	if m.changeTimer != nil {
		m.changeTimer.Stop()
	}
	m.changeTimer = time.AfterFunc(20*time.Second, func() {
		logger.Debug("checkValid")
		wlc := m.getWarnLevelConfig()
		if !wlc.isValid() {
			logger.Info("Warn level config is invalid, reset")
			err := m.Reset()
			if err != nil {
				logger.Warning(err)
			}
		}
	})
}

func (m *WarnLevelConfigManager) notifyChange() {
	if m.changeCb != nil {
		logger.Debug("WarnLevelConfig change")
		m.changeCb()
	}
	m.delayCheckValid()
}

func (m *WarnLevelConfigManager) connectSettingsChanged() {
	gsettings.ConnectChanged(gsSchemaPower, "*", func(key string) {
		switch key {
		case settingKeyUsePercentageForPolicy,
			settingKeyLowPowerNotifyThreshold,
			settingKeyLowPercentage,      // 废弃
			settingKeyDangerlPercentage,  // 废弃
			settingKeyCriticalPercentage, // 废弃
			settingKeyActionPercentage,

			settingKeyLowTime,
			settingKeyDangerTime,
			settingKeyCriticalTime,
			settingKeyActionTime:

			logger.Debug("key changed", key)
			m.notifyChange()
		}
	})

}

func (m *WarnLevelConfigManager) Reset() *dbus.Error {
	s := m.settings
	s.Reset(settingKeyUsePercentageForPolicy)
	s.Reset(settingKeyLowPowerNotifyThreshold)
	s.Reset(settingKeyLowPercentage)      // 废弃
	s.Reset(settingKeyDangerlPercentage)  // 废弃
	s.Reset(settingKeyCriticalPercentage) // 废弃
	s.Reset(settingKeyActionPercentage)   // 废弃
	s.Reset(settingKeyLowTime)
	s.Reset(settingKeyDangerTime)
	s.Reset(settingKeyCriticalTime)
	s.Reset(settingKeyActionTime)
	return nil
}

func (*WarnLevelConfigManager) GetInterfaceName() string {
	return dbusInterface + ".WarnLevelConfig"
}
