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

type Sink struct {
	audio   *Audio
	service *dbusutil.Service
	PropsMu sync.RWMutex
	index   uint32

	Name        string
	Description string

	// 默认音量值
	BaseVolume float64

	// 是否静音
	Mute bool

	// 当前音量
	Volume     float64
	cVolume    pulse.CVolume
	channelMap pulse.ChannelMap
	// 左右声道平衡值
	Balance float64
	// 是否支持左右声道调整
	SupportBalance bool
	// 前后声道平衡值
	Fade float64
	// 是否支持前后声道调整
	SupportFade bool

	// dbusutil-gen: equal=portsEqual
	// 支持的输出端口
	Ports []Port
	// 当前使用的输出端口
	ActivePort Port
	// 声卡的索引
	Card uint32

	props map[string]string

	// 当前是否是可插拔sink
	pluggable bool
}

func newSink(sinkInfo *pulse.Sink, audio *Audio) *Sink {
	s := &Sink{
		audio:   audio,
		service: audio.service,
		index:   sinkInfo.Index,
		props:   sinkInfo.PropList,
	}
	s.update(sinkInfo)
	return s
}

// 检测端口是否被禁用
func (s *Sink) CheckPort() *dbus.Error {
	enabled, err := s.audio.IsPortEnabled(s.Card, s.ActivePort.Name)
	if err != nil {
		return err
	}

	if !enabled {
		return dbusutil.ToError(fmt.Errorf("port<%d:%s> is disabled", s.Card, s.ActivePort.Name))
	}

	return nil
}

// 设置音量大小
//
// v: 音量大小
//
// isPlay: 是否播放声音反馈
func (s *Sink) SetVolume(value float64, isPlay bool) *dbus.Error {
	logger.Infof("dbus call SetVolume with value %f and isPlay %t, the sink name is %s", value, isPlay, s.Name)

	err := s.CheckPort()
	if err != nil {
		logger.Warning(err.Body...)
		return err
	}

	if !isVolumeValid(value) {
		err1 := fmt.Errorf("invalid volume value: %v", value)
		logger.Warning(err1)
		return dbusutil.ToError(err1)
	}

	if value == 0 {
		value = 0.001
		s.SetMute(true)
	}
	s.PropsMu.Lock()
	cv := s.cVolume.SetAvg(value)
	s.PropsMu.Unlock()
	s.audio.context().SetSinkVolumeByIndex(s.index, cv)

	GetConfigKeeper().SetVolume(s.audio.getCardNameById(s.Card), s.ActivePort.Name, value)

	if isPlay {
		s.playFeedback()
	}
	return nil
}

// 设置左右声道平衡值
//
// v: 声道平衡值
//
// isPlay: 是否播放声音反馈
func (s *Sink) SetBalance(value float64, isPlay bool) *dbus.Error {
	logger.Infof("dbus call SetBalance with value %f and isPlay %t, the sink name is %s", value, isPlay, s.Name)

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
	s.audio.context().SetSinkVolumeByIndex(s.index, cv)

	GetConfigKeeper().SetBalance(s.audio.getCardNameById(s.Card), s.ActivePort.Name, value)

	if isPlay {
		s.playFeedback()
	}
	return nil
}

// 设置前后声道平衡值
//
// v: 声道平衡值
//
// isPlay: 是否播放声音反馈
func (s *Sink) SetFade(value float64) *dbus.Error {
	logger.Infof("dbus call SetFade with value %f, the sink name is %s", value, s.Name)

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
	s.audio.context().SetSinkVolumeByIndex(s.index, cv)
	s.playFeedback()
	return nil
}

// 设置同时设置音量和平衡
//
// v: volume音量值
// b: balance左右平衡值
// f: fade前后平衡值
func (s *Sink) setVBF(v, b, f float64) *dbus.Error {
	if !isVolumeValid(v) {
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
	s.audio.context().SetSinkVolumeByIndex(s.index, cv)

	return nil
}

// 是否静音
func (s *Sink) SetMute(value bool) *dbus.Error {
	logger.Infof("dbus call SetMute with value %t, the sink name is %s", value, s.Name)

	err := s.CheckPort()
	if err != nil {
		logger.Warning(err.Body...)
		return err
	}

	s.audio.context().SetSinkMuteByIndex(s.index, value)
	GetConfigKeeper().SetMuteOutput(value)

	if !value {
		s.playFeedback()
	}
	return nil
}

// 设置此设备的当前使用端口
func (s *Sink) SetPort(name string) *dbus.Error {
	logger.Infof("dbus call SetPort with name %s, the sink name is %s", name, s.Name)

	s.audio.context().SetSinkPortByIndex(s.index, name)
	return nil
}

func (s *Sink) getPath() dbus.ObjectPath {
	return dbus.ObjectPath(dbusPath + "/Sink" + strconv.Itoa(int(s.index)))
}

func (*Sink) GetInterfaceName() string {
	return dbusInterface + ".Sink"
}

func (s *Sink) update(sinkInfo *pulse.Sink) {
	s.PropsMu.Lock()

	s.Name = sinkInfo.Name
	s.Description = sinkInfo.Description
	s.Card = sinkInfo.Card
	s.BaseVolume = sinkInfo.BaseVolume.ToPercent()
	s.cVolume = sinkInfo.Volume
	s.channelMap = sinkInfo.ChannelMap

	s.setPropMute(sinkInfo.Mute)
	s.setPropVolume(floatPrecision(sinkInfo.Volume.Avg()))

	s.setPropSupportFade(false)
	s.setPropFade(sinkInfo.Volume.Fade(sinkInfo.ChannelMap))
	s.setPropSupportBalance(true)
	s.setPropBalance(sinkInfo.Volume.Balance(sinkInfo.ChannelMap))

	newActivePort := toPort(sinkInfo.ActivePort)
	var activePortChanged bool

	s.setPropPorts(toPorts(sinkInfo.Ports))
	activePortChanged = s.setPropActivePort(newActivePort)

	s.props = sinkInfo.PropList
	s.PropsMu.Unlock()

	if activePortChanged && s.audio.defaultSinkName == s.Name {
		logger.Debugf("default sink update active port %s", sinkInfo.ActivePort.Name)
		s.audio.resumeSinkConfig(s)
	}
}

func (s *Sink) GetMeter() (meter dbus.ObjectPath, busErr *dbus.Error) {
	//TODO
	return "/", nil
}

func (s *Sink) playFeedback() {
	s.PropsMu.RLock()
	name := s.Name
	s.PropsMu.RUnlock()
	playFeedbackWithDevice(name)
}

func (s *Sink) setMute(v bool) {
	logger.Debugf("Sink #%d setMute %v", s.index, v)
	s.audio.context().SetSinkMuteByIndex(s.index, v)
}
