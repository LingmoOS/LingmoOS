// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"fmt"
	"os/exec"
	"strconv"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	notifications "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.notifications"
	"github.com/linuxdeepin/go-lib/dbusutil"
	. "github.com/linuxdeepin/go-lib/gettext"
)

const (
	notifyIconBluetoothConnected     = "notification-bluetooth-connected"
	notifyIconBluetoothDisconnected  = "notification-bluetooth-disconnected"
	notifyIconBluetoothConnectFailed = "notification-bluetooth-error"
	// dialog use for show pinCode
	notifyDdeDialogPath = "/usr/lib/deepin-daemon/dde-bluetooth-dialog"
	// notification window stay time
	notifyTimerDuration = 30 * time.Second
)

const bluetoothDialog string = "dde-bluetooth-dialog"

var globalNotifications notifications.Notifications
var globalNotifyId uint32
var globalNotifyMu sync.Mutex

func initNotifications() error {
	// init global notification timer instance
	globalTimerNotifier = GetTimerNotifyInstance()

	sessionBus, err := dbus.SessionBus()
	if err != nil {
		return err
	}
	globalNotifications = notifications.NewNotifications(sessionBus)

	// monitor notification-close-signal
	sessionLoop := dbusutil.NewSignalLoop(sessionBus, 10)
	sessionLoop.Start()
	globalNotifications.InitSignalExt(sessionLoop, true)
	_, err = globalNotifications.ConnectActionInvoked(func(id uint32, actionKey string) {
		// has received signal, use id to compare with last globalNotifyId
		if id == globalNotifyId {
			if actionKey == "cancel" {
				err = globalBluetooth.agent.Cancel()
				if err != nil {
					logger.Warning("Cancel error:", err)
				}
			}
			// if it is the same, then send chan to instance chan to close window
			globalTimerNotifier.actionInvokedChan <- true
		}
	})
	if err != nil {
		logger.Warningf("listen action invoked failed,err:%v", err)
	}

	return nil
}

func notify(icon, summary, body string) {
	logger.Info("notify", icon, summary, body)

	globalNotifyMu.Lock()
	nid := globalNotifyId
	globalNotifyMu.Unlock()

	nid, err := globalNotifications.Notify(0, Tr("dde-control-center"), nid, icon,
		summary, body, nil, nil, -1)
	if err != nil {
		logger.Warning(err)
		return
	}
	globalNotifyMu.Lock()
	globalNotifyId = nid
	globalNotifyMu.Unlock()
}

// notify pc initiative connect to device
// so do not need to show notification window
func notifyInitiativeConnect(dev *DeviceInfo, pinCode string, needCancel string) error {
	if checkProcessExists(bluetoothDialog) {
		logger.Info("initiative already exist")
		return nil
	}

	timestamp := strconv.FormatInt(time.Now().UnixNano(), 10)
	//use command to open osd window to show pin code
	// #nosec G204
	cmd := exec.Command(notifyDdeDialogPath, pinCode, string(dev.Path), timestamp, needCancel)
	err := cmd.Start()
	if err != nil {
		logger.Infof("execute cmd command failed,err:%v", err)
		return err
	}

	go func() {
		err := cmd.Wait()
		if err != nil {
			logger.Warning(err)
		}
	}()

	return nil
}

// device passive connect to pc
// so need to show notification window
func notifyPassiveConnect(dev *DeviceInfo, pinCode string) error {
	format := Tr("Click here to connect to %q")
	summary := Tr("Add Bluetooth devices")
	body := fmt.Sprintf(format, dev.Name)
	globalNotifyMu.Lock()
	nid := globalNotifyId
	globalNotifyMu.Unlock()
	// check if bluetooth dialog is exist
	if checkProcessExists(bluetoothDialog) {
		logger.Info("Passive is not exist")
		return nil
	}
	var as = []string{"pair", Tr("Pair"), "cancel", Tr("Cancel")}
	var timestamp = strconv.FormatInt(time.Now().UnixNano(), 10)
	cmd := notifyDdeDialogPath + "," + pinCode + "," + string(dev.Path) + "," + timestamp
	hints := map[string]dbus.Variant{"x-deepin-action-pair": dbus.MakeVariant(cmd)}

	// to make sure last notification has been closed
	err := globalNotifications.CloseNotification(0, nid)
	if err != nil {
		logger.Warningf("close last notification failed,err:%v", err)
	}

	// notify connect request to dde-control-center
	// set notify time out as -1, default time out is 5 seconds
	nid, err = globalNotifications.Notify(0, Tr("dde-control-center"), nid, notifyIconBluetoothConnected,
		summary, body, as, hints, 30*1000)
	if err != nil {
		logger.Warningf("notify message failed,err:%v", err)
		return err
	}

	globalNotifyMu.Lock()
	globalNotifyId = nid
	globalNotifyMu.Unlock()

	return nil
}

// global timer notifier
var globalTimerNotifier *timerNotify

// notify timer instance
// use chan bool instead of timer, but in case to fit new requirements of future flexibly, we keep element timer
type timerNotify struct {
	actionInvokedChan chan bool
}

// GetTimerNotifyInstance get timer instance
func GetTimerNotifyInstance() *timerNotify {
	// create a global timer notify object
	timerNotifier := &timerNotify{
		actionInvokedChan: make(chan bool),
	}
	return timerNotifier
}

// begin timer routine to monitor window click notification window
func beginTimerNotify(notifyTimer *timerNotify) {
	for {
		select {
		case <-notifyTimer.actionInvokedChan:
			// monitor click window signal
			logger.Info("user click notify,close notify")
			err := globalNotifications.CloseNotification(0, globalNotifyId)
			if err != nil {
				logger.Warningf("click event close notify icon failed,err:%v", err)
			}
		}
	}
}
