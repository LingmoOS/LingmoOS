// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

// nolint
const (
	gsSchemaPower = "com.deepin.dde.power"
	// settingKeys
	settingKeyBatteryScreensaverDelay = "battery-screensaver-delay"
	settingKeyBatteryScreenBlackDelay = "battery-screen-black-delay"
	settingKeyBatterySleepDelay       = "battery-sleep-delay"
	settingKeyBatteryLockDelay        = "battery-lock-delay"

	settingKeyLinePowerScreensaverDelay = "line-power-screensaver-delay"
	settingKeyLinePowerScreenBlackDelay = "line-power-screen-black-delay"
	settingKeyLinePowerSleepDelay       = "line-power-sleep-delay"
	settingKeyLinePowerLockDelay        = "line-power-lock-delay"

	settingKeyAdjustBrightnessEnabled       = "adjust-brightness-enabled"
	settingKeyAmbientLightAdjuestBrightness = "ambient-light-adjust-brightness"
	settingKeyScreenBlackLock               = "screen-black-lock"
	settingKeySleepLock                     = "sleep-lock"

	settingKeyLinePowerLidClosedAction     = "line-power-lid-closed-action"
	settingKeyLinePowerPressPowerBtnAction = "line-power-press-power-button"
	settingKeyBatteryLidClosedAction       = "battery-lid-closed-action"
	settingKeyBatteryPressPowerBtnAction   = "battery-press-power-button"
	settingKeyLowPowerNotifyEnable         = "low-power-notify-enable"
	settingKeyLowPowerNotifyThreshold      = "low-power-notify-threshold"
	settingKeyLowPowerAutoSleepThreshold   = "percentage-action"
	settingKeyBrightnessDropPercent        = "brightness-drop-percent"
	settingKeyPowerSavingEnabled           = "power-saving-mode-enabled"

	settingKeyPowerButtonPressedExec = "power-button-pressed-exec"

	settingKeyFullScreenWorkaroundEnabled = "fullscreen-workaround-enabled"
	settingKeyUsePercentageForPolicy      = "use-percentage-for-policy"

	settingKeyLowPercentage      = "percentage-low"
	settingKeyDangerlPercentage  = "percentage-danger"
	settingKeyCriticalPercentage = "percentage-critical"
	settingKeyActionPercentage   = "percentage-action"

	settingKeyLowTime      = "time-to-empty-low"
	settingKeyDangerTime   = "time-to-empty-danger"
	settingKeyCriticalTime = "time-to-empty-critical"
	settingKeyActionTime   = "time-to-empty-action"

	settingKeySaveBrightnessWhilePsm = "save-brightness-while-psm"

	settingLightSensorEnabled = "light-sensor-enabled"
	settingKeyMode            = "mode"

	settingKeyHighPerformanceEnabled = "high-performance-enabled"

	// cmd
	cmdDDELowPower = "/usr/lib/deepin-daemon/dde-lowpower"

	batteryDisplay = "Display"
)

const (
	powerActionShutdown int32 = iota
	powerActionSuspend
	powerActionHibernate
	powerActionTurnOffScreen
	powerActionShowShutdownInterface
	powerActionDoNothing
)
