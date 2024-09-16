// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"fmt"
	"strconv"
	"sync"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/pulse"
)

type Source struct {
	audio       *Audio
	service     *dbusutil.Service
	PropsMu     sync.RWMutex
	index       uint32
	cVolume     pulse.CVolume
	channelMap  pulse.ChannelMap
	Name        string
	Description string
	// 默认的输入音量
	BaseVolume     float64
	Mute           bool
	Volume         float64
	Balance        float64
	SupportBalance bool
	Fade           float64
	SupportFade    bool
	// dbusutil-gen: equal=portsEqual
	Ports      []Port
	ActivePort Port
	// 声卡的索引
	Card uint32
}

func newSource(sourceInfo *pulse.Source, audio *Audio) *Source {
	s := &Source{
		audio:   audio,
		index:   sourceInfo.Index,
		service: audio.service,
	}
	if !isPhysicalDevice(sourceInfo.Name) {
		masterSourceInfo := audio.getSourceInfoByName(sourceInfo.Proplist["device.master_device"])
		if masterSourceInfo == nil {
			logger.Warningf("cannot get master source for %s", sourceInfo.Name)
		} else {
			sourceInfo.Card = masterSourceInfo.Card
			sourceInfo.Ports = masterSourceInfo.Ports
			sourceInfo.ActivePort = masterSourceInfo.ActivePort
			logger.Debugf("create reducing noise source on %s", masterSourceInfo.Name)
		}
	}

	s.update(sourceInfo)
	return s
}

// 检测端口是否被禁用
func (s *Source) CheckPort() *dbus.Error {
	enabled, err := s.audio.IsPortEnabled(s.Card, s.ActivePort.Name)
	if err != nil {
		return err
	}

	if !enabled {
		return dbusutil.ToError(fmt.Errorf("port<%d:%s> is disabled", s.Card, s.ActivePort.Name))
	}

	return nil
}

// 如何反馈输入音量？
func (s *Source) SetVolume(value float64, isPlay bool) *dbus.Error {
	logger.Infof("dbus call SetVolume with value %f and isPlay %t, the source name is %s",
		value, isPlay, s.Name)

	err := s.CheckPort()
	if err != nil {
		logger.Warning(err.Body...)
		return err
	}

	if !isVolumeValid(value) {
		err1 := fmt.Errorf("invalid volume value: %v", value)
		logger.Warning(err.Body...)
		return dbusutil.ToError(err1)
	}

	if value == 0 {
		value = 0.001
	}
	s.PropsMu.RLock()
	cv := s.cVolume.SetAvg(value)
	s.PropsMu.RUnlock()
	s.audio.context().SetSourceVolumeByIndex(s.index, cv)

	GetConfigKeeper().SetVolume(s.audio.getCardNameById(s.Card), s.ActivePort.Name, value)

	if isPlay {
		playFeedback()
	}
	return nil
}

func (s *Source) SetBalance(value float64, isPlay bool) *dbus.Error {
	logger.Infof("dbus call SetBalance with value %f and isPlay %t, the source name is %s",
		value, isPlay, s.Name)

	err := s.CheckPort()
	if err != nil {
		logger.Warning(err.Body...)
		return err
	}

	if value < -1.00 || value > 1.00 {
		err1 := fmt.Errorf("invalid volume value: %v", value)
		logger.Warning(err1)
		return dbusutil.ToError(err1)
	}

	s.PropsMu.RLock()
	cv := s.cVolume.SetBalance(s.channelMap, value)
	s.PropsMu.RUnlock()
	s.audio.context().SetSourceVolumeByIndex(s.index, cv)

	GetConfigKeeper().SetBalance(s.audio.getCardNameById(s.Card), s.ActivePort.Name, value)

	if isPlay {
		playFeedback()
	}
	return nil
}

func (s *Source) SetFade(value float64) *dbus.Error {
	logger.Infof("dbus call SetFade with value %f, the source name is %s", value, s.Name)

	err := s.CheckPort()
	if err != nil {
		logger.Warning(err.Body...)
		return err
	}

	if value < -1.00 || value > 1.00 {
		err1 := fmt.Errorf("invalid volume value: %v", value)
		logger.Warning(err1)
		return dbusutil.ToError(err1)
	}

	s.PropsMu.RLock()
	cv := s.cVolume.SetFade(s.channelMap, value)
	s.PropsMu.RUnlock()
	s.audio.context().SetSourceVolumeByIndex(s.index, cv)

	playFeedback()
	return nil
}

// 设置同时设置音量和平衡
//
// v: volume音量值
// b: balance左右平衡值
// f: fade前后平衡值
func (s *Source) setVBF(v, b, f float64) *dbus.Error {
	if v < -1.00 || v > 1.00 {
		return dbusutil.ToError(fmt.Errorf("invalid volume value: %v", v))
	}

	if b < -1.00 || b > 1.00 {
		return dbusutil.ToError(fmt.Errorf("invalid balance value: %v", b))
	}

	if f < -1.00 || b > 1.00 {
		return dbusutil.ToError(fmt.Errorf("invalid fade value: %v", f))
	}

	s.PropsMu.RLock()
	cv := s.cVolume.SetAvg(v)
	cv = cv.SetBalance(s.channelMap, b)
	cv = cv.SetFade(s.channelMap, f)
	s.PropsMu.RUnlock()
	s.audio.context().SetSourceVolumeByIndex(s.index, cv)

	return nil
}

func (s *Source) SetMute(value bool) *dbus.Error {
	logger.Infof("dbus call SetMute with value %t, the source name is %s", value, s.Name)

	err := s.CheckPort()
	if err != nil {
		logger.Warning(err.Body...)
		return err
	}

	s.audio.context().SetSourceMuteByIndex(s.index, value)
	GetConfigKeeper().SetMuteInput(value)

	if !value {
		playFeedback()
	}
	return nil
}

func (s *Source) SetPort(name string) *dbus.Error {
	logger.Infof("dbus call SetPort with name %s, the source name is %s", name, s.Name)

	s.audio.context().SetSourcePortByIndex(s.index, name)
	return nil
}

func (s *Source) GetMeter() (meter dbus.ObjectPath, busErr *dbus.Error) {
	logger.Infof("dbus call GetMeter, the source name is %s", s.Name)

	id := fmt.Sprintf("source%d", s.index)
	s.audio.mu.Lock()
	m, ok := s.audio.meters[id]
	s.audio.mu.Unlock()
	if ok {
		return m.getPath(), nil
	}

	sourceMeter := pulse.NewSourceMeter(s.audio.ctx, s.index)
	m = newMeter(id, sourceMeter, s.audio)
	meterPath := m.getPath()
	err := s.service.Export(meterPath, m)
	if err != nil {
		logger.Warning(err)
		return "/", dbusutil.ToError(err)
	}

	s.audio.mu.Lock()
	s.audio.meters[id] = m
	s.audio.mu.Unlock()

	m.core.ConnectChanged(func(v float64) {
		m.PropsMu.Lock()
		m.setPropVolume(v)
		m.PropsMu.Unlock()
	})
	return meterPath, nil
}

func (s *Source) getPath() dbus.ObjectPath {
	return dbus.ObjectPath(dbusPath + "/Source" + strconv.Itoa(int(s.index)))
}

func (*Source) GetInterfaceName() string {
	return dbusInterface + ".Source"
}

func (s *Source) update(sourceInfo *pulse.Source) {
	// 如果是虚拟通道，则将card和ports等设为对应主通道的值，这是为了能够正常使用降噪的训通道
	if !isPhysicalDevice(sourceInfo.Name) {
		masterSourceInfo := s.audio.getSourceInfoByName(sourceInfo.Proplist["device.master_device"])
		if masterSourceInfo == nil {
			logger.Warningf("cannot get master source for %s", sourceInfo.Name)
		} else {
			sourceInfo.Card = masterSourceInfo.Card
			sourceInfo.Ports = masterSourceInfo.Ports
			sourceInfo.ActivePort = masterSourceInfo.ActivePort
			logger.Debugf("create reducing noise source on %s", masterSourceInfo.Name)
		}
	}

	s.PropsMu.Lock()
	s.cVolume = sourceInfo.Volume
	s.channelMap = sourceInfo.ChannelMap
	s.Name = sourceInfo.Name
	s.Description = sourceInfo.Description
	s.Card = sourceInfo.Card
	s.BaseVolume = sourceInfo.BaseVolume.ToPercent()

	s.setPropVolume(floatPrecision(sourceInfo.Volume.Avg()))
	s.setPropMute(sourceInfo.Mute)

	//TODO: handle this
	s.setPropSupportFade(false)
	s.setPropFade(sourceInfo.Volume.Fade(sourceInfo.ChannelMap))
	s.setPropSupportBalance(true)
	s.setPropBalance(sourceInfo.Volume.Balance(sourceInfo.ChannelMap))

	var ports []Port
	for _, p := range sourceInfo.Ports {
		ports = append(ports, toPort(p))
	}

	s.setPropPorts(ports)
	activePortChanged := s.setPropActivePort(toPort(sourceInfo.ActivePort))

	s.PropsMu.Unlock()

	if activePortChanged && s.audio.defaultSourceName == s.Name {
		logger.Debugf("default source update active port %s", sourceInfo.ActivePort.Name)
		s.audio.resumeSourceConfig(s, isPhysicalDevice(s.Name))
	}
}

func (s *Source) setMute(v bool) {
	logger.Debugf("Source #%d setMute %v", s.index, v)
	s.audio.context().SetSourceMuteByIndex(s.index, v)
}
