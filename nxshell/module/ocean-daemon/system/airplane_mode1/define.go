// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package airplane_mode

// RadioAction radio action use rfkill to operate radio
type RadioAction int

const (
	NoneRadioAction RadioAction = iota
	BlockRadioAction
	UnblockRadioAction
	ListRadioAction
	MonitorRadioAction
)

// ToRfkillState convert to rfkill action
func (action RadioAction) ToRfkillState() rfkillState {
	var ac rfkillState
	switch action {
	case UnblockRadioAction:
		ac = rfkillStateUnblock
	case BlockRadioAction:
		ac = rfkillStateBlock
	}
	return ac
}

// String action
func (action RadioAction) String() string {
	var name string
	switch action {
	case BlockRadioAction:
		name = "block"
	case UnblockRadioAction:
		name = "unblock"
	case ListRadioAction:
		name = "list"
	case MonitorRadioAction:
		name = "event"
	}
	return name
}
