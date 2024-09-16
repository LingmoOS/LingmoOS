// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package soundeffect

import (
	"bufio"
	"bytes"
	"errors"
	"os/exec"
	"sync"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/soundutils"
	soundthemeplayer "github.com/linuxdeepin/go-dbus-factory/system/com.deepin.api.soundthemeplayer"
	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
	"github.com/linuxdeepin/go-lib/strv"
)

//go:generate dbusutil-gen em -type Manager

const (
	gsSchemaSoundEffect = "com.deepin.dde.sound-effect"
	gsSchemaAppearance  = "com.deepin.dde.appearance"
	gsKeyEnabled        = "enabled"
	gsKeySoundTheme     = "sound-theme"

	DBusServiceName        = "org.deepin.dde.SoundEffect1"
	dbusPath               = "/org/deepin/dde/SoundEffect1"
	dbusInterface          = DBusServiceName
	allowPlaySoundMaxCount = 3
)

type Manager struct {
	service       *dbusutil.Service
	soundEffectGs *gio.Settings
	appearanceGs  *gio.Settings
	count         int
	countMu       sync.Mutex
	names         strv.Strv

	Enabled gsprop.Bool `prop:"access:rw"`
}

func NewManager(service *dbusutil.Service) *Manager {
	var m = new(Manager)

	m.service = service
	m.soundEffectGs = gio.NewSettings(gsSchemaSoundEffect)
	m.appearanceGs = gio.NewSettings(gsSchemaAppearance)
	return m
}

func (m *Manager) init() error {
	m.Enabled.Bind(m.soundEffectGs, gsKeyEnabled)
	var err error
	m.names, err = getSoundNames()
	if err != nil {
		return err
	}
	logger.Debug(m.names)
	return nil
}

func getSoundNames() ([]string, error) {
	var result []string
	out, err := exec.Command("gsettings", "list-recursively", gsSchemaSoundEffect).Output()
	if err != nil {
		return nil, err
	}
	scanner := bufio.NewScanner(bytes.NewReader(out))
	for scanner.Scan() {
		parts := bytes.Fields(scanner.Bytes())
		if len(parts) == 3 {
			if bytes.Equal(parts[1], []byte("enabled")) {
				// skip key enabled
				continue
			}
			key := string(parts[1])
			value := parts[2]
			if bytes.Equal(value, []byte("true")) || bytes.Equal(value, []byte("false")) {
				result = append(result, key)
			}
		}
	}
	if scanner.Err() != nil {
		return nil, scanner.Err()
	}

	return result, nil
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

// deprecated
func (m *Manager) PlaySystemSound(name string) *dbus.Error {
	return m.PlaySound(name)
}

func (m *Manager) PlaySound(name string) *dbus.Error {
	m.service.DelayAutoQuit()

	if name == "" {
		return nil
	}

	go func() {
		m.countMu.Lock()
		m.count++
		logger.Debug("start", m.count)
		m.countMu.Unlock()

		if m.count <= allowPlaySoundMaxCount {
			err := soundutils.PlaySystemSound(name, "")
			if err != nil {
				logger.Error(err)
			}
		} else {
			logger.Warning("PlaySystemSound thread more than 3")
		}

		m.countMu.Lock()
		logger.Debug("end", m.count)
		m.count--
		m.countMu.Unlock()
	}()
	return nil
}

// deprecated
func (m *Manager) GetSystemSoundFile(name string) (file string, busErr *dbus.Error) {
	return m.GetSoundFile(name)
}

func (m *Manager) GetSoundFile(name string) (file string, busErr *dbus.Error) {
	m.service.DelayAutoQuit()

	file = soundutils.GetSystemSoundFile(name)
	if file == "" {
		return "", dbusutil.ToError(errors.New("sound file not found"))
	}

	return file, nil
}

func (m *Manager) canQuit() bool {
	m.countMu.Lock()
	count := m.count
	m.countMu.Unlock()
	return count == 0
}

func (m *Manager) syncConfigToSoundThemePlayer(name string, enabled bool) error {
	logger.Debug("sync config to sound-theme-player")
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	player := soundthemeplayer.NewSoundThemePlayer(sysBus)
	err = player.EnableSound(0, name, enabled)
	if err != nil {
		return err
	}
	theme := m.appearanceGs.GetString(gsKeySoundTheme)
	err = player.SetSoundTheme(0, theme)
	return err
}

func (m *Manager) EnableSound(name string, enabled bool) *dbus.Error {
	if !m.names.Contains(name) {
		return dbusutil.ToError(errors.New("invalid sound event"))
	}

	if name == soundutils.EventDesktopLogin ||
		name == soundutils.EventSystemShutdown {
		err := m.syncConfigToSoundThemePlayer(name, enabled)
		if err != nil {
			return dbusutil.ToError(err)
		}
	}

	m.soundEffectGs.SetBoolean(name, enabled)
	return nil
}

func (m *Manager) IsSoundEnabled(name string) (enabled bool, busErr *dbus.Error) {
	if !m.names.Contains(name) {
		return false, dbusutil.ToError(errors.New("invalid sound event"))
	}

	return m.soundEffectGs.GetBoolean(name), nil
}

func (m *Manager) GetSoundEnabledMap() (result map[string]bool, busErr *dbus.Error) {
	result = make(map[string]bool, len(m.names))
	for _, name := range m.names {
		result[name] = m.soundEffectGs.GetBoolean(name)
	}
	return result, nil
}

func (m *Manager) enabledWriteCb(write *dbusutil.PropertyWrite) *dbus.Error {
	enabled := write.Value.(bool)

	// NOTE: 已经约定 name 为空表示控制总开关
	err := m.syncConfigToSoundThemePlayer("", enabled)
	if err != nil {
		logger.Warning(err)
	}
	return nil
}
