// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package sessionmsg

// 用于dde-system-daemon 的各个模块通过 system bus 发送消息到 session。
// dde-session-daemon 的 service_trigger 模块负责首先接受处理这个消息。
// 目前实现了 BodyNotify 用来发送系统消息通知。

import (
	"encoding/json"
	"errors"
	"fmt"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/multierr"
)

type Message struct {
	msgCommon
	Body Body
}

type Body interface {
	MessageType() MessageType
}

// 用于解序列化 Message
type msg struct {
	msgCommon
	Body json.RawMessage
}

type msgCommon struct {
	OnlyActive bool
	Type       MessageType
}

type MessageType uint

const (
	MessageTypeNotify MessageType = iota + 1
)

func (m *Message) UnmarshalJSON(bytes []byte) error {
	var msg msg
	err := json.Unmarshal(bytes, &msg)
	if err != nil {
		return err
	}
	m.Type = msg.Type
	m.OnlyActive = msg.OnlyActive

	switch m.Type {
	case MessageTypeNotify:
		var notify BodyNotify
		err = json.Unmarshal(msg.Body, &notify)
		if err != nil {
			return err
		}
		m.Body = &notify
	}
	return nil
}

func NewMessage(onlyActive bool, body Body) *Message {
	m := &Message{
		Body: body,
	}
	m.OnlyActive = onlyActive
	m.Type = body.MessageType()
	return m
}

type BodyNotify struct {
	Icon          string
	Summary       *LocalizeStr
	Body          *LocalizeStr
	AppName       string // 如果为空，则为 dde-control-center
	ExpireTimeout int    // -1 使用默认值，0 永不过期，> 0 的是具体秒数
	Actions       []string
	Hints         map[string]dbus.Variant
}

func (*BodyNotify) MessageType() MessageType {
	return MessageTypeNotify
}

type LocalizeStr struct {
	Format string
	Args   []string
}

func (str *LocalizeStr) String() string {
	if str == nil {
		return ""
	}

	args := make([]interface{}, len(str.Args))
	for idx, arg := range str.Args {
		args[idx] = arg
	}
	return fmt.Sprintf(gettext.Tr(str.Format), args...)
}

// AgentInfoPublisher 用于从 system/bluetooth 模块获取来自 session 的 agent 注册的服务名
// 只要知道 agent 的服务名，就可以调用 service_trigger 模块在 system bus 上导出的 agent 对象，
// 没有增加新的 DBus 接口用于注册 agent。
type AgentInfoPublisher interface {
	GetActiveAgentName() string
	GetAllAgentNames() []string
}

var _publisher AgentInfoPublisher

func SetAgentInfoPublisher(v AgentInfoPublisher) {
	_publisher = v
}

func getActiveAgentName() string {
	if _publisher == nil {
		return ""
	}

	return _publisher.GetActiveAgentName()
}

func getAllAgentNames() []string {
	if _publisher == nil {
		return nil
	}

	return _publisher.GetAllAgentNames()
}

type agent struct {
	obj dbus.BusObject
}

func (a *agent) sendMessage(msg *Message) error {
	jsonData, err := json.Marshal(msg)
	if err != nil {
		return err
	}

	err = a.obj.Call(AgentIfc+"."+MethodSendMessage, 0, string(jsonData)).Err
	return err
}

// 这个 agent 实际在 dde-session-daemon 中的 service_trigger 模块中实现的
const (
	AgentPath         = "/org/deepin/dde/ServiceTrigger1/Agent"
	AgentIfc          = "org.deepin.dde.ServiceTrigger1.Agent"
	MethodSendMessage = "SendMessage"
)

func getActiveAgent() *agent {
	name := getActiveAgentName()
	if name == "" {
		return nil
	}
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return nil
	}
	obj := sysBus.Object(name, AgentPath)
	return &agent{
		obj: obj,
	}
}

func getAllAgents() []*agent {
	names := getAllAgentNames()
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return nil
	}
	result := make([]*agent, 0, len(names))
	for _, name := range names {
		if name == "" {
			continue
		}
		obj := sysBus.Object(name, AgentPath)
		result = append(result, &agent{
			obj: obj,
		})
	}
	return result
}

func SendMessage(msg *Message) error {
	if msg.OnlyActive {
		agent := getActiveAgent()
		if agent == nil {
			return errors.New("not found active agent")
		}

		return agent.sendMessage(msg)
	} else {
		agents := getAllAgents()
		var err error
		for _, agent := range agents {
			err1 := agent.sendMessage(msg)
			if err1 != nil {
				err = multierr.Append(err, err1)
			}
		}
		return err
	}
}
