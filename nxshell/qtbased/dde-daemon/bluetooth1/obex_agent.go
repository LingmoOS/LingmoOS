// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"errors"
	"fmt"

	"math"
	"math/rand"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	dutils "github.com/linuxdeepin/go-lib/utils"

	"github.com/godbus/dbus/v5"
	notifications "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.notifications"
	obex "github.com/linuxdeepin/go-dbus-factory/system/org.bluez.obex"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gettext"
	"github.com/linuxdeepin/go-lib/xdg/userdir"
)

const (
	obexAgentDBusPath      = dbusPath + "/ObexAgent"
	obexAgentDBusInterface = "org.bluez.obex.Agent1"

	receiveFileNotifyTimeout = 15 * 1000
	receiveFileTimeout       = 40 * time.Second
)

var receiveBaseDir = userdir.Get(userdir.Download)

type obexAgent struct {
	b *Bluetooth

	service *dbusutil.Service
	sigLoop *dbusutil.SignalLoop

	obexManager obex.Manager

	requestNotifyCh   chan bool
	requestNotifyChMu sync.Mutex

	receiveCh   chan struct{}
	recevieChMu sync.Mutex

	isCancel bool

	acceptedSessions   map[dbus.ObjectPath]int
	acceptedSessionsMu sync.Mutex

	notify   notifications.Notifications
	notifyID uint32
}

type transferObj struct {
	obex.Transfer
	sessionPath  dbus.ObjectPath
	deviceName   string
	oriFilename  string
	tempFileName string
}

func (*obexAgent) GetInterfaceName() string {
	return obexAgentDBusInterface
}

func newObexAgent(service *dbusutil.Service, bluetooth *Bluetooth) *obexAgent {
	return &obexAgent{
		b:                bluetooth,
		service:          service,
		acceptedSessions: make(map[dbus.ObjectPath]int),
	}
}

func (a *obexAgent) init() {
	sessionBus := a.service.Conn()
	a.obexManager = obex.NewManager(sessionBus)
	a.registerAgent()

	a.sigLoop = dbusutil.NewSignalLoop(a.service.Conn(), 0)
	a.sigLoop.Start()
}

// registerAgent 注册 OBEX 的代理
func (a *obexAgent) registerAgent() {
	err := a.obexManager.AgentManager().RegisterAgent(0, obexAgentDBusPath)
	if err != nil {
		logger.Error("failed to register obex agent:", err)
	}
}

// nolint
// unregisterAgent 注销 OBEX 的代理
func (a *obexAgent) unregisterAgent() {
	err := a.obexManager.AgentManager().UnregisterAgent(0, obexAgentDBusPath)
	if err != nil {
		logger.Error("failed to unregister obex agent:", err)
	}
}

// AuthorizePush 用于请求用户接收文件
func (a *obexAgent) AuthorizePush(transferPath dbus.ObjectPath) (tempFileName string, busErr *dbus.Error) {
	logger.Infof("dbus call obexAgent AuthorizePush with transferPath %v", transferPath)

	transfer, err := obex.NewTransfer(a.service.Conn(), transferPath)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	oriFilename, err := transfer.Name().Get(0)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}
	tempFileName = randFileName(oriFilename)
	if len(tempFileName) > 255 {
		tempFileName = tempFileName[:255]
	}
	sessionPath, err := transfer.Session().Get(0)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}
	logger.Debug("session path:", sessionPath)

	session, err := obex.NewSession(a.service.Conn(), sessionPath)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	deviceAddress, err := session.Session().Destination().Get(0)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	dev := a.b.getDeviceByAddress(deviceAddress)
	if dev == nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}

	deviceName := dev.Alias
	if len(deviceName) == 0 {
		deviceName = dev.Name
	}
	transferObj := &transferObj{
		transfer,
		sessionPath,
		deviceName,
		oriFilename,
		tempFileName,
	}
	accepted, err := a.isSessionAccepted(transferObj)
	if err != nil {
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}
	if !accepted {
		err = errors.New("session declined")
		logger.Warning(err)
		return "", dbusutil.ToError(err)
	}
	// 设置未文件不能传输状态
	a.b.setPropTransportable(false)

	return tempFileName, nil
}

func (a *obexAgent) isSessionAccepted(transfer *transferObj) (bool, error) {
	a.acceptedSessionsMu.Lock()
	defer a.acceptedSessionsMu.Unlock()
	if transfer == nil {
		return false, errors.New("valid object")
	}
	_, accepted := a.acceptedSessions[transfer.sessionPath]
	if !accepted {
		// 多个文件传输时只第一次判断可传输状态
		if !a.b.Transportable {
			return false, errors.New("declined")
		}
		var err error
		accepted, err = a.requestReceive(transfer.deviceName, transfer.oriFilename)
		if err != nil {
			return false, err
		}

		if !accepted {
			return false, nil
		}
		a.acceptedSessions[transfer.sessionPath] = 0
		a.notify = notifications.NewNotifications(a.service.Conn())
		a.notify.InitSignalExt(a.sigLoop, true)
		a.notifyID = 0
		a.recevieChMu.Lock()
		a.receiveCh = make(chan struct{}, 1)
		a.recevieChMu.Unlock()
		a.receiveProgress(transfer)
	} else {
		<-a.receiveCh
		a.receiveProgress(transfer)
	}

	a.acceptedSessions[transfer.sessionPath]++
	return true, nil
}

func (a *obexAgent) receiveProgress(transfer *transferObj) {
	transfer.InitSignalExt(a.sigLoop, true)

	fileSize, err := transfer.Size().Get(0)
	if err != nil {
		logger.Error("failed to get file size:", err)
	}

	var notifyMu sync.Mutex

	err = transfer.Status().ConnectChanged(func(hasValue bool, value string) {
		if !hasValue {
			return
		}

		if value != transferStatusComplete && value != transferStatusError {
			return
		}
		a.b.setPropTransportable(true)
		// 手机会在一个传送完成之后再开始下一个传送，所以 transfer path 会一样
		transfer.RemoveAllHandlers()

		a.notify.RemoveAllHandlers()

		if value == transferStatusComplete {
			// 如果获取不到FileName时，文件应该在~/.cache/obexd下
			oriFilepath, err := transfer.Filename().Get(0)
			if err != nil || oriFilepath == "" {
				oriFilepath = filepath.Join(dutils.GetCacheDir(), "obexd", transfer.tempFileName)
			}
			// 传送完成，移动到下载目录
			realFileName := moveTempFile(oriFilepath, filepath.Join(receiveBaseDir, transfer.oriFilename))

			notifyMu.Lock()
			a.notifyID = a.notifyProgress(a.notify, a.notifyID, realFileName, transfer.deviceName, 100)
			notifyMu.Unlock()
		} else {
			// 区分点击取消的传输失败和蓝牙断开的传输失败
			if a.isCancel {
				notifyMu.Lock()
				a.notifyID = a.notifyFailed(a.notify, a.notifyID, true)
				notifyMu.Unlock()
				a.isCancel = false
			} else {
				notifyMu.Lock()
				a.notifyID = a.notifyFailed(a.notify, a.notifyID, false)
				notifyMu.Unlock()
			}
		}

		a.recevieChMu.Lock()
		a.receiveCh <- struct{}{}
		a.recevieChMu.Unlock()

		// 避免下个传输还没开始就被清空，导致需要重新询问，故加上一秒的延迟
		time.AfterFunc(time.Second, func() {
			a.acceptedSessionsMu.Lock()
			a.acceptedSessions[transfer.sessionPath]--
			if a.acceptedSessions[transfer.sessionPath] == 0 {
				delete(a.acceptedSessions, transfer.sessionPath)
			}
			a.acceptedSessionsMu.Unlock()
		})
	})
	if err != nil {
		logger.Warning("connect status changed failed:", err)
	}

	var progress uint64 = math.MaxUint64
	err = transfer.Transferred().ConnectChanged(func(hasValue bool, value uint64) {
		if !hasValue {
			return
		}
		if value == 0 {
			return
		}
		status, err := transfer.Status().Get(0)
		if err != nil {
			logger.Warning(err)
			return
		}
		if status == transferStatusComplete || status == transferStatusError {
			return
		}
		newProgress := value * 100 / fileSize
		if progress == newProgress || value == fileSize {
			return
		}

		progress = newProgress
		logger.Infof("transferPath: %q, progress: %d", transfer.Path_(), progress)

		notifyMu.Lock()
		a.notifyID = a.notifyProgress(a.notify, a.notifyID, transfer.oriFilename, transfer.deviceName, progress)
		notifyMu.Unlock()
	})
	if err != nil {
		logger.Warning("connect transferred changed failed:", err)
	}
	_, err = a.notify.ConnectActionInvoked(func(id uint32, actionKey string) {
		notifyMu.Lock()
		if a.notifyID != id {
			notifyMu.Unlock()
			return
		}
		notifyMu.Unlock()
		if actionKey != "cancel" {
			return
		}
		a.isCancel = true
		a.b.setPropTransportable(true)
		err := transfer.Cancel(0)
		if err != nil {
			logger.Warning("failed to cancel transfer:", err)
		}
	})
	if err != nil {
		logger.Warning("connect action invoked failed:", err)
	}
}

func moveTempFile(src, dest string) string {
	count := 0
	suffix := filepath.Ext(dest)
	fileName := strings.TrimSuffix(dest, suffix)
	for {
		if dutils.IsFileExist(dest) {
			count++
			dest = fmt.Sprintf("%v(%v)%v", fileName, count, suffix)
		} else {
			err := os.Rename(src, dest)
			if err != nil {
				fmt.Println("failed to move file:", err)
			}
			break
		}
	}
	return dest
}

// 获取随机字母+数字组合字符串
func getRandstring(length int) string {
	if length < 1 {
		return ""
	}
	char := "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	charArr := strings.Split(char, "")
	charlen := len(charArr)
	ran := rand.New(rand.NewSource(time.Now().Unix()))
	var rchar string = ""
	for i := 1; i <= length; i++ {
		rchar = rchar + charArr[ran.Intn(charlen)]
	}
	return rchar
}

// 随机文件名
func randFileName(fileName string) string {
	randStr := getRandstring(16)
	return strings.Join([]string{
		randStr,
		fileName,
	}, "_")
}

// notifyProgress 发送文件传输进度通知
func (a *obexAgent) notifyProgress(notify notifications.Notifications, replaceID uint32, filename string, device string, progress uint64) uint32 {
	var actions []string
	var notifyID uint32
	var err error
	if progress != 100 {
		actions = []string{"cancel", gettext.Tr("Cancel")}
		hints := map[string]dbus.Variant{"suppress-sound": dbus.MakeVariant(true)}

		notifyID, err = notify.Notify(0,
			gettext.Tr("dde-control-center"),
			replaceID,
			notifyIconBluetoothConnected,
			fmt.Sprintf(gettext.Tr("Receiving %[1]q from %[2]q"), filename, device),
			fmt.Sprintf("%d%%", progress),
			actions,
			hints,
			receiveFileNotifyTimeout)
		if err != nil {
			logger.Warning("failed to send notify:", err)
		}
	} else {
		actions = []string{"_view", gettext.Tr("View")}
		hints := map[string]dbus.Variant{"x-deepin-action-_view": dbus.MakeVariant("dde-file-manager,--show-item," + filename)}
		notifyID, err = notify.Notify(0,
			gettext.Tr("dde-control-center"),
			replaceID,
			notifyIconBluetoothConnected,
			fmt.Sprintf(gettext.Tr("You have received files from %q successfully"), device),
			gettext.Tr("Done"),
			actions,
			hints,
			receiveFileNotifyTimeout)
		if err != nil {
			logger.Warning("failed to send notify:", err)
		}
	}

	return notifyID
}

// notifyFailed 发送文件传输失败通知
func (a *obexAgent) notifyFailed(notify notifications.Notifications, replaceID uint32, isCancel bool) uint32 {
	var body string
	summary := gettext.Tr("Stop Receiving Files")
	if isCancel {
		body = gettext.Tr("You have cancelled the file transfer")
	} else {
		body = gettext.Tr("Bluetooth connection failed")
	}

	notifyID, err := notify.Notify(0,
		gettext.Tr("dde-control-center"),
		replaceID,
		notifyIconBluetoothConnectFailed,
		summary,
		body,
		nil,
		nil,
		receiveFileNotifyTimeout)
	if err != nil {
		logger.Warning("failed to send notify:", err)
	}

	return notifyID
}

// Cancel 用于在客户端取消发送文件时取消文件传输请求
func (a *obexAgent) Cancel() *dbus.Error {
	logger.Info("dbus call obexAgent Cancel")

	a.requestNotifyChMu.Lock()
	defer a.requestNotifyChMu.Unlock()
	if a.requestNotifyCh == nil {
		err := errors.New("no such process")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	a.requestNotifyCh <- false
	return nil
}

// requestReceive 询问用户是否接收文件
func (a *obexAgent) requestReceive(deviceName, filename string) (bool, error) {
	notify := notifications.NewNotifications(a.service.Conn())
	notify.InitSignalExt(a.sigLoop, true)

	actions := []string{"decline", gettext.Tr("Decline"), "receive", gettext.Tr("Receive")}
	notifyID, err := notify.Notify(0,
		gettext.Tr("dde-control-center"),
		0,
		notifyIconBluetoothConnected,
		gettext.Tr("Bluetooth File Transfer"),
		fmt.Sprintf(gettext.Tr("%q wants to send files to you. Receive?"), deviceName),
		actions,
		nil,
		receiveFileNotifyTimeout)
	if err != nil {
		logger.Warning("failed to send notify:", err)
		return false, err
	}

	a.requestNotifyChMu.Lock()
	a.requestNotifyCh = make(chan bool, 10)
	a.requestNotifyChMu.Unlock()

	_, err = notify.ConnectActionInvoked(func(id uint32, actionKey string) {
		if notifyID != id {
			return
		}
		a.requestNotifyChMu.Lock()
		if a.requestNotifyCh != nil {
			a.requestNotifyCh <- actionKey == "receive"
		}
		a.requestNotifyChMu.Unlock()
	})
	if err != nil {
		logger.Warning("ConnectActionInvoked failed:", err)
		return false, dbusutil.ToError(err)
	}

	var result bool
	select {
	case result = <-a.requestNotifyCh:
		if result == false {
			a.b.setPropTransportable(true)
		}
	case <-time.After(receiveFileTimeout):
		a.b.setPropTransportable(true)
		a.notifyReceiveFileTimeout(notify, notifyID, filename)
	}

	a.requestNotifyChMu.Lock()
	a.requestNotifyCh = nil
	a.requestNotifyChMu.Unlock()

	notify.RemoveAllHandlers()

	return result, nil
}

// notifyReceiveFileTimeout 接收文件请求超时通知
func (a *obexAgent) notifyReceiveFileTimeout(notify notifications.Notifications, replaceID uint32, filename string) {
	_, err := notify.Notify(0,
		gettext.Tr("dde-control-center"),
		replaceID,
		notifyIconBluetoothConnectFailed,
		gettext.Tr("Stop Receiving Files"),
		fmt.Sprintf(gettext.Tr("Receiving %q timed out"), filename),
		nil,
		nil,
		receiveFileNotifyTimeout)
	if err != nil {
		logger.Warning("failed to send notify:", err)
	}
}
