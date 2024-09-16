// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package inputdevices1

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	configManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	"github.com/linuxdeepin/go-lib/dbusutil"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

const (
	_usbDevicePath                  = "/sys/bus/usb/devices"
	_pciWakeupDevicePath            = "/proc/acpi/wakeup"
	_dsettingsAppID                 = "org.deepin.dde.daemon"
	_dsettingsInputdevicesName      = "org.deepin.dde.daemon.inputdevices"
	_dsettingsDeviceWakeupStatusKey = "deviceWakeupStatus"
	_dsettingsTouchpadEnabledKey    = "toupadEnabled"
	_ps2mDevice                     = "PS2M"
)

//go:generate dbusutil-gen -type InputDevices,Touchpad inputdevices.go touchpad.go
//go:generate dbusutil-gen em -type InputDevices,Touchpad
type InputDevices struct {
	service       *dbusutil.Service
	systemSigLoop *dbusutil.SignalLoop
	l             *libinput

	maxTouchscreenId uint32

	touchscreensMu sync.Mutex
	touchscreens   map[dbus.ObjectPath]*Touchscreen
	// dbusutil-gen: equal=touchscreenSliceEqual
	Touchscreens []dbus.ObjectPath

	touchpadMu sync.Mutex
	touchpad   *Touchpad

	supportWakeupDevices  []string          //所有支持usbhid的设备路径
	SupportWakeupDevices  map[string]string `prop:"access:rw"` //保存所有支持usbhid设备的power/wakeup状态
	dsgWakeupDeviceStatus []string          //用于存储dsg的数据
	dsgInputDevices       configManager.Manager

	//nolint
	signals *struct {
		TouchscreenAdded, TouchscreenRemoved struct {
			path dbus.ObjectPath
		}

		// Libinput events state changed
		EventChanged struct {
			devNode string //设备节点
			devName string //设备名称
			path    string //设备DEVPATH
			state   bool   //插拔状态，true(插入)， false(拔出)
		}
	}
}

func newInputDevices() *InputDevices {
	return &InputDevices{
		touchscreens: make(map[dbus.ObjectPath]*Touchscreen),
	}
}

func (*InputDevices) GetInterfaceName() string {
	return dbusInterface
}

func (m *InputDevices) init() {
	m.initDSettings(m.service)
	m.l = newLibinput(m)
	m.l.start()
	go func() {
		m.SupportWakeupDevices = make(map[string]string)
		m.updateSupportWakeupDevices()
		if err := TouchpadExist(touchpadSwitchFile); err == nil {
			m.newTouchpad()
			if m.dsgInputDevices == nil {
				return
			}
			v, err := m.dsgInputDevices.Value(0, _dsettingsTouchpadEnabledKey)
			if err != nil {
				logger.Warning(err)
				return
			}
			err = m.touchpad.setTouchpadEnable(v.Value().(bool))
			if err != nil {
				logger.Warning(err)
			}
		}
	}()
}

// Note：由于数组默认长度为0，后面append时，需要重新申请内存和拷贝，所以效率较低
func getMapKeys(m map[string]string) []string {
	keys := []string{}
	for k := range m {
		keys = append(keys, k)
	}
	return keys
}

func (m *InputDevices) setWakeupDevices(path string, value string) error {
	supportWakeupDevicessupport := getMapKeys(m.SupportWakeupDevices)
	for _, key := range supportWakeupDevicessupport {
		if key == path {
			err := m.saveDeviceFile(path, value, true)
			if err != nil {
				logger.Warning(err)
				return err
			}
			return nil
		}
	}

	return dbusutil.ToError(fmt.Errorf(" invalid. %s not exist", path))
}

func delteDataFromArr(list []string, value string) []string {
	for i, val := range list {
		if strings.Contains(val, value) {
			list = append(list[:i], list[i+1:]...)
			break
		}
	}
	return list
}

func (m *InputDevices) tryApeendDsgData(list []string, key, value string) []string {
	//更新已经存在的数据
	for _, data := range list {
		dd := strings.Split(data, ":")
		if len(dd) < 2 {
			continue
		}

		if dd[0] != key {
			continue
		}

		if dd[1] == value {
			return list
		}

		list = delteDataFromArr(list, key)
		list = append(list, fmt.Sprintf("%s:%s", key, value))
		return list
	}
	//添加新数据
	list = append(list, fmt.Sprintf("%s:%s", key, value))
	return list
}

func (m *InputDevices) dsgSetValue(path, value string) error {
	if m.dsgInputDevices == nil {
		return errors.New("dsgInputDevices is nil")
	}
	//获取旧的数据
	v, err := m.dsgInputDevices.Value(0, _dsettingsDeviceWakeupStatusKey)
	if err != nil {
		logger.Warning("[dsgSetValue] getDsg err : ", err)
		return err
	}
	dsgWakeupDeviceStatus := v.Value().([]dbus.Variant)
	m.dsgWakeupDeviceStatus = nil
	logger.Info("[dsgSetValue] dsg save devices : ", dsgWakeupDeviceStatus)
	for _, val := range dsgWakeupDeviceStatus {
		dd := strings.Split(regexp.MustCompile("\"").ReplaceAllString(val.String(), ""), ":")
		logger.Debug("[dsgSetValue] strings.Split dd : ", dd)
		if len(dd) < 2 {
			continue
		}
		m.dsgWakeupDeviceStatus = m.tryApeendDsgData(m.dsgWakeupDeviceStatus, dd[0], dd[1])
	}

	//更新dsg数据
	m.dsgWakeupDeviceStatus = m.tryApeendDsgData(m.dsgWakeupDeviceStatus, path, value)
	logger.Info("[dsgSetValue]11 update value; will set dsgList : ", m.dsgWakeupDeviceStatus)

	err = m.dsgInputDevices.SetValue(0, _dsettingsDeviceWakeupStatusKey, dbus.MakeVariant(m.dsgWakeupDeviceStatus))
	if err != nil {
		logger.Warning("[dsgSetValue] SetValue err : ", err)
	} else {
		logger.Info("[dsgSetValue] SetValue success.")
	}
	return err
}

func (m *InputDevices) initDSettings(sysBus *dbusutil.Service) {
	if sysBus == nil {
		return
	}
	dsg := configManager.NewConfigManager(sysBus.Conn())

	inputDevicesPath, err := dsg.AcquireManager(0, _dsettingsAppID, _dsettingsInputdevicesName, "")
	if err != nil {
		logger.Warning(err)
		return
	}

	m.dsgInputDevices, err = configManager.NewManager(sysBus.Conn(), inputDevicesPath)
	if err != nil {
		logger.Warning(err)
		return
	}

	getDeviceWakeupStatusFunc := func() {
		v, err := m.dsgInputDevices.Value(0, _dsettingsDeviceWakeupStatusKey)
		if err != nil {
			logger.Warning(err)
			return
		}

		dsgWakeupDeviceStatus := v.Value().([]dbus.Variant)
		for _, value := range dsgWakeupDeviceStatus {
			if !strings.Contains(value.String(), ":") {
				continue
			}
			curV := value.String()
			d := regexp.MustCompile("\"").ReplaceAllString(curV, "")
			dsgVal := strings.Split(d, ":")

			if len(dsgVal) < 2 {
				continue
			}
			m.dsgWakeupDeviceStatus = append(m.dsgWakeupDeviceStatus, fmt.Sprintf("%s:%s", dsgVal[0], dsgVal[1]))
		}
	}

	getDeviceWakeupStatusFunc()

	if m.dsgInputDevices != nil {
		//dsg配置数据改变
		m.dsgInputDevices.InitSignalExt(m.systemSigLoop, true)
		_, err = m.dsgInputDevices.ConnectValueChanged(func(key string) {
			if key == _dsettingsDeviceWakeupStatusKey {
				getDeviceWakeupStatusFunc()
			}
		})
	}
	m.systemSigLoop.Start()
	if err != nil {
		logger.Warning(err)
	}
}

// 通过map不能将相同数据添加进去，append数据后计算map长度来确认重复数据
func getDeleteData(truthValue, mapValue []string) (ret []string) {
	if len(truthValue) < len(mapValue) {
		tmpArr := make(map[string]int)
		for _, v := range truthValue {
			tmpArr[v] = 0
		}
		lenTmpArr := len(tmpArr)
		for _, v := range mapValue {
			tmpArr[v] = 0
			if len(tmpArr) > lenTmpArr {
				ret = append(ret, v)
			}
		}
	}
	return ret
}

func (m *InputDevices) updatePluggableDevice() {
	// 移除了设备，删除map中数据
	delArr := getDeleteData(m.supportWakeupDevices, getMapKeys(m.SupportWakeupDevices))
	for _, value := range delArr {
		delete(m.SupportWakeupDevices, value)
	}
}

func (m *InputDevices) updateSupportWakeupDevices() {
	logger.Info("updateSupportWakeupDevices")
	m.supportWakeupDevices = getSupportUsbhidDevices()
	devicePath, err := getSupportAcpiDevices(_ps2mDevice)
	if err != nil {
		logger.Warning(err)
	} else {
		m.supportWakeupDevices = append(m.supportWakeupDevices, devicePath)
	}

	m.updatePluggableDevice()
	//更新完拔出设备后，再处理可能新插入的设备
	for _, devicePath := range m.supportWakeupDevices {
		value := getArrValueFromKey(m.dsgWakeupDeviceStatus, devicePath)
		logger.Infof("getArrValueFromKey path %s,value %s", devicePath, value)
		if isAcpiDevice(devicePath) {
			if value != "" {
				m.setPropSupportWakeupDevices(devicePath, value)
			} else {
				m.setPropSupportWakeupDevices(devicePath, getSupportAcpiDeviceEnable(devicePath))
			}
		} else {
			if value != "" {
				m.setPropSupportWakeupDevices(devicePath, value)
			} else {
				m.setPropSupportWakeupDevices(devicePath, getReadFileValidData(devicePath))
			}
		}
	}
}

func getArrValueFromKey(list []string, key string) string {
	for _, value := range list {
		dd := strings.Split(value, ":")
		if len(dd) < 2 {
			continue
		}
		if dd[0] == key {
			return dd[1]
		}
	}
	return ""
}

func (m *InputDevices) saveDeviceFile(path, value string, userSet bool) error {
	if path == "" || value == "" {
		return dbusutil.ToError(fmt.Errorf("can't set empty value"))
	}
	dsgValue := getArrValueFromKey(m.dsgWakeupDeviceStatus, path)
	// 使用dsg数据的时候,不需要再次更新配置
	bNeedUpdateDsg := true
	if dsgValue != "" && dsgValue != value {
		//从设备读取数据时，如果存在dsg设置则，以dsg的值为准
		if !userSet {
			value = dsgValue
			bNeedUpdateDsg = false
			logger.Infof("use user set device state. path : %s, value : %s", path, value)
		} else {
			logger.Infof("use kernel device state.  path : %s, value : %s", path, value)
		}
	} else {
		logger.Debugf("dsgValue equal with kernel value.  path : %s, value : %s", path, value)
	}

	// 内核仅支持 disabled/enabled
	if value == "disabled" || value == "enabled" {
		if isAcpiDevice(path) {
			err := setSupportAcpiDeviceEnable(path)
			if err != nil {
				logger.Warning(err)
				return dbusutil.ToError(err)
			}

		} else {
			readData := getReadFileValidData(path)
			logger.Debugf("[saveDeviceFile] value : %s, readData : %s", value, readData)
			if readData == value {
				logger.Warningf("can't WriteFile, the value is equal. path : %s, value : %s", path, value)
				return nil
			}
			err := ioutil.WriteFile(path, []byte(value), 0755)
			if err != nil {
				return dbusutil.ToError(fmt.Errorf("WriteFile err : %s", err))
			}
		}

		//只有用户设置的值才需要保存，默认值不需要存储
		if bNeedUpdateDsg {
			m.dsgSetValue(path, value)
		}
		m.SupportWakeupDevices[path] = value
		logger.Infof("WriteFile success, path : %s , value : %s", path, value)
	} else {
		return dbusutil.ToError(fmt.Errorf("[saveDeviceFile]set value is invalid : %s", value))
	}

	return nil
}

func (m *InputDevices) newTouchscreen(dev *libinputDevice) {
	m.touchscreensMu.Lock()
	defer m.touchscreensMu.Unlock()

	m.maxTouchscreenId++
	t := newTouchscreen(dev, m.service, m.maxTouchscreenId)

	path := dbus.ObjectPath(touchscreenDBusPath + strconv.FormatUint(uint64(t.id), 10))
	t.export(path)

	m.touchscreens[path] = t

	touchscreens := append(m.Touchscreens, path)
	m.setPropTouchscreens(touchscreens)

	m.service.Emit(m, "TouchscreenAdded", path)
}

func (m *InputDevices) newTouchpad() {
	m.touchpadMu.Lock()
	defer m.touchpadMu.Unlock()

	t := newTouchpad(m.service)
	err := t.export(dbus.ObjectPath(touchpadDBusPath))
	if err != nil {
		logger.Warning(err)
	}

	m.touchpad = t
}

func (m *InputDevices) removeTouchscreen(dev *libinputDevice) {
	m.touchscreensMu.Lock()
	defer m.touchscreensMu.Unlock()

	i := m.getIndexByDevNode(dev.GetDevNode())
	if i == -1 {
		logger.Warningf("device %s not found", dev.GetDevNode())
		return
	}

	path := m.Touchscreens[i]

	touchscreens := append(m.Touchscreens[:i], m.Touchscreens[i+1:]...)
	m.setPropTouchscreens(touchscreens)

	t := m.touchscreens[path]
	t.stopExport()

	delete(m.touchscreens, path)

	m.service.Emit(m, "TouchscreenRemoved", path)
}

func (m *InputDevices) getIndexByDevNode(devNode string) int {
	var path dbus.ObjectPath
	for i, v := range m.touchscreens {
		if v.DevNode == devNode {
			path = i
		}
	}

	if path == "" {
		return -1
	}

	for i, v := range m.Touchscreens {
		if v == path {
			return i
		}
	}

	return -1
}

func touchscreenSliceEqual(v1 []dbus.ObjectPath, v2 []dbus.ObjectPath) bool {
	if len(v1) != len(v2) {
		return false
	}
	for i, e1 := range v1 {
		if e1 != v2[i] {
			return false
		}
	}
	return true
}

func getReadFileValidData(path string) string {
	// "03" : [48 51]
	// data: [48 51 10] , need delete 10
	data, err := ioutil.ReadFile(path)
	if err != nil {
		logger.Warning(err)
	}

	var databyte []byte
	for i := 0; i < len(data)-1; i++ {
		databyte = append(databyte, data[i])
	}
	return string(databyte)
}

func getSupportUsbhidDevices() []string {
	dirs, err := ioutil.ReadDir(_usbDevicePath)
	if err != nil {
		logger.Warning(" [getSupportUsbhidDevices] err : ", err)
		return nil
	}
	//记录所有类似1-2这样的目录名
	var listDir []string
	for _, dir := range dirs {
		if strings.Contains(dir.Name(), ":") || !strings.Contains(dir.Name(), "-") {
			continue
		}
		listDir = append(listDir, dir.Name())
	}

	// 记录所有包含:,且存在类似1-2名字的目录名
	var listDirInterface []string
	for _, path := range listDir {
		for _, dir := range dirs {
			if strings.Contains(dir.Name(), path) && strings.Contains(dir.Name(), ":") {
				listDirInterface = append(listDirInterface, dir.Name())
				continue
			}
		}
	}
	logger.Debug("[getSupportUsbhidDevices] listDirs : ", listDir, listDirInterface)

	// 记录类似后面文件值为3，即支持usbhid /sys/bus/usb/devices/1-10:1.0/bInterfaceSubClass
	var tmp []string
	for _, dir := range listDirInterface {
		bInterfaceClassPath := fmt.Sprintf("%s/%s/%s", _usbDevicePath, dir, "bInterfaceClass")
		if !dutils.IsFileExist(bInterfaceClassPath) {
			logger.Warningf("[getSupportUsbhidDevices] bInterfaceClassPath : %s not exist.", bInterfaceClassPath)
			continue
		}

		bInterfaceClassValue := getReadFileValidData(bInterfaceClassPath)

		if bInterfaceClassValue == string("03") {
			tmp = append(tmp, dir)
			logger.Debug("[getSupportUsbhidDevices] valid bInterfaceClassPath : ", bInterfaceClassPath, bInterfaceClassValue)
		}
	}

	// 支持usbhid的目录名
	var validList []string
	bValidData := true
	for _, dir := range tmp { //example: 【1-3:1.0
		for _, dirName := range listDir { //example: [1-3]
			if strings.Contains(dir, dirName) {
				value := fmt.Sprintf("%s/%s/power/wakeup", _usbDevicePath, dirName)
				bValidData = true
				//  已经包含的数据不需要再次添加
				for _, valid := range validList {
					if valid == value {
						bValidData = false
						break
					}
				}
				if bValidData {
					validList = append(validList, value)
				}
				continue
			}
		}
	}
	logger.Debug("[getSupportUsbhidDevices] validList : ", validList)
	return validList
}

// 由于/proc/acpi/wakeup下面设备较多，将路径改为/proc/acpi/wakeup:PS2M格式
func getSupportAcpiDevices(device string) (string, error) {
	data, err := ioutil.ReadFile(_pciWakeupDevicePath)
	if err != nil {
		return "", err
	}

	content := string(data)
	lines := strings.Split(content, "\n")

	for _, line := range lines {
		if strings.HasPrefix(line, device) {
			return _pciWakeupDevicePath + "-" + device, nil
		}
	}

	return "", err
}

func isAcpiDevice(devicePath string) bool {
	paths := strings.Split(devicePath, "-")
	if len(paths) != 2 {
		logger.Warningf("device path %s is invalid", devicePath)
		return false
	}

	if paths[0] == _pciWakeupDevicePath {
		return true
	}

	return false
}

func getSupportAcpiDeviceEnable(devicePath string) string {
	paths := strings.Split(devicePath, "-")
	if len(paths) != 2 {
		logger.Warningf("device path %s is invalid", devicePath)
		return "disabled"
	}

	data, err := ioutil.ReadFile(paths[0])
	if err != nil {
		logger.Warning(err)
		return "disabled"
	}

	content := string(data)
	lines := strings.Split(content, "\n")

	for _, line := range lines {
		if strings.HasPrefix(line, paths[1]) {
			arrs := strings.Fields(line)
			if len(arrs) < 3 {
				logger.Warningf("%s divide into no more than 3 groups according to spaces", line)
				return ""
			}
			if arrs[2] == "*enabled" {
				return "enabled"
			} else {
				return "disabled"
			}
		}
	}

	logger.Warningf("not found acpi device : %s info", paths[1])
	return "disabled"
}

func setSupportAcpiDeviceEnable(devicePath string) error {
	logger.Infof("set device path %s :", devicePath)

	paths := strings.Split(devicePath, "-")
	if len(paths) != 2 {
		err := fmt.Errorf("device path %s is invalid", devicePath)
		return err
	}

	if paths[0] != _pciWakeupDevicePath {
		err := fmt.Errorf("device path %s is invalid", devicePath)
		return err
	}
	logger.Infof("paths[0] %s,paths[1] %s", paths[0], paths[1])

	cmd := exec.Command("/bin/sh", "-c", "echo "+paths[1]+"> "+paths[0])
	err := cmd.Run()
	if err != nil {
		return err
	}
	return nil
}
