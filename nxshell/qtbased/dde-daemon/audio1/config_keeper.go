// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"encoding/json"
	"io/ioutil"
	"path/filepath"
	"strings"
	"sync"

	"github.com/linuxdeepin/go-lib/xdg/basedir"
)

type PortConfig struct {
	Name           string
	Enabled        bool
	Volume         float64
	IncreaseVolume bool
	Balance        float64
	ReduceNoise    bool
	Mute           bool // 静音改为全局，此配置废弃
}

type CardConfig struct {
	Name  string
	Ports map[string]*PortConfig // Name => PortConfig
}

type MuteConfig struct {
	MuteOutput bool
	MuteInput  bool
}

type ConfigKeeper struct {
	Cards    map[string]*CardConfig // Name => CardConfig
	Mute     *MuteConfig            // 全局静音
	file     string                 // 配置文件路径
	muteFile string                 // 静音配置文件路径
	mu       sync.Mutex             //
}

// 创建单例
func createConfigKeeperSingleton(path string, mutePath string) func() *ConfigKeeper {
	var ck *ConfigKeeper = nil
	return func() *ConfigKeeper {
		if ck == nil {
			ck = NewConfigKeeper(path, mutePath)
		}
		return ck
	}
}

// 获取单例
// 由于优先级管理需要在很多个对象中使用，放在Audio对象中需要添加额外参数传递到各个模块很不方便，因此在此创建一个全局的单例
var globalConfigKeeperFile = filepath.Join(basedir.GetUserConfigDir(), "deepin/dde-daemon/audio-config-keeper.json")
var globalConfigKeeperMuteFile = filepath.Join(basedir.GetUserConfigDir(), "deepin/dde-daemon/audio-config-keeper-mute.json")
var GetConfigKeeper = createConfigKeeperSingleton(globalConfigKeeperFile, globalConfigKeeperMuteFile)

func NewConfigKeeper(path string, mutePath string) *ConfigKeeper {
	return &ConfigKeeper{
		Cards:    make(map[string]*CardConfig),
		Mute:     NewMuteConfig(),
		file:     path,
		muteFile: mutePath,
	}
}

func NewMuteConfig() *MuteConfig {
	return &MuteConfig{
		MuteOutput: false,
		MuteInput:  false,
	}
}

func NewCardConfig(name string) *CardConfig {
	return &CardConfig{
		Name:  name,
		Ports: make(map[string]*PortConfig),
	}
}

func NewPortConfig(name string) *PortConfig {
	pname := strings.ToLower(name)
	volume := 0.5
	if strings.Contains(pname, "input") {
		volume = defaultInputVolume
	} else if strings.Contains(pname, "output") {
		if strings.Contains(pname, "headphone") || strings.Contains(pname, "headset") {
			volume = defaultHeadphoneOutputVolume
		} else {
			volume = defaultOutputVolume
		}

	}
	return &PortConfig{
		Name:           name,
		Enabled:        true,
		Volume:         volume,
		IncreaseVolume: false,
		Balance:        0.0,
		ReduceNoise:    defaultReduceNoise,
		Mute:           false,
	}
}

func (ck *ConfigKeeper) Save() error {
	data, err := json.MarshalIndent(ck.Cards, "", "  ")
	if err != nil {
		logger.Warning(err)
		return err
	}

	err = ioutil.WriteFile(ck.file, data, 0644)
	if err != nil {
		logger.Warning(err)
		// 这里不返回，因为可能可以写另一个配置
	}

	data, err = json.MarshalIndent(ck.Mute, "", "  ")
	if err != nil {
		logger.Warning(err)
		return err
	}

	err = ioutil.WriteFile(ck.muteFile, data, 0644)
	if err != nil {
		logger.Warning(err)
		return err
	}

	return nil
}

func (ck *ConfigKeeper) Load() error {
	data, err := ioutil.ReadFile(ck.file)
	if err != nil {
		logger.Warning(err)
		return err
	}

	err = json.Unmarshal(data, &ck.Cards)
	if err != nil {
		logger.Warning(err)
	}

	data, err = ioutil.ReadFile(ck.muteFile)
	if err != nil {
		logger.Warning(err)
		return err
	}

	err = json.Unmarshal(data, &ck.Mute)
	if err != nil {
		logger.Warning(err)
		return err
	}

	return nil
}

func (ck *ConfigKeeper) Print() {
	data, err := json.MarshalIndent(ck.Cards, "", "  ")
	if err != nil {
		logger.Warning(err)
		return
	}
	logger.Debug(string(data))
}

func (ck *ConfigKeeper) UpdateCardConfig(cardConfig *CardConfig) {
	ck.Cards[cardConfig.Name] = cardConfig
}

func (ck *ConfigKeeper) RemoveCardConfig(cardName string) {
	delete(ck.Cards, cardName)
}

func (ck *ConfigKeeper) GetCardAndPortConfig(cardName string, portName string) (*CardConfig, *PortConfig) {
	card, ok := ck.Cards[cardName]
	if !ok {
		card = NewCardConfig(cardName)
		port := NewPortConfig(portName)
		card.UpdatePortConfig(port)
		ck.UpdateCardConfig(card)
		return card, port
	}

	port, ok := card.Ports[portName]
	if !ok {
		port = NewPortConfig(portName)
		card.UpdatePortConfig(port)
		ck.UpdateCardConfig(card)
	}
	return card, port
}

func (ck *ConfigKeeper) SetEnabled(cardName string, portName string, enabled bool) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	_, port := ck.GetCardAndPortConfig(cardName, portName)
	port.Enabled = enabled
	ck.Save()
}

func (ck *ConfigKeeper) SetVolume(cardName string, portName string, volume float64) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	_, port := ck.GetCardAndPortConfig(cardName, portName)
	port.Volume = volume
	ck.Save()
}

func (ck *ConfigKeeper) SetIncreaseVolume(cardName string, portName string, enhance bool) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	_, port := ck.GetCardAndPortConfig(cardName, portName)
	port.IncreaseVolume = enhance
	ck.Save()
}

func (ck *ConfigKeeper) SetBalance(cardName string, portName string, balance float64) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	_, port := ck.GetCardAndPortConfig(cardName, portName)
	port.Balance = balance
	ck.Save()
}

func (ck *ConfigKeeper) SetReduceNoise(cardName string, portName string, reduce bool) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	_, port := ck.GetCardAndPortConfig(cardName, portName)
	port.ReduceNoise = reduce
	ck.Save()
}

func (ck *ConfigKeeper) SetMuteOutput(mute bool) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	ck.Mute.MuteOutput = mute
	ck.Save()
}

func (ck *ConfigKeeper) SetMuteInput(mute bool) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	ck.Mute.MuteInput = mute
	ck.Save()
}

func (ck *ConfigKeeper) SetMuteAll(mute bool) {
	ck.mu.Lock()
	defer ck.mu.Unlock()

	for _, card := range ck.Cards {
		for _, port := range card.Ports {
			port.Mute = mute
		}
	}
	ck.Save()
}

func (card *CardConfig) UpdatePortConfig(portConfig *PortConfig) {
	card.Ports[portConfig.Name] = portConfig
}

func (card *CardConfig) RemovePortConfig(portName string) {
	delete(card.Ports, portName)
}
