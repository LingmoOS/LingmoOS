// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"encoding/json"
	"io/ioutil"
	"path/filepath"

	"github.com/linuxdeepin/go-lib/pulse"
	"github.com/linuxdeepin/go-lib/xdg/basedir"
)

// dconfig默认优先级
var (
	inputDefaultPriorities  = []int{}
	outputDefaultPriorities = []int{}
)

// 优先级策略组，包含输入和输出
type PriorityManager struct {
	Output *PriorityPolicy
	Input  *PriorityPolicy

	file string // 配置文件的路径，私有成员不会被json导出
}

// 创建优先级组
func NewPriorityManager(path string) *PriorityManager {
	return &PriorityManager{
		Output: NewPriorityPolicy(),
		Input:  NewPriorityPolicy(),
		file:   path,
	}
}

// 创建单例
func createPriorityManagerSingleton(path string) func() *PriorityManager {
	var pm *PriorityManager = nil
	return func() *PriorityManager {
		if pm == nil {
			pm = NewPriorityManager(path)
		}
		return pm
	}
}

// 获取单例
// 由于优先级管理需要在很多个对象中使用，放在Audio对象中需要添加额外参数传递到各个模块很不方便，因此在此创建一个全局的单例
var globalPrioritiesFilePath = filepath.Join(basedir.GetUserConfigDir(), "deepin/dde-daemon/priorities.json")
var GetPriorityManager = createPriorityManagerSingleton(globalPrioritiesFilePath)

// 打印优先级列表，用于调试
func (pm *PriorityManager) Print() {
	data, err := json.MarshalIndent(pm, "", "  ")
	if err != nil {
		logger.Warning(err)
		return
	}

	logger.Debugf("PriorityManager:\n %s", string(data))
}

// 保存配置文件
func (pm *PriorityManager) Save() {
	data, err := json.MarshalIndent(pm, "", "  ")
	if err != nil {
		logger.Warning(err)
		return
	}

	err = ioutil.WriteFile(pm.file, data, 0644)
	if err != nil {
		logger.Warning(err)
		return
	}
}

// 读取配置文件
func (pm *PriorityManager) Load() bool {
	data, err := ioutil.ReadFile(pm.file)
	if err != nil {
		logger.Warningf("failed to read file '%s': %v", pm.file, err)
		return false
	}

	err = json.Unmarshal(data, pm)
	if err != nil {
		logger.Warningf("failed to parse json of file '%s': %v", pm.file, err)
		return false
	}

	return true
}

// 加载dconfig默认优先级
func (pm *PriorityManager) completeDefaultTypes() {
	for _, i := range inputDefaultPriorities {
		if !pm.Input.Types.hasElement(i) {
			pm.Input.Types = append(pm.Input.Types, i)
			logger.Debugf("input defualt append type %d", i)
		}
	}
	for _, i := range outputDefaultPriorities {
		if !pm.Output.Types.hasElement(i) {
			pm.Output.Types = append(pm.Output.Types, i)
			logger.Debugf("output defualt append type %d", i)
		}
	}
}

// 使用默认值进行初始化
func (pm *PriorityManager) defaultInit(cards CardList) {
	// 初始化类型优先级列表
	pm.completeDefaultTypes()
	pm.Output.completeTypes()
	pm.Input.completeTypes()

	// 添加可用的端口
	for _, card := range cards {
		for _, port := range card.Ports {
			if port.Available == pulse.AvailableTypeNo {
				logger.Debugf("unavailable port '%s(%s)' card:<%s> port:<%s>", port.Description, card.Name, card.core.Name, port.Name)
				continue
			}

			if port.Direction == pulse.DirectionSink {
				pm.Output.AddRawPort(card.core, &port)
			} else if port.Direction == pulse.DirectionSource {
				pm.Input.AddRawPort(card.core, &port)
			} else {
				logger.Warningf("unexpected direction %d of port <%s:%s>",
					port.Direction, card.core.Name, port.Name)
			}
		}
	}
}

// 设置有效端口
// 这是因为从配置文件读出来的数据是上次运行时保存的，两次运行之间（例如关机状态下）可能插拔了设备
// 因此需要删除无效端口，添加新的有效端口
func (pm *PriorityManager) SetPorts(cards CardList) {
	outputPorts := make(PriorityPortList, 0)
	inputPorts := make(PriorityPortList, 0)
	for _, card := range cards {
		for _, port := range card.Ports {
			if port.Available == pulse.AvailableTypeNo {
				logger.Debugf("unavailable port '%s(%s)' card:<%s> port:<%s>", port.Description, card.Name, card.core.Name, port.Name)
				continue
			}

			_, portConfig := GetConfigKeeper().GetCardAndPortConfig(card.core.Name, port.Name)
			if !portConfig.Enabled {
				logger.Debugf("diabled port '%s(%s)' card:<%s> port:<%s>", port.Description, card.Name, card.core.Name, port.Name)
				continue
			}

			p := PriorityPort{
				CardName: card.core.Name,
				PortName: port.Name,
				PortType: DetectPortType(card.core, &port),
			}

			if port.Direction == pulse.DirectionSink {
				outputPorts = append(outputPorts, &p)
				logger.Debugf("append output port %s:%s", p.CardName, p.PortName)
			} else {
				inputPorts = append(inputPorts, &p)
				logger.Debugf("append input port %s:%s", p.CardName, p.PortName)
			}
		}
	}
	pm.Output.SetPorts(outputPorts)
	pm.Input.SetPorts(inputPorts)
}

// 进行初始化
func (pm *PriorityManager) Init(cards CardList) {
	if !pm.Load() {
		pm.defaultInit(cards)
		pm.Save()
		return
	}

	// 设置有效端口
	pm.SetPorts(cards)

	// 补充缺少的类型（产品修改了端口类型），并更新端口排序
	pm.completeDefaultTypes()
	pm.Output.completeTypes()
	pm.Output.sortPorts()
	pm.Input.completeTypes()
	pm.Input.sortPorts()

	pm.Save()
}

// 设置优先级最高的端口（自动识别输入输出）
func (pm *PriorityManager) SetTheFirstPort(card *pulse.Card, port *pulse.CardPortInfo) {
	if port.Direction == pulse.DirectionSink {
		pm.Output.SetTheFirstPort(card.Name, port.Name)
	} else if port.Direction == pulse.DirectionSource {
		pm.Input.SetTheFirstPort(card.Name, port.Name)
	} else {
		logger.Warningf("unexpected direction %d of port <%s:%s>",
			port.Direction, card.Name, port.Name)
	}

	// 打印并保存
	pm.Print()
	pm.Save()
}

// 设置优先级最高的输出端口
// 注意：cardName和portName有好几种，不要传错了
// 应当使用 pulse.Card.Name 和 pulse.CardPortInfo.Name
// 形似："alsa_card.pci-0000_00_1f.3" 和 "hdmi-output-0"
// 而不是: "HDA Intel PCH" 和 "HDMI / DisplayPort"
func (pm *PriorityManager) SetFirstOutputPort(cardName string, portName string) {
	pm.Output.SetTheFirstPort(cardName, portName)
	pm.Print()
	pm.Save()
}

// 设置优先级最高的输入端口
// 注意：cardName和portName有好几种，不要传错了
// 应当使用 pulse.Card.Name 和 pulse.CardPortInfo.Name
// 形似："alsa_card.pci-0000_00_1f.3" 和 "hdmi-output-0"
// 而不是: "HDA Intel PCH" 和 "HDMI / DisplayPort"
func (pm *PriorityManager) SetFirstInputPort(cardName string, portName string) {
	pm.Input.SetTheFirstPort(cardName, portName)
	pm.Print()
	pm.Save()
}
