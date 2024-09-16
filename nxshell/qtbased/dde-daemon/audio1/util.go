// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"bufio"
	"bytes"
	"encoding/json"
	"io/ioutil"
	"math"
	"strings"
	"unicode"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/soundutils"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.dbus"
	mpris2 "github.com/linuxdeepin/go-dbus-factory/session/org.mpris.mediaplayer2"
	//"github.com/linuxdeepin/go-lib/pulse"
)

func isVolumeValid(v float64) bool {
	if v < 0 || v > gMaxUIVolume {
		return false
	}
	return true
}

func playFeedback() {
	playFeedbackWithDevice("")
}

func playFeedbackWithDevice(device string) {
	go func() {
		err := soundutils.PlaySystemSound(soundutils.EventAudioVolumeChanged, device)
		if err != nil {
			logger.Warning(err)
		}
	}()
}

func toJSON(v interface{}) string {
	data, err := json.Marshal(v)
	if err != nil {
		return ""
	}
	return string(data)
}

const (
	mprisPlayerDestPrefix = "org.mpris.MediaPlayer2"
)

func getMprisPlayers(sessionConn *dbus.Conn) ([]string, error) {
	var playerNames []string
	dbusDaemon := ofdbus.NewDBus(sessionConn)
	names, err := dbusDaemon.ListNames(0)
	if err != nil {
		return nil, err
	}
	for _, name := range names {
		if strings.HasPrefix(name, mprisPlayerDestPrefix) {
			// is mpris player
			playerNames = append(playerNames, name)
		}
	}
	return playerNames, nil
}

func pauseAllPlayers() {
	sessionConn, err := dbus.SessionBus()
	if err != nil {
		return
	}
	playerNames, err := getMprisPlayers(sessionConn)
	if err != nil {
		logger.Warning("getMprisPlayers failed:", err)
		return
	}

	logger.Debug("pause all players")
	for _, playerName := range playerNames {
		player := mpris2.NewMediaPlayer(sessionConn, playerName)
		err := player.Player().Pause(0)
		if err != nil {
			logger.Warningf("failed to pause player %s: %v", playerName, err)
		}
	}
}

// 四舍五入
func floatPrecision(f float64) float64 {
	// 精确到小数点后2位
	pow10N := math.Pow10(2)
	return math.Trunc((f+0.5/pow10N)*pow10N) / pow10N
	// return math.Trunc((f)*pow10N) / pow10N
}

const defaultPaFile = "/etc/pulse/default.pa"

func loadDefaultPaConfig(filename string) (cfg defaultPaConfig) {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		logger.Warning(err)
		return
	}

	scanner := bufio.NewScanner(bytes.NewReader(content))
	for scanner.Scan() {
		line := scanner.Bytes()
		line = bytes.TrimLeftFunc(line, unicode.IsSpace)
		if bytes.HasPrefix(line, []byte{'#'}) {
			continue
		}

		if bytes.Contains(line, []byte("set-default-sink")) {
			cfg.setDefaultSink = true
		}
		if bytes.Contains(line, []byte("set-default-source")) {
			cfg.setDefaultSource = true
		}
	}
	err = scanner.Err()
	if err != nil {
		logger.Warning(err)
	}
	return
}

type defaultPaConfig struct {
	setDefaultSource bool
	setDefaultSink   bool
}
