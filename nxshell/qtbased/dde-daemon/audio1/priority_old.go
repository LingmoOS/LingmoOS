// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"encoding/json"
	"io/ioutil"
	"strings"

	"github.com/linuxdeepin/go-lib/pulse"
)

// const (
// 	PortTypeBluetooth    = iota // 蓝牙音频
// 	PortTypeHeadset             // USB和3.5mm 耳麦
// 	PortTypeBuiltin             // 内置扬声器和话筒
// 	PortTypeHdmi                // HDMI
// 	PortTypeLineIO              // 线缆输入输出
// 	PortTypeMultiChannel        // 多声道
// 	PortTypeUnknown             // 其他类型

// 	PortTypeCount // 类型数量
// )

type PortToken struct {
	CardName string
	PortName string
}

type Priorities struct {
	OutputTypePriority     []int
	OutputInstancePriority []*PortToken
	InputTypePriority      []int
	InputInstancePriority  []*PortToken
}

type Skipper func(cardName string, portName string) bool

var (
	priorities = NewPriorities()
)

func hasElement(slice []int, value int) bool {
	for _, v := range slice {
		if value == v {
			return true
		}
	}

	return false
}

func contains(cardName string, portName string, substr string) bool {
	return strings.Contains(strings.ToLower(cardName), substr) ||
		strings.Contains(strings.ToLower(portName), substr)
}

func GetPortType(cardName string, portName string) int {
	if contains(cardName, portName, "multichannel") {
		return PortTypeMultiChannel
	}

	if contains(cardName, portName, "bluez") ||
		contains(cardName, portName, "bluetooth") {
		return PortTypeBluetooth
	}

	if contains(cardName, portName, "linein") ||
		contains(cardName, portName, "lineout") {
		return PortTypeLineIO
	}

	if contains(cardName, portName, "usb") ||
		contains(cardName, portName, "rear-mic") ||
		contains(cardName, portName, "front-mic") ||
		contains(cardName, portName, "headset") ||
		contains(cardName, portName, "headphone") {
		return PortTypeHeadset
	}

	if contains(cardName, portName, "hdmi") {
		return PortTypeHdmi
	}

	if contains(cardName, portName, "speaker") ||
		contains(cardName, portName, "input-mic") {
		return PortTypeBuiltin
	}

	return PortTypeUnknown
}

// 图标类型 扬声器 > 耳机 > HDMI > 蓝牙
func GetIconPortType(cardName string, portName string) int {

	if contains(cardName, portName, "linein") ||
		contains(cardName, portName, "lineout") {
		return PortTypeLineIO
	}

	if contains(cardName, portName, "speaker") ||
		contains(cardName, portName, "input-mic") {
		return PortTypeBuiltin
	}

	if contains(cardName, portName, "rear-mic") ||
		contains(cardName, portName, "front-mic") ||
		contains(cardName, portName, "headset") ||
		contains(cardName, portName, "headphone") {
		return PortTypeHeadset
	}

	if contains(cardName, portName, "hdmi") {
		return PortTypeHdmi
	}

	if contains(cardName, portName, "bluez") ||
		contains(cardName, portName, "bluetooth") {
		return PortTypeBluetooth
	}

	if contains(cardName, portName, "usb") {
		return PortTypeUsb
	}

	return PortTypeUnknown
}

func NewPriorities() *Priorities {
	return &Priorities{
		OutputTypePriority:     make([]int, 0),
		OutputInstancePriority: make([]*PortToken, 0),
		InputTypePriority:      make([]int, 0),
		InputInstancePriority:  make([]*PortToken, 0),
	}
}

func (pr *Priorities) Save(file string) error {
	data, err := json.MarshalIndent(pr, "", "  ")
	if err != nil {
		return err
	}

	return ioutil.WriteFile(file, data, 0644)
}

func (pr *Priorities) Print() {
	data, err := json.MarshalIndent(pr, "", "  ")
	if err != nil {
		logger.Warning(err)
	}

	logger.Debug(string(data))
}

func (pr *Priorities) Load(file string, cards CardList) {
	data, err := ioutil.ReadFile(file)
	if err != nil {
		logger.Warning(err)
		pr.defaultInit(cards)
		return
	}
	err = json.Unmarshal(data, pr)
	if err != nil {
		logger.Warning(err)
		pr.defaultInit(cards)
		return
	}
	pr.completeTypes()
	pr.RemoveUnavailable(cards)
	pr.AddAvailable(cards)
	pr.sortOutput()
	pr.sortInput()
}

// 读取配置文件获得的类型优先级中类型的数量少于PortTypeCount时
// 将缺少的类型补充完整
// 通常发生在增加了新的类型的时候
func (pr *Priorities) completeTypes() {
	for i := 0; i < PortTypeCount; i++ {
		if !hasElement(pr.OutputTypePriority, i) {
			pr.OutputTypePriority = append(pr.OutputTypePriority, i)
		}
	}

	for i := 0; i < PortTypeCount; i++ {
		if !hasElement(pr.InputTypePriority, i) {
			pr.InputTypePriority = append(pr.InputTypePriority, i)
		}
	}
}

// 调整输出端口实例的优先级
// 当修改端口类型时，某些端口的优先级需要进行调整
// 例如原先将 多声道 视为 内置扬声器
// 现在将 多声道 单独分为一类
// 从旧的配置文件里读出来的多声道实例的优先级就不合理了
// 此时通过类型优先级对实例优先级进行排序
// 为了避免原先类型优先级相同的端口在排序时被打乱
// 不能使用快速排序，这里使用稳定的冒泡排序
func (pr *Priorities) sortOutput() {
	length := len(pr.OutputInstancePriority)
	for i := 0; i+1 < length; i++ {
		for j := i; j+1 < length; j++ {
			port1 := pr.OutputInstancePriority[j]
			port2 := pr.OutputInstancePriority[j+1]
			type1 := GetPortType(port1.CardName, port1.PortName)
			type2 := GetPortType(port2.CardName, port2.PortName)

			// type1优先级在type2后面，交换
			if pr.IsOutputTypeAfter(type2, type1) {
				pr.OutputInstancePriority[j], pr.OutputInstancePriority[j+1] = pr.OutputInstancePriority[j+1], pr.OutputInstancePriority[j]
			}
		}
	}
}

// 调整输入端口实例的优先级
func (pr *Priorities) sortInput() {
	length := len(pr.InputInstancePriority)
	for i := 0; i+1 < length; i++ {
		for j := i; j+1 < length; j++ {
			port1 := pr.InputInstancePriority[j]
			port2 := pr.InputInstancePriority[j+1]
			type1 := GetPortType(port1.CardName, port1.PortName)
			type2 := GetPortType(port2.CardName, port2.PortName)

			// type1优先级在type2后面，交换
			if pr.IsInputTypeAfter(type2, type1) {
				pr.InputInstancePriority[j], pr.InputInstancePriority[j+1] = pr.InputInstancePriority[j+1], pr.InputInstancePriority[j]
			}
		}
	}
}

func (pr *Priorities) RemoveUnavailable(cards CardList) {
	for i := 0; i < len(pr.InputInstancePriority); {
		portToken := pr.InputInstancePriority[i]
		if !pr.checkAvailable(cards, portToken.CardName, portToken.PortName) {
			logger.Debugf("remove input port %s %s", portToken.CardName, portToken.PortName)
			pr.removeInput(i)
		} else {
			i++
		}
	}

	for i := 0; i < len(pr.OutputInstancePriority); {
		portToken := pr.OutputInstancePriority[i]
		if !pr.checkAvailable(cards, portToken.CardName, portToken.PortName) {
			logger.Debugf("remove output port %s %s", portToken.CardName, portToken.PortName)
			pr.removeOutput(i)
		} else {
			i++
		}
	}
}

func (pr *Priorities) AddAvailable(cards CardList) {
	for _, card := range cards {

		logger.Debugf("+++++++++++++++++++++++++++++++++++ %v", card.Ports)
		for _, port := range card.Ports {
			if port.Available == pulse.AvailableTypeNo {
				logger.Debugf("unavailable port %s %s", card.core.Name, port.Name)
				continue
			}

			_, portConfig := GetConfigKeeper().GetCardAndPortConfig(card.core.Name, port.Name)
			if !portConfig.Enabled {
				logger.Debugf("disabled port %s %s", card.core.Name, port.Name)
				continue
			}

			if port.Direction == pulse.DirectionSink && pr.findOutput(card.core.Name, port.Name) < 0 {
				logger.Debugf("add output port %s %s", card.core.Name, port.Name)
				pr.AddOutputPort(card.core.Name, port.Name)
			} else if port.Direction == pulse.DirectionSource && pr.findInput(card.core.Name, port.Name) < 0 {
				logger.Debugf("add input port %s %s", card.core.Name, port.Name)
				pr.AddInputPort(card.core.Name, port.Name)
			}
		}
	}
}

func (pr *Priorities) AddInputPort(cardName string, portName string) {
	portType := GetPortType(cardName, portName)
	token := PortToken{cardName, portName}
	for i := 0; i < len(pr.InputInstancePriority); i++ {
		p := pr.InputInstancePriority[i]
		t := GetPortType(p.CardName, p.PortName)
		if t == portType || pr.IsInputTypeAfter(portType, t) {
			pr.insertInput(i, &token)
			return
		}
	}

	pr.InputInstancePriority = append(pr.InputInstancePriority, &token)
}

func (pr *Priorities) AddOutputPort(cardName string, portName string) {
	portType := GetPortType(cardName, portName)
	token := PortToken{cardName, portName}
	for i := 0; i < len(pr.OutputInstancePriority); i++ {
		p := pr.OutputInstancePriority[i]
		t := GetPortType(p.CardName, p.PortName)
		if t == portType || pr.IsOutputTypeAfter(portType, t) {
			pr.insertOutput(i, &token)
			return
		}
	}

	pr.OutputInstancePriority = append(pr.OutputInstancePriority, &token)
}

func (pr *Priorities) RemoveCard(cardName string) {
	for i := 0; i < len(pr.InputInstancePriority); {
		p := pr.InputInstancePriority[i]
		if p.CardName == cardName {
			pr.removeInput(i)
		} else {
			i++
		}
	}

	for i := 0; i < len(pr.OutputInstancePriority); {
		p := pr.OutputInstancePriority[i]
		if p.CardName == cardName {
			pr.removeOutput(i)
		} else {
			i++
		}
	}
}

func (pr *Priorities) RemoveInputPort(cardName string, portName string) {
	index := pr.findInput(cardName, portName)
	if index >= 0 {
		pr.removeInput(index)
	}
}

func (pr *Priorities) RemoveOutputPort(cardName string, portName string) {
	index := pr.findOutput(cardName, portName)
	if index >= 0 {
		pr.removeOutput(index)
	}
}

func (pr *Priorities) SetInputPortFirst(cardName string, portName string) {
	portType := GetPortType(cardName, portName)
	token := PortToken{cardName, portName}
	index := pr.findInput(cardName, portName)
	if index < 0 {
		return
	}
	pr.removeInput(index)
	pr.insertInput(0, &token)
	pr.setInputTypeFirst(portType)

	index = 1
	for i := 1; i < len(pr.InputInstancePriority); i++ {
		p := pr.InputInstancePriority[i]
		t := GetPortType(p.CardName, p.PortName)
		if t == portType {
			pr.removeInput(i)
			pr.insertInput(index, p)
			index++
		}
	}
}

func (pr *Priorities) SetOutputPortFirst(cardName string, portName string) {
	portType := GetPortType(cardName, portName)
	token := PortToken{cardName, portName}
	index := pr.findOutput(cardName, portName)
	if index < 0 {
		return
	}
	pr.removeOutput(index)
	pr.insertOutput(0, &token)
	pr.setOutputTypeFirst(portType)

	index = 1
	for i := 1; i < len(pr.OutputInstancePriority); i++ {
		p := pr.OutputInstancePriority[i]
		t := GetPortType(p.CardName, p.PortName)
		if t == portType {
			pr.removeOutput(i)
			pr.insertOutput(index, p)
			index++
		}
	}
}

func (pr *Priorities) GetFirstInput() (string, string) {
	if len(pr.InputInstancePriority) > 0 {
		port := pr.InputInstancePriority[0]
		return port.CardName, port.PortName
	} else {
		return "", ""
	}
}

func (pr *Priorities) GetFirstInputSkip(skipper Skipper) (string, string) {
	for _, port := range pr.InputInstancePriority {
		if !skipper(port.CardName, port.PortName) {
			logger.Debugf("select %s %s", port.CardName, port.PortName)
			return port.CardName, port.PortName
		}
		logger.Debugf("skip %s %s", port.CardName, port.PortName)
	}
	return "", ""
}

func (pr *Priorities) GetFirstOutput() (string, string) {
	if len(pr.OutputInstancePriority) > 0 {
		port := pr.OutputInstancePriority[0]
		return port.CardName, port.PortName
	} else {
		return "", ""
	}
}

func (pr *Priorities) GetFirstOutputSkip(skipper Skipper) (string, string) {
	for _, port := range pr.OutputInstancePriority {
		if !skipper(port.CardName, port.PortName) {
			logger.Debugf("select %s %s", port.CardName, port.PortName)
			return port.CardName, port.PortName
		}
		logger.Debugf("skip %s %s", port.CardName, port.PortName)
	}
	return "", ""
}

// 这个判断的是 type2 是否在 type1 之后
func (pr *Priorities) IsInputTypeAfter(type1 int, type2 int) bool {
	for _, t := range pr.InputTypePriority {
		if t == type1 {
			return true
		}

		if t == type2 {
			return false
		}
	}

	return false
}

// 这个判断的是 type2 是否在 type1 之后
func (pr *Priorities) IsOutputTypeAfter(type1 int, type2 int) bool {
	for _, t := range pr.OutputTypePriority {
		if t == type1 {
			return true
		}

		if t == type2 {
			return false
		}
	}

	return false
}

func (pr *Priorities) defaultInit(cards CardList) {
	for t := 0; t < PortTypeCount; t++ {
		pr.OutputTypePriority = append(pr.OutputTypePriority, t)
		pr.InputTypePriority = append(pr.InputTypePriority, t)
	}

	pr.AddAvailable(cards)
}

func (pr *Priorities) checkAvailable(cards CardList, cardName string, portName string) bool {
	for _, card := range cards {
		if cardName != card.core.Name {
			continue
		}
		for _, port := range card.Ports {
			if portName != port.Name {
				continue
			}

			if port.Available == pulse.AvailableTypeYes {
				_, portConfig := GetConfigKeeper().GetCardAndPortConfig(cardName, portName)
				return portConfig.Enabled
			} else if port.Available == pulse.AvailableTypeUnknow {
				logger.Warningf("port(%s %s) available is unknown", cardName, portName)
				_, portConfig := GetConfigKeeper().GetCardAndPortConfig(cardName, portName)
				return portConfig.Enabled
			} else {
				return false
			}
		}
	}

	return false
}

func (pr *Priorities) removeInput(index int) {
	pr.InputInstancePriority = append(
		pr.InputInstancePriority[:index],
		pr.InputInstancePriority[index+1:]...,
	)
}

func (pr *Priorities) removeOutput(index int) {
	pr.OutputInstancePriority = append(
		pr.OutputInstancePriority[:index],
		pr.OutputInstancePriority[index+1:]...,
	)
}

func (pr *Priorities) insertInput(index int, portToken *PortToken) {
	tail := append([]*PortToken{}, pr.InputInstancePriority[index:]...)
	pr.InputInstancePriority = append(pr.InputInstancePriority[:index], portToken)
	pr.InputInstancePriority = append(pr.InputInstancePriority, tail...)
}

func (pr *Priorities) insertOutput(index int, portToken *PortToken) {
	tail := append([]*PortToken{}, pr.OutputInstancePriority[index:]...)
	pr.OutputInstancePriority = append(pr.OutputInstancePriority[:index], portToken)
	pr.OutputInstancePriority = append(pr.OutputInstancePriority, tail...)
}

func (pr *Priorities) setInputTypeFirst(portType int) {
	temp := []int{portType}
	for _, t := range pr.InputTypePriority {
		if t != portType {
			temp = append(temp, t)
		}
	}
	pr.InputTypePriority = temp
}

func (pr *Priorities) setOutputTypeFirst(portType int) {
	temp := []int{portType}
	for _, t := range pr.OutputTypePriority {
		if t != portType {
			temp = append(temp, t)
		}
	}
	pr.OutputTypePriority = temp
}

func (pr *Priorities) findInput(cardName string, portName string) int {
	for i := 0; i < len(pr.InputInstancePriority); i++ {
		p := pr.InputInstancePriority[i]
		if p.CardName == cardName && p.PortName == portName {
			return i
		}
	}

	return -1
}

func (pr *Priorities) findOutput(cardName string, portName string) int {
	for i := 0; i < len(pr.OutputInstancePriority); i++ {
		p := pr.OutputInstancePriority[i]
		if p.CardName == cardName && p.PortName == portName {
			return i
		}
	}

	return -1
}
