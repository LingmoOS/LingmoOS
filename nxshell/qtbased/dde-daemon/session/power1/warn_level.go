// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package power

type WarnLevel uint32

const (
	WarnLevelNone WarnLevel = iota
	WarnLevelRemind
	WarnLevelLow
	WarnLevelDanger
	WarnLevelCritical
	WarnLevelAction
)

func (lv WarnLevel) String() string {
	switch lv {
	case WarnLevelNone:
		return "None"
	case WarnLevelRemind:
		return "Remind"
	case WarnLevelLow:
		return "Low"
	case WarnLevelDanger:
		return "Danger"
	case WarnLevelCritical:
		return "Critical"
	case WarnLevelAction:
		return "Action"
	default:
		return "Unknown"
	}
}

func getWarnLevel(config *warnLevelConfig, onBattery bool,
	percentage float64, timeToEmpty uint64) WarnLevel { // 低电量的处理

	if !onBattery {
		return WarnLevelNone
	}

	usePercentageForPolicy := config.UsePercentageForPolicy
	logger.Debugf("_getWarnLevel onBattery %v, percentage %v, timeToEmpty %v, usePercentage %v",
		onBattery, percentage, timeToEmpty, usePercentageForPolicy)
	if usePercentageForPolicy {
		// 电源管理模块异常时获取到百分比会一直为0，此时设置告警等级为WarnLevelNone
		if percentage == 0.0 {
			return WarnLevelNone
		}
		// 当电池电量到达低电量阈值且达到系统固定低电量提醒值时，才去弹低电量提醒通知
		if percentage <= config.LowPowerNotifyThreshold {
			if percentage <= config.ActionPercentage {
				return WarnLevelAction
			}

			if percentage <= config.CriticalPercentage {
				return WarnLevelCritical
			}

			if percentage <= config.DangerPercentage {
				return WarnLevelDanger
			}

			if percentage <= config.LowPercentage {
				return WarnLevelLow
			}

			if percentage <= config.remindPercentage {
				return WarnLevelRemind
			}

			return WarnLevelNone
		}

		return WarnLevelNone
	} else {
		if timeToEmpty > config.LowTime || timeToEmpty == 0 {
			return WarnLevelNone
		}
		if timeToEmpty > config.DangerTime {
			return WarnLevelLow
		}
		if timeToEmpty > config.CriticalTime {
			return WarnLevelDanger
		}
		if timeToEmpty > config.ActionTime {
			return WarnLevelCritical
		}
		return WarnLevelAction
	}
}

func (m *Manager) getWarnLevel(percentage float64, timeToEmpty uint64) WarnLevel {
	return getWarnLevel(m.warnLevelConfig.getWarnLevelConfig(), m.OnBattery, percentage, timeToEmpty)
}
