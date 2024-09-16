// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package timedate

import (
	"errors"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gettext"
)

const (
	dbusFormatPath        = "/org/deepin/dde/Format"
	dbusFormatInterface   = "org.deepin.dde.Format"
	configManagerId       = "org.desktopspec.ConfigManager"
	dbusFormatServiceName = dbusFormatInterface
)

//go:generate dbusutil-gen -type ManagerFormat manager_format.go
//go:generate dbusutil-gen em -type ManagerFormat

// Manage format settings
type ManagerFormat struct {
	service       *dbusutil.Service
	systemSigLoop *dbusutil.SignalLoop
	PropsMu       sync.RWMutex

	// dsg config
	CurrencySymbol         string `prop:"access:rw"`
	PositiveCurrencyFormat string `prop:"access:rw"`
	NegativeCurrencyFormat string `prop:"access:rw"`
	DecimalSymbol          string `prop:"access:rw"`
	DigitGroupingSymbol    string `prop:"access:rw"`
	DigitGrouping          string `prop:"access:rw"`

	configManagerPath dbus.ObjectPath
}

// Create Manager, if create freedesktop format failed return error
func newManagerFormat(service *dbusutil.Service) (*ManagerFormat, error) {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}

	var m = &ManagerFormat{
		service: service,
	}

	m.systemSigLoop = dbusutil.NewSignalLoop(systemBus, 10)

	// 加载dsg配置
	systemConnObj := systemBus.Object(configManagerId, "/")
	err = systemConnObj.Call(configManagerId+".acquireManager", 0, "org.deepin.dde.daemon", "org.deepin.dde.daemon.format", "").Store(&m.configManagerPath)
	if err != nil {
		logger.Warning(err)
	}

	err = dbusutil.NewMatchRuleBuilder().Type("signal").
		PathNamespace(string(m.configManagerPath)).
		Interface("org.desktopspec.ConfigManager.Manager").
		Member("valueChanged").Build().AddTo(systemBus)
	if err != nil {
		logger.Warning(err)
	}

	//初始化的时候，根据环境变量修改dsg中"Space"的翻译内容
	digitGroupingSymbol := m.getDsgData("digitGroupingSymbol")
	decimalSymbol := m.getDsgData("decimalSymbol")
	space := gettext.Tr("Space")
	logger.Infof(" [newManagerFormat] space : %v, digitGroupingSymbol : %v", space, digitGroupingSymbol)
	if digitGroupingSymbol == string("Space") || digitGroupingSymbol == space {
		if m.setPropDigitGroupingSymbol(space) {
			m.setDsgData("digitGroupingSymbol", space)
		}
	}

	logger.Infof(" [newManagerFormat] space : %v, decimalSymbol : %v", space, decimalSymbol)
	if decimalSymbol == string("Space") || decimalSymbol == space {
		if m.setPropDecimalSymbol(space) {
			m.setDsgData("decimalSymbol", space)
		}
	}

	logger.Infof(" [newManagerFormat] digitGroupingSymbol : %v, decimalSymbol : %v", digitGroupingSymbol, decimalSymbol)
	return m, nil
}

func (m *ManagerFormat) init() {
	m.setPropValue()
	m.systemSigLoop.Start()
	go m.listenDsgPropChanged()
}

func (m *ManagerFormat) setPropValue() {
	m.PropsMu.Lock()
	m.setPropCurrencySymbol(m.getDsgData("currencySymbol"))
	m.setPropDecimalSymbol(m.getDsgData("decimalSymbol"))
	m.setPropDigitGrouping(m.getDsgData("digitGrouping"))
	m.setPropDigitGroupingSymbol(m.getDsgData("digitGroupingSymbol"))
	m.setPropNegativeCurrencyFormat(m.getDsgData("negativeCurrencyFormat"))
	m.setPropPositiveCurrencyFormat(m.getDsgData("positiveCurrencyFormat"))
	m.PropsMu.Unlock()
}

func (m *ManagerFormat) initPropertyWriteCallback(service *dbusutil.Service) error {
	logger.Debug("nitPropertyWriteCallback.")
	obj := service.GetServerObject(m)
	err := obj.SetWriteCallback(m, "CurrencySymbol", m.setWriteCurrencySymbolCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "DecimalSymbol", m.setWriteDecimalSymbolCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "DigitGrouping", m.setWriteDigitGroupingCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "DigitGroupingSymbol", m.setWriteDigitGroupingSymbolCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "NegativeCurrencyFormat", m.setWriteNegativeCurrencyFormatCb)
	if err != nil {
		return err
	}
	err = obj.SetWriteCallback(m, "PositiveCurrencyFormat", m.setWritePositiveCurrencyFormatCb)
	if err != nil {
		return err
	}
	return nil
}

func (m *ManagerFormat) setWriteCurrencySymbolCb(write *dbusutil.PropertyWrite) *dbus.Error {
	curValue, ok := write.Value.(string)
	logger.Info("setPropCurrencySymbol value, ok : ", curValue, " , ok : ", ok)
	if !ok {
		err := errors.New("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	if m.setPropCurrencySymbol(curValue) {
		m.setDsgData("currencySymbol", curValue)
	}
	return nil
}

func (m *ManagerFormat) setWriteDecimalSymbolCb(write *dbusutil.PropertyWrite) *dbus.Error {
	curValue, ok := write.Value.(string)
	logger.Info("setWriteDecimalSymbolCb value, ok : ", curValue, " , ok : ", ok)
	if !ok {
		err := errors.New("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	if m.setPropDecimalSymbol(curValue) {
		m.setDsgData("decimalSymbol", curValue)
	}
	return nil
}

func (m *ManagerFormat) setWriteDigitGroupingCb(write *dbusutil.PropertyWrite) *dbus.Error {
	curValue, ok := write.Value.(string)
	logger.Info("setWriteDigitGroupingCb value, ok : ", curValue, " , ok : ", ok)
	if !ok {
		err := errors.New("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	if m.setPropDigitGrouping(curValue) {
		m.setDsgData("digitGrouping", curValue)
	}
	return nil
}

func (m *ManagerFormat) setWriteDigitGroupingSymbolCb(write *dbusutil.PropertyWrite) *dbus.Error {
	curValue, ok := write.Value.(string)
	logger.Info("setWriteDigitGroupingSymbolCb value, ok : ", curValue, " , ok : ", ok)
	if !ok {
		err := errors.New("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	if m.setPropDigitGroupingSymbol(curValue) {
		m.setDsgData("digitGroupingSymbol", curValue)
	}
	return nil
}

func (m *ManagerFormat) setWriteNegativeCurrencyFormatCb(write *dbusutil.PropertyWrite) *dbus.Error {
	curValue, ok := write.Value.(string)
	logger.Info("setWriteNegativeCurrencyFormatCb value, ok : ", curValue, " , ok : ", ok)
	if !ok {
		err := errors.New("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	if m.setPropNegativeCurrencyFormat(curValue) {
		m.setDsgData("negativeCurrencyFormat", curValue)
	}
	return nil
}

func (m *ManagerFormat) setWritePositiveCurrencyFormatCb(write *dbusutil.PropertyWrite) *dbus.Error {
	curValue, ok := write.Value.(string)
	logger.Info("setWritePositiveCurrencyFormatCb value, ok : ", curValue, " , ok : ", ok)
	if !ok {
		err := errors.New("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	if m.setPropPositiveCurrencyFormat(curValue) {
		m.setDsgData("positiveCurrencyFormat", curValue)
	}
	return nil
}

func (m *ManagerFormat) listenDsgPropChanged() {
	// 监听dsg配置变化
	m.systemSigLoop.AddHandler(&dbusutil.SignalRule{
		Name: "org.desktopspec.ConfigManager.Manager.valueChanged",
	}, func(sig *dbus.Signal) {
		if strings.Contains(string(sig.Name), "org.desktopspec.ConfigManager.Manager.valueChanged") &&
			strings.Contains(string(sig.Path), "org_deepin_dde_daemon_format") {
			logger.Info("[listenDsgPropChanged] org.desktopspec.ConfigManager.Manager.valueChanged path : ", string(sig.Path))
			m.setPropValue()
		}
	})
}

func (m *ManagerFormat) destroy() {
	m.systemSigLoop.Stop()
}

func (*ManagerFormat) GetInterfaceName() string {
	return dbusFormatInterface
}

func (m *ManagerFormat) getDsgData(key string) string {
	systemConn, err := dbus.SystemBus()
	if err != nil {
		logger.Warning("getDsgData systemConn err: ", err)
		return ""
	}
	systemConnObj := systemConn.Object("org.desktopspec.ConfigManager", m.configManagerPath)
	var value string
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.value", 0, key).Store(&value)
	if err != nil {
		logger.Warningf("getDsgData key : %s. err : %s", key, err)
		return ""
	}
	logger.Infof("getDsgData key : %s , value : %s", key, value)
	return value
}

func (m *ManagerFormat) setDsgData(key, value string) bool {
	systemConn, err := dbus.SystemBus()
	if err != nil {
		logger.Warning("setDsgData systemConn err: ", err)
		return false
	}
	systemConnObj := systemConn.Object("org.desktopspec.ConfigManager", m.configManagerPath)
	err = systemConnObj.Call("org.desktopspec.ConfigManager.Manager.setValue", 0, key, dbus.MakeVariant(value)).Store()
	if err != nil {
		logger.Warningf("setDsgData key : %s. err : %s", key, err)
		return false
	}
	logger.Infof("setDsgData key : %s , value : %s", key, value)

	return true
}
