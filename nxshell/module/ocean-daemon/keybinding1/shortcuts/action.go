// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

type ActionType uint

const (
	ActionTypeNonOp ActionType = iota
	ActionTypeExecCmd
	ActionTypeOpenMimeType
	ActionTypeDesktopFile
	ActionTypeShowNumLockOSD
	ActionTypeShowCapsLockOSD
	ActionTypeSystemShutdown
	ActionTypeSystemSuspend
	ActionTypeSystemLogOff
	ActionTypeSystemAway

	// controllers
	ActionTypeAudioCtrl
	ActionTypeMediaPlayerCtrl // MPRIS
	ActionTypeDisplayCtrl
	ActionTypeKbdLightCtrl
	ActionTypeTouchpadCtrl

	ActionTypeSwitchKbdLayout

	ActionTypeToggleWireless
	ActionTypeShowControlCenter

	ActionTypeCallback // 触发回调函数点Action

	// end
	actionTypeMax
)

const ActionTypeCount = int(actionTypeMax)

type Action struct {
	Type ActionType
	Arg  interface{}
}

var ActionNoOp = &Action{Type: ActionTypeNonOp}

// exec commandline
type ActionExecCmdArg struct {
	ExecOnRelease bool
	Cmd           string
}

func NewExecCmdAction(cmd string, execOnRelease bool) *Action {
	return &Action{
		Type: ActionTypeExecCmd,
		Arg: &ActionExecCmdArg{
			ExecOnRelease: execOnRelease,
			Cmd:           cmd,
		},
	}
}

// run the program which default handle mimeType
func NewOpenMimeTypeAction(mimeType string) *Action {
	return &Action{
		Type: ActionTypeOpenMimeType,
		Arg:  mimeType,
	}
}

type ActionCmd uint

const (
	// audio ctrl
	AudioSinkMuteToggle ActionCmd = iota + 1
	AudioSinkVolumeUp
	AudioSinkVolumeDown
	AudioSourceMuteToggle

	// media play ctrl
	MediaPlayerPlay
	MediaPlayerPause
	MediaPlayerStop
	MediaPlayerPrevious
	MediaPlayerNext
	MediaPlayerRewind
	MediaPlayerForword
	MediaPlayerRepeat

	// display ctrl
	MonitorBrightnessUp
	MonitorBrightnessDown
	DisplayModeSwitch
	AdjustBrightnessSwitch

	// keyboard backlight ctrl
	KbdLightToggle
	KbdLightBrightnessUp
	KbdLightBrightnessDown

	// touchpad ctrl
	TouchpadToggle
	TouchpadOn
	TouchpadOff
)

func NewAudioCtrlAction(cmd ActionCmd) *Action {
	return &Action{
		Type: ActionTypeAudioCtrl,
		Arg:  cmd,
	}
}

func NewMediaPlayerCtrlAction(cmd ActionCmd) *Action {
	return &Action{
		Type: ActionTypeMediaPlayerCtrl,
		Arg:  cmd,
	}
}

func NewDisplayCtrlAction(cmd ActionCmd) *Action {
	return &Action{
		Type: ActionTypeDisplayCtrl,
		Arg:  cmd,
	}
}

func NewKbdBrightnessCtrlAction(cmd ActionCmd) *Action {
	return &Action{
		Type: ActionTypeKbdLightCtrl,
		Arg:  cmd,
	}
}

func NewTouchpadCtrlAction(cmd ActionCmd) *Action {
	return &Action{
		Type: ActionTypeTouchpadCtrl,
		Arg:  cmd,
	}
}

func NewCallbackAction(fn func(ev *KeyEvent)) *Action {
	return &Action{
		Type: ActionTypeCallback,
		Arg:  fn,
	}
}
