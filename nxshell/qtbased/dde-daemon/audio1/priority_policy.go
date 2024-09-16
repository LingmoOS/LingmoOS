// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package audio

import (
	"strings"

	"github.com/linuxdeepin/go-lib/pulse"
)

const (
	PortTypeBluetooth    int = iota // 蓝牙音频
	PortTypeHeadset                 // 3.5mm 耳麦
	PortTypeUsb                     // USB
	PortTypeBuiltin                 // 内置扬声器和话筒
	PortTypeHdmi                    // HDMI
	PortTypeLineIO                  // 线缆输入输出
	PortTypeMultiChannel            // 多声道
	PortTypeUnknown                 // 其他类型

	PortTypeCount        // 有效的端口类型个数
	PortTypeInvalid = -1 // 表示无效的类型
)

// 判断一组字符串中，是否存在其中一个字符串A，使得substr是A的子字符串，不区分大小写
func hasKeyword(stringList []string, substr string) bool {
	for _, str := range stringList {
		if strings.Contains(strings.ToLower(str), strings.ToLower(substr)) {
			return true
		}
	}

	return false
}

// 检测端口的类型
func DetectPortType(card *pulse.Card, port *pulse.CardPortInfo) int {
	// 需要判断的字段列表
	stringList := []string{
		card.Name,
		port.Name,
		card.PropList["alsa.card_name"],
		card.PropList["alsa.long_card_name"],
	}

	if hasKeyword(stringList, "multichannel") {
		return PortTypeMultiChannel
	}

	if hasKeyword(stringList, "bluez") ||
		hasKeyword(stringList, "bluetooth") {
		return PortTypeBluetooth
	}

	if hasKeyword(stringList, "linein") ||
		hasKeyword(stringList, "lineout") {
		return PortTypeLineIO
	}

	if hasKeyword(stringList, "rear-mic") ||
		hasKeyword(stringList, "front-mic") ||
		hasKeyword(stringList, "headphone") ||
		hasKeyword(stringList, "headset") {
		return PortTypeHeadset
	}

	if hasKeyword(stringList, "usb") {
		return PortTypeUsb
	}

	if hasKeyword(stringList, "hdmi") {
		return PortTypeHdmi
	}

	if hasKeyword(stringList, "speaker") ||
		hasKeyword(stringList, "input-mic") {
		return PortTypeBuiltin
	}

	return PortTypeUnknown
}

// 优先级中使用的端口（注意：用于表示端口的结构体有好几个，不要弄混）
type PriorityPort struct {
	CardName string
	PortName string
	PortType int // 部分声卡需要在Property里判断类型，只有CardName和PortName不足以用来判断PortType，因此添加此项
}

// 端口实例优先级列表
type PriorityPortList []*PriorityPort

// 判断端口实例优先级列表中是否包含某个值，判断时只考虑CardName和PortName，忽略PortType
func (portList *PriorityPortList) hasElement(port *PriorityPort) bool {
	for _, p := range *portList {
		if p.CardName == port.CardName && p.PortName == port.PortName {
			return true
		}
	}

	return false
}

// 端口类型优先级列表
type PriorityTypeList []int

// 判断端口类型优先级列表中是否包含某个值
func (typeList *PriorityTypeList) hasElement(value int) bool {
	for _, v := range *typeList {
		if value == v {
			return true
		}
	}

	return false
}

// 管理一组实例和类型的优先级
type PriorityPolicy struct {
	Ports PriorityPortList
	Types PriorityTypeList
}

// 新建一个PriorityPolicy
func NewPriorityPolicy() *PriorityPolicy {
	return &PriorityPolicy{
		Ports: make(PriorityPortList, 0),
		Types: make(PriorityTypeList, 0),
	}
}

// 读取配置文件获得的类型优先级中类型的数量少于PortTypeCount时
// 将缺少的类型补充完整
// 通常发生在增加了新的端口类型的时候
// 也可以用于初始化空的优先级列表
func (pp *PriorityPolicy) completeTypes() {
	for i := 0; i < PortTypeCount; i++ {
		if !pp.Types.hasElement(i) {
			pp.Types = append(pp.Types, i)
			logger.Debugf("append type %d", i)
		}
	}
}

// 调整输出端口实例的优先级
// 当代码中定义的端口类型发生了变化时，某些端口的优先级需要进行调整
// 例如原先将 多声道 视为 内置扬声器
// 现在将 多声道 单独分为一类
// 从旧的配置文件里读出来的多声道实例的优先级就不合理了
// 此时通过类型优先级对实例优先级进行排序
// 为了避免原先类型优先级相同的端口在排序时被打乱
// 不能使用快速排序，这里使用稳定的冒泡排序
func (pp *PriorityPolicy) sortPorts() {
	length := len(pp.Ports)
	for i := 0; i+1 < length; i++ {
		for j := i; j+1 < length; j++ {
			port1 := pp.Ports[j]
			port2 := pp.Ports[j+1]
			type1 := port1.PortType
			type2 := port2.PortType

			// type1优先级比type2低，交换
			if type1 != type2 && pp.GetPreferType(type1, type2) == type2 {
				pp.Ports[j], pp.Ports[j+1] = pp.Ports[j+1], pp.Ports[j]
				logger.Debugf("swap <%s:%s> and <%s:%s>",
					pp.Ports[j].CardName, pp.Ports[j].PortName,
					pp.Ports[j+1].CardName, pp.Ports[j+1].PortName)
			}
		}
	}
}

// 获取端口数量
func (pp *PriorityPolicy) CountPort() int {
	return len(pp.Ports)
}

// 比较端口类型的优先级
func (pp *PriorityPolicy) GetPreferType(type1 int, type2 int) int {
	for _, t := range pp.Types {
		if t == type1 || t == type2 {
			return t
		}
	}

	return -1
}

// 比较端口实例的优先级
func (pp *PriorityPolicy) GetPreferPort(port1 *PriorityPort, port2 *PriorityPort) *PriorityPort {
	for _, port := range pp.Ports {
		if *port == *port1 || *port == *port2 {
			return port
		}
	}

	return nil
}

// 查找一个端口的index（这个index仅仅指在优先级列表中的index，和PulseAudio中的index无关）
func (pp *PriorityPolicy) FindPortIndex(cardName string, portName string) int {
	for i, port := range pp.Ports {
		if port.CardName == cardName && port.PortName == portName {
			return i
		}
	}

	return -1
}

// 在末尾添加一个端口
func (pp *PriorityPolicy) AppendPort(port *PriorityPort) {
	pp.Ports = append(pp.Ports, port)
}

// 在指定位置之前插入一个端口,index <= 0 则插入到开头，index >= len 则添加到末尾
func (pp *PriorityPolicy) InsertPortBeforeIndex(port *PriorityPort, index int) {
	if index < 0 {
		index = 0
	}

	if index > len(pp.Ports) {
		index = len(pp.Ports)
	}

	tail := append(PriorityPortList{}, pp.Ports[index:]...) // deep copy
	pp.Ports = append(pp.Ports[:index], port)
	pp.Ports = append(pp.Ports, tail...)
}

// 添加一个端口，返回插入的位置索引
func (pp *PriorityPolicy) AddPort(port *PriorityPort) int {
	// 根据类型优先级，将端口插入到合适的位置
	length := len(pp.Ports)
	for i, p := range pp.Ports {
		if pp.GetPreferType(port.PortType, p.PortType) == port.PortType {
			pp.InsertPortBeforeIndex(port, i)
			return i
		}
	}

	// 如果其它端口优先级都比它高，添加到末尾
	pp.AppendPort(port)
	return length
}

// 添加一个原始端口，返回插入的位置索引
func (pp *PriorityPolicy) AddRawPort(card *pulse.Card, port *pulse.CardPortInfo) int {
	portType := DetectPortType(card, port)
	newPort := PriorityPort{card.Name, port.Name, portType}

	// 根据类型优先级，将端口插入到合适的位置
	length := len(pp.Ports)
	for i, p := range pp.Ports {
		if pp.GetPreferType(portType, p.PortType) == portType {
			pp.InsertPortBeforeIndex(&newPort, i)
			return i
		}
	}

	// 如果其它端口优先级都比它高，添加到末尾
	pp.AppendPort(&newPort)
	return length
}

// 通过index删除一个端口
func (pp *PriorityPolicy) RemovePortByIndex(index int) bool {
	if index < 0 || index >= len(pp.Ports) {
		return false
	}

	pp.Ports = append(pp.Ports[:index], pp.Ports[index+1:]...)
	return true
}

// 通过名称删除一个端口
func (pp *PriorityPolicy) RemovePortByName(cardName string, portName string) bool {
	index := pp.FindPortIndex(cardName, portName)
	return pp.RemovePortByIndex(index)
}

// 删除一个端口
func (pp *PriorityPolicy) RemovePort(port *PriorityPort) bool {
	return pp.RemovePortByName(port.CardName, port.PortName)
}

// 设置有效端口
// 这是因为从配置文件读出来的数据是上次运行时保存的，两次运行之间（例如关机状态下）可能插拔了设备
// 因此需要删除无效端口，添加新的有效端口
func (pp *PriorityPolicy) SetPorts(ports PriorityPortList) {
	// 清除无效的端口
	count := len(pp.Ports)
	for i := 0; i < count; {
		port := pp.Ports[i]
		if !ports.hasElement(port) {
			pp.RemovePort(port)
			logger.Debugf("remove port <%s:%s>", port.CardName, port.PortName)
			count--
		} else {
			logger.Debugf("valid port <%s:%s>", port.CardName, port.PortName)
			i++
		}
	}

	// 添加缺少的有效端口
	count = len(ports)
	for i := 0; i < count; i++ {
		port := ports[i]
		if !pp.Ports.hasElement(port) {
			pp.AddPort(port)
			logger.Debugf("add port <%s:%s>", port.CardName, port.PortName)
			if port.PortType < 0 || port.PortType >= PortTypeCount {
				logger.Warningf("unexpected port type <%d> of port <%s:%s>", port.PortType, port.CardName, port.PortName)
			}
		} else {
			logger.Debugf("exist port <%s:%s>", port.CardName, port.PortName)
		}
	}
}

// 获取优先级最高的端口
func (pp *PriorityPolicy) GetTheFirstPort() PriorityPort {
	if len(pp.Ports) > 0 {
		return *(pp.Ports[0])
	} else {
		return PriorityPort{
			"",
			"",
			PortTypeInvalid,
		}
	}
}

// 获取优先级最高的类型
func (pp *PriorityPolicy) GetTheFirstType() int {
	if len(pp.Types) > 0 {
		return pp.Types[0]
	} else {
		return -1
	}
}

// 将指定类型的优先级设为最高，此函数不会提高该类型的端口的优先级
func (pp *PriorityPolicy) SetTheFirstType(portType int) bool {
	if portType < 0 || portType >= PortTypeCount {
		return false
	}

	newTypes := []int{portType}
	for _, t := range pp.Types {
		if t != portType {
			newTypes = append(newTypes, t)
		}
	}

	pp.Types = newTypes
	return true
}

// 将指定端口的优先级设为最高，副作用：将该端口类型的优先级设为最高，将同类端口的优先级提高
func (pp *PriorityPolicy) SetTheFirstPort(cardName string, portName string) bool {
	portIndex := pp.FindPortIndex(cardName, portName)
	if portIndex < 0 {
		logger.Warningf("cannot find <%s:%s> in priority list", cardName, portName)
		return false
	}

	// 类型优先级设为最高
	portType := pp.Ports[portIndex].PortType
	pp.SetTheFirstType(portType)

	// 端口优先级设为最高
	port := pp.Ports[portIndex]
	pp.RemovePortByIndex(portIndex)
	pp.InsertPortBeforeIndex(port, 0)

	// 提升相同类型端口的优先级
	insertPos := 0
	for i, p := range pp.Ports {
		if p.PortType == portType {
			pp.RemovePortByIndex(i)
			pp.InsertPortBeforeIndex(p, insertPos)
			insertPos++
		}
	}

	return true
}

// 删除一个声卡上的所有端口
func (pp *PriorityPolicy) RemoveCard(cardName string) {
	for i := 0; i < len(pp.Ports); {
		p := pp.Ports[i]
		if p.CardName == cardName {
			pp.RemovePortByIndex(i)
		} else {
			i++
		}
	}
}
