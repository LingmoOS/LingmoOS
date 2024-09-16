// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package uadpagent

import (
	"crypto/rand"
	"encoding/base64"
	"errors"
	"fmt"
	"os"
	"sync"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/session/common"
	secrets "github.com/linuxdeepin/go-dbus-factory/session/org.freedesktop.secrets"
	uadp "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.uadp1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/procfs"
)

//go:generate dbusutil-gen em -type UadpAgent

var logger = log.NewLogger("daemon/session/UadpAgent")

const (
	dbusServiceName = "org.deepin.dde.UadpAgent1"
	dbusPath        = "/org/deepin/dde/UadpAgent1"
	dbusInterface   = dbusServiceName
)

const (
	userUid = "userUid"
)

func (*UadpAgent) GetInterfaceName() string {
	return dbusInterface
}

type UadpAgent struct {
	service             *dbusutil.Service
	secretService       secrets.Service
	secretSessionPath   dbus.ObjectPath
	defaultCollection   secrets.Collection
	defaultCollectionMu sync.Mutex
	uadpDaemon          uadp.Uadp // 提供加解密接口
	keyringKey          string    // 密钥缓存
	mu                  sync.Mutex
}

func newUadpAgent(service *dbusutil.Service) (*UadpAgent, error) {
	sessionBus := service.Conn()
	secretsObj := secrets.NewService(sessionBus)
	_, sessionPath, err := secretsObj.OpenSession(0, "plain", dbus.MakeVariant(""))
	if err != nil {
		logger.Warning("failed to get sessionPath:", err)
		return nil, err
	}
	sysBus, err := dbus.SystemBus()
	if err != nil {
		return nil, err
	}
	uadpDaemon := uadp.NewUadp(sysBus)
	u := &UadpAgent{
		service:           service,
		secretService:     secretsObj,
		secretSessionPath: sessionPath,
		uadpDaemon:        uadpDaemon,
	}
	err = common.ActivateSysDaemonService(u.uadpDaemon.ServiceName_())
	if err != nil {
		logger.Warning(err)
	}
	return u, nil

}

// 提供给应用调用,用户通过此接口存储密钥
func (u *UadpAgent) SetDataKey(sender dbus.Sender, keyName string, dataKey string) *dbus.Error {
	executablePath, keyringKey, err := u.getExePathAndKeyringKey(sender, true)
	if err != nil {
		logger.Warning("failed to get exePath and keyringKey:", err)
		return dbusutil.ToError(err)
	}
	// 将用户希望存储的密钥加密存储
	logger.Debug("begin to set data key")
	err = u.uadpDaemon.SetDataKey(0, executablePath, keyName, dataKey, keyringKey)
	if err != nil {
		logger.Warning("failed to save data key:", err)
		return dbusutil.ToError(err)
	}
	return nil
}

// 提供给用户调用,用户通过此接口获取密钥
func (u *UadpAgent) GetDataKey(sender dbus.Sender, keyName string) (dataKey string, busErr *dbus.Error) {
	executablePath, keyringKey, err := u.getExePathAndKeyringKey(sender, false)
	if err != nil {
		logger.Warning("failed to get exePath and keyringKey:", err)
		return "", dbusutil.ToError(err)
	}
	// 将密钥解密,并返回给用户
	dataKey, err = u.uadpDaemon.GetDataKey(0, executablePath, keyName, keyringKey)
	if err != nil {
		logger.Warning("failed to get data key:", err)
		return "", dbusutil.ToError(err)
	}
	return dataKey, nil
}

// 获取调用者二进制可执行文件路径和用于加解密数据的密钥
func (u *UadpAgent) getExePathAndKeyringKey(sender dbus.Sender, createIfNotExist bool) (string, string, error) {
	executablePath, err := u.getExePath(sender)
	if err != nil {
		return "", "", err
	}

	keyringKey, err := u.getKeyringKey(os.Getuid(), createIfNotExist)
	if err != nil {
		return "", "", err
	}
	return executablePath, keyringKey, nil
}

func (u *UadpAgent) getKeyringKey(uid int, createIfNotExist bool) (string, error) {
	u.mu.Lock()
	defer u.mu.Unlock()
	var err error
	if u.keyringKey == "" {
		u.keyringKey, err = u.getUserKeyringKey(uid)
		if err != nil {
			logger.Warning(err)
		}
		if u.keyringKey == "" && !createIfNotExist {
			return "", errors.New("keyringKey is not exist")
		}
		if u.keyringKey == "" {
			randKey, err := getRandKey(16)
			if err != nil {
				logger.Warning(err)
				return "", err
			}
			err = u.saveExeKeyringKey(uid, randKey)
			if err != nil {
				return "", err
			}
			u.keyringKey = randKey
		}
	}

	return u.keyringKey, nil
}

func (u *UadpAgent) getExePath(sender dbus.Sender) (string, error) {
	pid, err := u.service.GetConnPID(string(sender))
	if err != nil {
		logger.Warning("failed to get PID:", err)
		return "", err
	}

	process := procfs.Process(pid)
	executablePath, err := process.Exe()
	if err != nil {
		logger.Warning("failed to get executablePath:", err)
		return "", err
	}
	return executablePath, nil
}

func (u *UadpAgent) saveExeKeyringKey(uid int, keyringKey string) error {
	label := fmt.Sprintf("UadpAgent code/decode secret for %d", uid)
	logger.Debugf("set label: %q, uid: %d, keyringKey: %q", label, uid, keyringKey)
	itemSecret := secrets.Secret{
		Session:     u.secretSessionPath,
		Value:       []byte(keyringKey),
		ContentType: "text/plain",
	}

	properties := map[string]dbus.Variant{
		"org.freedesktop.Secret.Item.Label": dbus.MakeVariant(label),
		"org.freedesktop.Secret.Item.Type":  dbus.MakeVariant("org.freedesktop.Secret.Generic"),
		"org.freedesktop.Secret.Item.Attributes": dbus.MakeVariant(map[string]string{
			userUid: fmt.Sprintf("%d", uid),
		}),
	}

	defaultCollection, err := u.getDefaultCollection()
	if err != nil {
		logger.Warning("failed to get defaultCollection:", err)
		return err
	}

	_, _, err = defaultCollection.CreateItem(0, properties, itemSecret, true)

	return err
}

func (u *UadpAgent) getUserKeyringKey(uid int) (string, error) {
	attributes := map[string]string{
		userUid: fmt.Sprintf("%d", uid),
	}

	defaultCollection, err := u.getDefaultCollection()
	if err != nil {
		logger.Warning("failed to get defaultCollection:", err)
		return "", err
	}
	items, err := defaultCollection.SearchItems(0, attributes)
	if err != nil {
		logger.Warning("failed to get items:", err)
		return "", err
	}

	secretData, err := u.secretService.GetSecrets(0, items, u.secretSessionPath)
	if err != nil {
		logger.Warning("failed to get secretData:", err)
		return "", err
	}
	var keyringKey string
	sessionBus, err := dbus.SessionBus()
	if err != nil {
		logger.Warning(err)
		return "", err
	}
	for itemPath, secret := range secretData {
		itemObj, err := secrets.NewItem(sessionBus, itemPath)
		if err != nil {
			logger.Warning(err)
			return "", err
		}
		attributes, _ := itemObj.Attributes().Get(0)
		if attributes[userUid] != "" {
			keyringKey = string(secret.Value)
		}
	}
	return keyringKey, nil
}

func (u *UadpAgent) getDefaultCollection() (secrets.Collection, error) {
	u.defaultCollectionMu.Lock()
	defer u.defaultCollectionMu.Unlock()

	if u.defaultCollection != nil {
		return u.defaultCollection, nil
	}

	cPath, err := u.secretService.ReadAlias(0, "default")
	if err != nil {
		logger.Warning("failed to get collectionPath:", err)
		return nil, err
	}

	if cPath == "/" {
		return nil, errors.New("failed to get default collection path")
	}

	sessionBus, err := dbus.SessionBus()
	if err != nil {
		return nil, err
	}

	collectionObj, err := secrets.NewCollection(sessionBus, cPath)
	if err != nil {
		logger.Warning(err)
		return nil, err
	}
	u.defaultCollection = collectionObj
	return collectionObj, nil
}

func getRandKey(length int) (string, error) {
	b := make([]byte, length)
	if _, err := rand.Read(b); err != nil {
		logger.Warning(err)
		return "", err
	}
	return base64.URLEncoding.EncodeToString(b), nil
}
