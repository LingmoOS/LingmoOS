// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package keybinding

import (
	"errors"
	"strings"

	dbus "github.com/godbus/dbus/v5"
	. "github.com/linuxdeepin/dde-daemon/keybinding1/shortcuts"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.dbus"
	mpris2 "github.com/linuxdeepin/go-dbus-factory/session/org.mpris.mediaplayer2"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/proxy"
)

const (
	senderTypeMpris = "org.mpris.MediaPlayer2"

	playerDelta int64 = 5000 * 1000 // 5s
)

type MediaPlayerController struct {
	conn         *dbus.Conn
	prevPlayer   string
	dbusDaemon   ofdbus.DBus
	loginManager login1.Manager
}

func NewMediaPlayerController(systemSigLoop *dbusutil.SignalLoop,
	sessionConn *dbus.Conn) *MediaPlayerController {

	c := new(MediaPlayerController)
	c.conn = sessionConn
	c.dbusDaemon = ofdbus.NewDBus(sessionConn)
	c.loginManager = login1.NewManager(systemSigLoop.Conn())
	c.loginManager.InitSignalExt(systemSigLoop, true)

	// move to power module
	// pause all player before system sleep
	// c.loginManager.ConnectPrepareForSleep(func(start bool) {
	// 	if !start {
	// 		return
	// 	}
	// 	c.pauseAllPlayer()
	// })
	return c
}

func (c *MediaPlayerController) Destroy() {
	c.loginManager.RemoveHandler(proxy.RemoveAllHandlers)
}

func (c *MediaPlayerController) Name() string {
	return "Media Player"
}

func (c *MediaPlayerController) ExecCmd(cmd ActionCmd) error {
	player := c.getActiveMpris()
	if player == nil {
		return errors.New("no player found")
	}

	logger.Debug("[HandlerAction] active player dest name:", player.ServiceName_())
	switch cmd {
	case MediaPlayerPlay:
		return player.Player().PlayPause(0)
	case MediaPlayerPause:
		return player.Player().Pause(0)
	case MediaPlayerStop:
		return player.Player().Stop(0)

	case MediaPlayerPrevious:
		if err := player.Player().Previous(0); err != nil {
			return err
		}
		return player.Player().Play(0)

	case MediaPlayerNext:
		if err := player.Player().Next(0); err != nil {
			return err
		}
		return player.Player().Play(0)

	case MediaPlayerRewind:
		pos, err := player.Player().Position().Get(0)
		if err != nil {
			return err
		}

		var offset int64
		if pos-playerDelta > 0 {
			offset = -playerDelta
		}

		if err := player.Player().Seek(0, offset); err != nil {
			return err
		}
		status, err := player.Player().PlaybackStatus().Get(0)
		if err != nil {
			return err
		}
		if status != "Playing" {
			return player.Player().PlayPause(0)
		}
	case MediaPlayerForword:
		if err := player.Player().Seek(0, playerDelta); err != nil {
			return err
		}
		status, err := player.Player().PlaybackStatus().Get(0)
		if err != nil {
			return err
		}
		if status != "Playing" {
			return player.Player().PlayPause(0)
		}
	case MediaPlayerRepeat:
		return player.Player().Play(0)

	default:
		return ErrInvalidActionCmd{cmd}
	}
	return nil
}

func (c *MediaPlayerController) getMprisSender() []string {
	if c.dbusDaemon == nil {
		return nil
	}
	var senders []string
	names, _ := c.dbusDaemon.ListNames(0)
	for _, name := range names {
		if strings.HasPrefix(name, senderTypeMpris) {
			senders = append(senders, name)
		}
	}

	return senders
}

func (c *MediaPlayerController) getActiveMpris() mpris2.MediaPlayer {
	var senders = c.getMprisSender()

	length := len(senders)
	if length == 0 {
		return nil
	}

	for _, sender := range senders {
		player := mpris2.NewMediaPlayer(c.conn, sender)

		if length == 1 {
			return player
		}

		if length == 2 && strings.Contains(sender, "vlc") {
			return player
		}

		status, err := player.Player().PlaybackStatus().Get(0)
		if err != nil {
			logger.Warning(err)
			continue
		}

		if status == "Playing" {
			c.prevPlayer = sender
			return player
		}

		if c.prevPlayer == sender {
			return player
		}
	}

	player := mpris2.NewMediaPlayer(c.conn, senders[0])
	return player
}
