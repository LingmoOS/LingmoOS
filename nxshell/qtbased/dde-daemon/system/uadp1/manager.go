// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadp

import (
	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/procfs"
)

//go:generate dbusutil-gen em -type Manager

// RSA-2048
const uadpEncryptMaxSize = 256 - 11
const uadpDecryptMaxSize = 256

type Manager struct {
	service *dbusutil.Service

	ctx    *CryptoContext
	aesCtx *AesContext
	dm     *DataManager
}

func newManager(service *dbusutil.Service) *Manager {
	logger.Debugf("newManager")
	m := &Manager{
		service: service,
		ctx:     NewCryptoContext(),
		aesCtx:  NewAesContext(),
		dm:      NewDataManager(uadpDataDir),
	}

	if m.ctx.handle != nil && !m.ctx.Load(uadpKeyFile) {
		m.ctx.CreateKey()
		m.ctx.Save(uadpKeyFile)
	}

	m.dm.Load(uadpDataMap)

	return m
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func (m *Manager) start() {

}

func (m *Manager) stop() {

}

func (m *Manager) Available() (bool, *dbus.Error) {
	return m.ctx.Available(), nil
}

func (m *Manager) ListName(sender dbus.Sender) ([]string, *dbus.Error) {
	exec, err := m.getExecPath(sender)
	if err != nil {
		logger.Warning(err)
		return []string{}, dbusutil.ToError(err)
	}

	return m.dm.ListName(exec), nil
}

func (m *Manager) Set(sender dbus.Sender, name string, data []byte) *dbus.Error {
	exec, err := m.getExecPath(sender)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	aesKey := m.aesCtx.GenKey()
	encryptedData, err := m.aesCtx.Encrypt(data, aesKey)
	if err != nil {
		logger.Warning(err)
	}
	encryptedKey := m.ctx.Encrypt(aesKey)
	if len(encryptedKey) == 0 {
		encryptedKey = aesKey
	}

	err = m.dm.SetData(uadpDataDir, exec, name, encryptedKey, encryptedData)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	err = m.dm.Save(uadpDataMap)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (m *Manager) Get(sender dbus.Sender, name string) ([]byte, *dbus.Error) {
	exec, err := m.getExecPath(sender)
	if err != nil {
		logger.Warning(err)
		return []byte{}, dbusutil.ToError(err)
	}

	key, data, err := m.dm.GetData(exec, name)
	if err != nil {
		logger.Warning(err)
	}

	decryptedKey := m.ctx.Decrypt(key)
	if len(decryptedKey) == 0 {
		decryptedKey = key
	}
	decryptedData, _ := m.aesCtx.Decrypt(data, decryptedKey)

	return decryptedData, dbusutil.ToError(err)
}

func (m *Manager) Delete(sender dbus.Sender, name string) *dbus.Error {
	exec, err := m.getExecPath(sender)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	m.dm.DeleteData(exec, name)

	err = m.dm.Save(uadpDataMap)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	return nil
}

func (m *Manager) Release(sender dbus.Sender) *dbus.Error {
	exec, err := m.getExecPath(sender)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	m.dm.DeleteProcess(exec)

	err = m.dm.Save(uadpDataMap)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	return nil
}

func (m *Manager) getExecPath(sender dbus.Sender) (string, error) {
	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		return "", err
	}

	execPath, err := procfs.Process(pid).Exe()
	if err != nil {
		return "", err
	}

	return execPath, nil
}
