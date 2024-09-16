// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"

	"github.com/linuxdeepin/dde-daemon/common/dsync"
	kwin "github.com/linuxdeepin/go-dbus-factory/session/org.kde.kwin"
	"github.com/linuxdeepin/go-gir/gio-2.0"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/dbusutil/gsprop"
	"github.com/linuxdeepin/go-lib/xdg/basedir"
)

const (
	gsSchemaInputDevices = "com.deepin.dde.inputdevices"
	gsKeyWheelSpeed      = "wheel-speed"
	imWheelBin           = "imwheel"
)

type devicePathInfo struct {
	Path string
	Type string
}
type devicePathInfos []*devicePathInfo

type Manager struct {
	Infos      devicePathInfos // readonly
	WheelSpeed gsprop.Uint     `prop:"access:rw"`

	settings          *gio.Settings
	imWheelConfigFile string

	kbd        *Keyboard
	mouse      *Mouse
	trackPoint *TrackPoint
	tpad       *Touchpad
	wacom      *Wacom

	kwinManager kwin.InputDeviceManager
	kwinIdList  []dbusutil.SignalHandlerId

	sessionSigLoop *dbusutil.SignalLoop
	syncConfig     *dsync.Config
}

func NewManager(service *dbusutil.Service) *Manager {
	var m = new(Manager)
	m.imWheelConfigFile = filepath.Join(basedir.GetUserHomeDir(), ".imwheelrc")

	m.Infos = devicePathInfos{
		&devicePathInfo{
			Path: kbdDBusInterface,
			Type: "keyboard",
		},
		&devicePathInfo{
			Path: mouseDBusInterface,
			Type: "mouse",
		},
		&devicePathInfo{
			Path: trackPointDBusInterface,
			Type: "trackpoint",
		},
		&devicePathInfo{
			Path: touchPadDBusInterface,
			Type: "touchpad",
		},
	}

	m.settings = gio.NewSettings(gsSchemaInputDevices)
	m.WheelSpeed.Bind(m.settings, gsKeyWheelSpeed)

	m.kbd = newKeyboard(service)
	m.wacom = newWacom(service)

	m.tpad = newTouchpad(service)

	m.mouse = newMouse(service, m.tpad)

	m.trackPoint = newTrackPoint(service)

	m.kwinManager = kwin.NewInputDeviceManager(service.Conn())

	m.sessionSigLoop = dbusutil.NewSignalLoop(service.Conn(), 10)
	m.syncConfig = dsync.NewConfig("peripherals", &syncConfig{m: m},
		m.sessionSigLoop, dbusPath, logger)

	return m
}

func (m *Manager) setWheelSpeed() {
	speed := m.settings.GetUint(gsKeyWheelSpeed)
	// speed range is [1,100]
	logger.Debug("setWheelSpeed", speed)

	// 为了避免imwheel对kwin影响，先杀死imwheel
	err := exec.Command("pkill", "-ef", imWheelBin).Run()
	if err != nil {
		logger.Warning(err)
	}

	err = m.setWaylandWheelSpeed(speed)
	if err == nil {
		logger.Info("set Wayland WheelSpeed finish")
		return
	}

	logger.Info("can not set WheelSpeed by Wayland interface, use imwheel")
	// 通过kwin设置wheel speed失败时候才通过命令设置
	err = writeImWheelConfig(m.imWheelConfigFile, speed)
	if err != nil {
		logger.Warning("failed to write imwheel config file:", err)
		return
	}

	err = controlImWheel(speed)
	if err != nil {
		logger.Warning("failed to control imwheel:", err)
		return
	}
}

func controlImWheel(speed uint32) error {
	if speed == 1 {
		// quit
		return exec.Command(imWheelBin, "-k", "-q").Run()
	}
	// restart
	return exec.Command(imWheelBin, "-k", "-b", "4 5").Run()
}

func writeImWheelConfig(file string, speed uint32) error {
	logger.Debugf("writeImWheelConfig file:%q, speed: %d", file, speed)

	const header = `# written by ` + dbusServiceName + `
".*"
Control_L,Up,Control_L|Button4
Control_R,Up,Control_R|Button4
Control_L,Down,Control_L|Button5
Control_R,Down,Control_R|Button5
Shift_L,Up,Shift_L|Button4
Shift_R,Up,Shift_R|Button4
Shift_L,Down,Shift_L|Button5
Shift_R,Down,Shift_R|Button5
`
	fh, err := os.Create(file)
	if err != nil {
		return err
	}
	defer fh.Close()
	writer := bufio.NewWriter(fh)

	_, err = writer.Write([]byte(header))
	if err != nil {
		return err
	}

	//  Delay Before Next KeyPress Event
	delay := 240000 / speed
	_, err = fmt.Fprintf(writer, "None,Up,Button4,%d,0,%d\n", speed, delay)
	if err != nil {
		return err
	}

	_, err = fmt.Fprintf(writer, "None,Down,Button5,%d,0,%d\n", speed, delay)
	if err != nil {
		return err
	}

	err = writer.Flush()
	if err != nil {
		return err
	}

	return fh.Sync()
}

func (m *Manager) init() {
	m.kbd.init()
	m.kbd.handleGSettings()
	m.wacom.init()
	m.wacom.handleGSettings()
	m.tpad.init()
	m.tpad.handleGSettings()
	m.mouse.init()
	m.mouse.handleGSettings()
	m.trackPoint.init()
	m.trackPoint.handleGSettings()

	m.setWheelSpeed()
	m.handleGSettings()

	m.sessionSigLoop.Start()
}
