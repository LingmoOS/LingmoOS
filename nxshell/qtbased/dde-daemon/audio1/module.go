// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"time"

	"github.com/linuxdeepin/dde-daemon/loader"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"golang.org/x/xerrors"
)

var (
	logger = log.NewLogger("daemon/audio")
)

func init() {
	loader.Register(NewModule(logger))
}

type Module struct {
	*loader.ModuleBase
	audio *Audio
}

func NewModule(logger *log.Logger) *Module {
	var d = new(Module)
	d.ModuleBase = loader.NewModuleBase("audio", d, logger)
	return d
}

func (*Module) GetDependencies() []string {
	return []string{}
}

func (m *Module) start() error {
	service := loader.GetService()

	err := startAudioServer(service) // 为了保证蓝牙模块依赖audio模块,并且audio模块启动音频服务完成.
	if err != nil {
		err = xerrors.Errorf("failed to start pulseaudio: %w", err)
		return err
	}

	m.audio = newAudio(service)
	err = m.audio.init()
	if err != nil {
		logger.Warning("failed to init audio module:", err)
		return nil
	}

	err = service.Export(dbusPath, m.audio, m.audio.syncConfig)
	if err != nil {
		return err
	}
	err = service.RequestName(dbusServiceName)
	if err != nil {
		return err
	}

	so := service.GetServerObject(m.audio)
	err = so.SetWriteCallback(m.audio, "ReduceNoise", m.audio.writeReduceNoise)

	if err != nil {
		logger.Warning("failed to bind callback for ReduceNoise:", err)
	}

	err = so.SetWriteCallback(m.audio, "PausePlayer", m.audio.writeKeyPausePlayer)

	if err != nil {
		logger.Warning("failed to bind callback for PausePlayer:", err)
	}

	err = m.audio.syncConfig.Register()
	if err != nil {
		logger.Warning("failed to register for deepin sync:", err)
	}
	return nil
}

func (m *Module) Start() error {
	if m.audio != nil {
		return nil
	}
	waitSoundThemePlayerExit()
	err := m.start()
	if err != nil {
		logger.Warning(err)
	}
	return nil
}

func (m *Module) Stop() error {
	if m.audio == nil {
		return nil
	}

	m.audio.destroy()

	service := loader.GetService()
	err := service.StopExport(m.audio)
	if err != nil {
		logger.Warning(err)
	}

	err = service.ReleaseName(dbusServiceName)
	if err != nil {
		logger.Warning(err)
	}

	m.audio = nil
	return nil
}

func waitSoundThemePlayerExit() {
	srv, err := dbusutil.NewSystemService()
	if err != nil {
		logger.Warning("Failed to connect system bus:", err)
		return
	}

	for {
		var owner string
		owner, err = srv.GetNameOwner("com.deepin.api.SoundThemePlayer")
		if err != nil {
			return
		}
		logger.Info("Owner:", owner)
		time.Sleep(time.Millisecond * 800)
	}
}
