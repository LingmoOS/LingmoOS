// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package lastore

import (
	"errors"
	"fmt"
	"os/exec"
	"strings"

	"github.com/godbus/dbus/v5"
	kwayland "github.com/linuxdeepin/go-dbus-factory/session/org.deepin.dde.kwayland1"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

func (*Agent) GetInterfaceName() string {
	return sessionAgentInterface
}

func (a *Agent) GetManualProxy(sender dbus.Sender) (map[string]string, *dbus.Error) {
	if !a.checkCallerAuth(sender) {
		return nil, dbusutil.ToError(fmt.Errorf("not allow %v call this method", sender))
	}
	method, err := a.lastoreObj.network.GetProxyMethod(0)
	if err != nil {
		logger.Warning(err)
		return nil, dbusutil.ToError(err)
	}
	if method != "manual" {
		return nil, dbusutil.ToError(errors.New("only support manual proxy"))
	}
	proxyTypeList := []string{
		proxyTypeHttp, proxyTypeHttps, proxyTypeFtp,
	}
	proxyEnvMap := make(map[string]string)
	for _, typ := range proxyTypeList {
		host, port, err := a.lastoreObj.network.GetProxy(0, typ)
		if err != nil {
			logger.Warning(err)
			continue
		}
		user, passwd, enable, err := a.lastoreObj.network.GetProxyAuthentication(0, typ)
		if err != nil {
			logger.Warning(err)
			continue
		}
		if enable {
			proxyEnvMap[fmt.Sprintf("%s_proxy", typ)] = fmt.Sprintf("%s://%s:%s@%s:%s", proxyTypeHttp, user, passwd, host, port)
		} else {
			proxyEnvMap[fmt.Sprintf("%s_proxy", typ)] = fmt.Sprintf("%s://%s:%s", proxyTypeHttp, host, port)
		}

	}
	host, port, err := a.lastoreObj.network.GetProxy(0, proxyTypeSocks)
	if err != nil {
		logger.Warning(err)
		return proxyEnvMap, nil
	}
	user, passwd, enable, err := a.lastoreObj.network.GetProxyAuthentication(0, proxyTypeSocks)
	if err != nil {
		logger.Warning(err)
		return proxyEnvMap, nil
	}
	if enable {
		proxyEnvMap[envAllProxy] = fmt.Sprintf("%s://%s:%s@%s:%s", proxyTypeSocks, user, passwd, host, port)
	} else {
		proxyEnvMap[envAllProxy] = fmt.Sprintf("%s://%s:%s", proxyTypeSocks, host, port)
	}

	return proxyEnvMap, nil
}

const (
	updateNotifyShow         = "dde-control-center"          // 无论控制中心状态，都需要发送的通知
	updateNotifyShowOptional = "dde-control-center-optional" // 根据控制中心更新模块焦点状态,选择性的发通知(由dde-session-daemon的lastore agent判断后控制)
)

func (a *Agent) SendNotify(sender dbus.Sender, appName string, replacesId uint32, appIcon string, summary string, body string, actions []string, hints map[string]dbus.Variant, expireTimeout int32) (uint32, *dbus.Error) {
	if !a.checkCallerAuth(sender) {
		return 0, dbusutil.ToError(fmt.Errorf("not allow %v call this method", sender))
	}
	logger.Info("receive notify from lastore daemon, app name:", appName)
	needSend := true
	if appName == updateNotifyShowOptional {
		appName = updateNotifyShow
		// 只有当控制中心获取焦点,且控制中心当前为更新模块时,不发通知
		if a.isWaylandSession {
			// 从kwayland获取
			winId, err := a.waylandWM.ActiveWindow(0)
			if err != nil {
				logger.Warning(err)
			} else {
				wInfo, err := kwayland.NewWindow(a.sessionService.Conn(), dbus.ObjectPath(fmt.Sprintf("/org/deepin/dde/KWayland1/PlasmaWindow_%v", winId)))
				if err != nil {
					logger.Warning(err)
				} else {
					name, err := wInfo.AppId(0)
					if err == nil {
						if strings.Contains(name, "dde-control-center") {
							// 焦点在控制中心上,需要判断是否为更新模块
							currentModule, err := a.controlCenter.CurrentModule().Get(0)
							if err != nil {
								logger.Warning(err)
							} else if currentModule == "update" {
								logger.Info("update module of dde-control-center is in the foreground, don't need send notify")
								needSend = false
							}
						} else if strings.Contains(name, "dde-lock") {
							// 前台应用在模态更新界面时,不发送通知(TODO: 如果后台更新时发生了锁屏，需要增加判断是否发通知)
							needSend = false
						}

					}
				}
			}
		} else {
			output, err := exec.Command("/bin/sh", "-c", "xprop -id $(xprop -root _NET_ACTIVE_WINDOW | cut -d ' ' -f 5) WM_CLASS").Output()
			if err != nil {
				logger.Warning(err)
			} else if strings.Contains(string(output), "dde-control-center") {
				// 焦点在控制中心上,需要判断是否为更新模块
				currentModule, err := a.controlCenter.CurrentModule().Get(0)
				if err != nil {
					logger.Warning(err)
				} else if currentModule == "update" {
					logger.Info("update module of dde-control-center is in the foreground, don't need send notify")
					needSend = false
				}
			} else if strings.Contains(string(output), "dde-lock") {
				// 前台应用在模态更新界面时,不发送通知(TODO: 如果后台更新时发生了锁屏，需要增加判断是否发通知)
				needSend = false
			}
		}
	}
	if needSend {
		id, err := a.lastoreObj.notifications.Notify(0, appName, replacesId, appIcon, summary, body, actions, hints, expireTimeout)
		return id, dbusutil.ToError(err)
	}
	return 0, nil
}

func (a *Agent) CloseNotification(sender dbus.Sender, id uint32) *dbus.Error {
	if !a.checkCallerAuth(sender) {
		return dbusutil.ToError(fmt.Errorf("not allow %v call this method", sender))
	}
	logger.Info("receive close notify from lastore daemon")
	return dbusutil.ToError(a.lastoreObj.notifications.CloseNotification(0, id))
}

func (a *Agent) ReportLog(sender dbus.Sender, msg string) *dbus.Error {
	if !a.checkCallerAuth(sender) {
		return dbusutil.ToError(fmt.Errorf("not allow %v call this method", sender))
	}
	logger.Info("report log from lastore daemon")
	return dbusutil.ToError(a.lastoreObj.eventLog.ReportLog(0, msg))
}
