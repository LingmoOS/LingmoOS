// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"errors"
	"strings"

	"github.com/godbus/dbus/v5"
	. "github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	audio "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.audio1"
	backlight "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.backlighthelper1"
	"github.com/linuxdeepin/go-gir/gio-2.0"
)

const (
	volumeMin = 0
	volumeMax = 1.5
)

const (
	gsKeyOsdAdjustVolState = "osd-adjust-volume-enabled"
)

type OsdVolumeState int32

// Osd音量调节控制
const (
	VolumeAdjustEnable OsdVolumeState = iota
	VolumeAdjustForbidden
	VolumeAdjustHidden
)

type AudioController struct {
	conn                   *dbus.Conn
	audioDaemon            audio.Audio
	huaweiMicLedWorkaround *huaweiMicLedWorkaround
	gsKeyboard             *gio.Settings
}

func NewAudioController(sessionConn *dbus.Conn,
	backlightHelper backlight.Backlight) *AudioController {
	c := &AudioController{
		conn:        sessionConn,
		audioDaemon: audio.NewAudio(sessionConn),
	}
	c.initHuaweiMicLedWorkaround(backlightHelper)
	c.gsKeyboard = gio.NewSettings(gsSchemaKeyboard)
	return c
}

func (c *AudioController) Destroy() {
	if c.huaweiMicLedWorkaround != nil {
		c.huaweiMicLedWorkaround.destroy()
		c.huaweiMicLedWorkaround = nil
	}
}

func (*AudioController) Name() string {
	return "Audio"
}

func (c *AudioController) ExecCmd(cmd ActionCmd) error {
	switch cmd {
	case AudioSinkMuteToggle:
		return c.toggleSinkMute()

	case AudioSinkVolumeUp:
		return c.changeSinkVolume(true)

	case AudioSinkVolumeDown:
		return c.changeSinkVolume(false)

	case AudioSourceMuteToggle:
		return c.toggleSourceMute()

	default:
		return ErrInvalidActionCmd{cmd}
	}
}

func (c *AudioController) toggleSinkMute() error {
	var osd string
	var state = OsdVolumeState(c.gsKeyboard.GetEnum(gsKeyOsdAdjustVolState))

	// 当OsdAdjustVolumeState的值为VolumeAdjustEnable时，才会去执行静音操作
	if VolumeAdjustEnable == state {
		sink, err := c.getDefaultSink()
		if err != nil {
			return err
		}

		mute, err := sink.Mute().Get(0)
		if err != nil {
			return err
		}

		err = sink.SetMute(0, !mute)
		if err != nil {
			return err
		}
		osd = "AudioMute"
	} else if VolumeAdjustForbidden == state {
		osd = "AudioMuteAsh"
	} else {
		return nil
	}

	showOSD(osd)
	return nil
}

func (c *AudioController) toggleSourceMute() error {
	var osd string
	var state = OsdVolumeState(c.gsKeyboard.GetEnum(gsKeyOsdAdjustVolState))

	source, err := c.getDefaultSource()
	if err != nil {
		return err
	}

	mute, err := source.Mute().Get(0)
	if err != nil {
		return err
	}
	mute = !mute

	// 当OsdAdjustVolumeState的值为VolumeAdjustEnable时，才会去执行静音操作
	if VolumeAdjustEnable == state {
		err = source.SetMute(0, mute)
		if err != nil {
			return err
		}

		if mute {
			osd = "AudioMicMuteOn"
		} else {
			osd = "AudioMicMuteOff"
		}
	} else if VolumeAdjustForbidden == state {
		if mute {
			osd = "AudioMicMuteOnAsh"
		} else {
			osd = "AudioMicMuteOffAsh"
		}
	} else {
		return nil
	}

	showOSD(osd)
	return nil
}

func (c *AudioController) changeSinkVolume(raised bool) error {
	var osd string
	var state = OsdVolumeState(c.gsKeyboard.GetEnum(gsKeyOsdAdjustVolState))

	// 当OsdAdjustVolumeState的值为VolumeAdjustEnable时，才会去执行调节音量的操作
	if VolumeAdjustEnable == state {
		sink, err := c.getDefaultSink()
		if err != nil {
			return err
		}

		osd = "AudioUp"
		v, err := sink.Volume().Get(0)
		if err != nil {
			return err
		}

		var step = 0.05
		if !raised {
			step = -step
			osd = "AudioDown"
		}

		maxVolume, err := c.audioDaemon.MaxUIVolume().Get(0)
		if err != nil {
			logger.Warning(err)
			maxVolume = volumeMax
		}

		v += step
		if v < volumeMin {
			v = volumeMin
		} else if v > maxVolume {
			v = maxVolume
		}

		logger.Debug("[changeSinkVolume] will set volume to:", v)
		mute, err := sink.Mute().Get(0)
		if err != nil {
			return err
		}

		if mute {
			err = sink.SetMute(0, false)
			if err != nil {
				logger.Warning(err)
			}
		}

		err = sink.SetVolume(0, v, true)
		if err != nil {
			return err
		}
	} else if VolumeAdjustForbidden == state {
		if raised {
			osd = "AudioUpAsh"
		} else {
			osd = "AudioDownAsh"
		}
	} else {
		return nil
	}

	showOSD(osd)
	return nil
}

func (c *AudioController) getDefaultSink() (audio.Sink, error) {
	sinkPath, err := c.audioDaemon.DefaultSink().Get(0)
	if err != nil {
		return nil, err
	}

	sink, err := audio.NewSink(c.conn, sinkPath)
	if err != nil {
		return nil, err
	}
	name, err := sink.Name().Get(0)
	if err != nil {
		return nil, err
	}
	ports, err := sink.Ports().Get(0)
	if err != nil {
		return nil, err
	}
	if len(ports) == 0 && strings.Contains(name, "auto_null") {
		return nil, errors.New("default sink (auto_null) is invalid")
	}

	return sink, nil
}

func (c *AudioController) getDefaultSource() (audio.Source, error) {
	sourcePath, err := c.audioDaemon.DefaultSource().Get(0)
	if err != nil {
		return nil, err
	}
	source, err := audio.NewSource(c.conn, sourcePath)
	if err != nil {
		return nil, err
	}
	name, err := source.Name().Get(0)
	if err != nil {
		return nil, err
	}
	ports, err := source.Ports().Get(0)
	if err != nil {
		return nil, err
	}
	if len(ports) == 0 && strings.Contains(name, "auto_null") {
		return nil, errors.New("default source (auto_null) is invalid")
	}

	return source, nil
}
