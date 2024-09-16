// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	accounts "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.accounts1"
	fprint "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.fprintd1"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	polkit "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.policykit1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/pam"
	"github.com/linuxdeepin/go-lib/utils"
)

//go:generate dbusutil-gen em -type Authority,PAMTransaction,FPrintTransaction

const (
	pamConfigDir             = "/etc/pam.d"
	polkitActionDoAuthorized = "org.deepin.dde.Authority1.doAuthorized"
)

func isPamServiceExist(name string) bool {
	_, err := os.Stat(filepath.Join(pamConfigDir, name))
	return err == nil
}

type Authority struct {
	service       *dbusutil.Service
	sigLoop       *dbusutil.SignalLoop
	count         uint64
	mu            sync.Mutex
	txs           map[uint64]Transaction
	fprintManager fprint.Fprintd
	dbusDaemon    ofdbus.DBus
	accounts      accounts.Accounts
}

type PolkitDetail struct {
	Message string `json:"message"`
}

func newAuthority(service *dbusutil.Service) *Authority {
	sysBus := service.Conn()
	auth := &Authority{
		service:       service,
		txs:           make(map[uint64]Transaction),
		fprintManager: fprint.NewFprintd(sysBus),
		dbusDaemon:    ofdbus.NewDBus(sysBus),
		accounts:      accounts.NewAccounts(sysBus),
		sigLoop:       dbusutil.NewSignalLoop(sysBus, 10),
	}

	auth.sigLoop.Start()
	auth.listenDBusSignals()
	return auth
}

func (*Authority) GetInterfaceName() string {
	return dbusInterface
}

var authTypeMap = map[string]string{
	"keyboard": "deepin-auth-keyboard",
}

func (a *Authority) listenDBusSignals() {
	a.dbusDaemon.InitSignalExt(a.sigLoop, true)
	_, err := a.dbusDaemon.ConnectNameOwnerChanged(func(name string, oldOwner string, newOwner string) {
		if strings.HasPrefix(name, ":") && newOwner == "" {
			var lostTxs []Transaction
			a.mu.Lock()
			for _, tx := range a.txs {
				if tx.matchSender(name) {
					logger.Debug("lost tx", name, tx.getId())
					lostTxs = append(lostTxs, tx)
				}
			}
			a.mu.Unlock()

			go func() {
				for _, tx := range lostTxs {
					_ = tx.End(dbus.Sender(name))
				}
			}()
		}
	})
	if err != nil {
		logger.Warning(err)
	}
}

const (
	authTypeFprint = "fprint"
)

func (a *Authority) Start(sender dbus.Sender, authType, user string,
	agent dbus.ObjectPath) (transaction dbus.ObjectPath, busErr *dbus.Error) {

	a.service.DelayAutoQuit()
	if !agent.IsValid() {
		return "/", dbusutil.ToError(errors.New("agent path is invalid"))
	}

	var path dbus.ObjectPath
	var err error
	var tx Transaction
	if authType == authTypeFprint {
		tx, path, err = a.StartFPrint(sender, user, agent)
	} else {
		tx, path, err = a.StartPAM(sender, authType, user, agent)
	}
	if err != nil {
		return "/", dbusutil.ToError(err)
	}
	logger.Debugf("%s start sender: %q, authType: %q, user %q, agent path: %q, tx path: %q",
		tx, sender, authType, user, agent, path)
	return path, nil
}

func (a *Authority) StartFPrint(sender dbus.Sender, user string, agent dbus.ObjectPath) (Transaction,
	dbus.ObjectPath, error) {

	a.mu.Lock()
	id := a.count
	a.count++
	a.mu.Unlock()

	tx := &FPrintTransaction{
		Sender: string(sender),
		baseTransaction: baseTransaction{
			authType: authTypeFprint,
			id:       id,
			parent:   a,
			user:     user,
		},
	}

	tx.agent = a.service.Conn().Object(string(sender), agent)
	path := getTxObjPath(id)
	err := a.service.Export(path, tx)
	if err != nil {
		return nil, "/", err
	}

	a.mu.Lock()
	a.txs[id] = tx
	a.mu.Unlock()

	return tx, path, nil
}

func (a *Authority) StartPAM(sender dbus.Sender, authType, user string, agent dbus.ObjectPath) (Transaction,
	dbus.ObjectPath, error) {

	var tx *PAMTransaction
	pamService, ok := authTypeMap[authType]
	if !ok {
		return nil, "/", errors.New("invalid auth type")
	}
	if !isPamServiceExist(pamService) {
		return nil, "/", fmt.Errorf("pam service %q not exist", pamService)
	}

	tx, err := a.startPAMTx(authType, pamService, user, string(sender))
	if err != nil {
		return nil, "/", err
	}

	tx.agent = a.service.Conn().Object(string(sender), agent)
	path := getTxObjPath(tx.id)
	err = a.service.Export(path, tx)
	if err != nil {
		return nil, "/", err
	}
	return tx, path, nil
}

func (a *Authority) startPAMTx(authType, service, user, sender string) (*PAMTransaction, error) {
	a.mu.Lock()
	id := a.count
	a.count++
	a.mu.Unlock()

	tx := &PAMTransaction{
		Sender: sender,
		baseTransaction: baseTransaction{
			authType: authType,
			id:       id,
			parent:   a,
			user:     user,
		},
	}

	pamTx, err := pam.Start(service, user, tx)
	if err != nil {
		return nil, err
	}
	tx.core = pamTx

	a.mu.Lock()
	a.txs[id] = tx
	a.mu.Unlock()

	return tx, nil
}

func (a *Authority) CheckCookie(user, cookie string) (result bool, authToken string, busErr *dbus.Error) {
	a.service.DelayAutoQuit()
	if user == "" || cookie == "" {
		return false, "", nil
	}

	a.mu.Lock()
	defer a.mu.Unlock()

	for _, tx := range a.txs {
		user0, cookie0 := tx.getUserCookie()
		if cookie == cookie0 && user == user0 {
			authToken := tx.getAuthToken()
			tx.clearSecret()
			logger.Debug("CheckCookie success", user)
			return true, authToken, nil
		}
	}
	return false, "", nil
}

func (a *Authority) CheckAuth(sender dbus.Sender, details string) *dbus.Error {
	ret, err := checkAuthByPolkit(polkitActionDoAuthorized, details, string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}
	if !ret.IsAuthorized {
		inf, err := getDetailsKey(ret.Details, "polkit.dismissed")
		if err == nil {
			if dismiss, ok := inf.(string); ok {
				if dismiss != "" {
					return dbusutil.ToError(errors.New(""))
				}
			}
		}
		return dbusutil.ToError(errors.New("policykit authentication failed"))
	}
	return nil
}

func checkAuthByPolkit(actionId string, details string, sysBusName string) (ret polkit.AuthorizationResult, err error) {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return
	}
	authority := polkit.NewAuthority(systemBus)
	subject := polkit.MakeSubject(polkit.SubjectKindSystemBusName)
	subject.SetDetail("name", sysBusName)

	var polkitDetail PolkitDetail
	err = json.Unmarshal([]byte(details), &polkitDetail)
	if err != nil {
		return
	}

	detail := map[string]string{
		"polkit.message": polkitDetail.Message,
		"exAuth":         "true",
		"exAuthFlags":    "1",
	}

	ret, err = authority.CheckAuthorization(0, subject,
		actionId, detail,
		polkit.CheckAuthorizationFlagsAllowUserInteraction, "")
	if err != nil {
		logger.Warningf("call check auth failed, err: %v", err)
		return
	}
	logger.Debugf("call check auth success, ret: %v", ret)
	return
}

func getDetailsKey(details map[string]dbus.Variant, key string) (interface{}, error) {
	result, ok := details[key]
	if !ok {
		return nil, errors.New("key dont exist in details")
	}
	if utils.IsInterfaceNil(result) {
		return nil, errors.New("result is nil")
	}
	return result.Value(), nil
}

func (a *Authority) HasCookie(user string) (result bool, busErr *dbus.Error) {
	a.service.DelayAutoQuit()
	if user == "" {
		return false, nil
	}

	a.mu.Lock()
	defer a.mu.Unlock()

	for _, tx := range a.txs {
		user0, cookie := tx.getUserCookie()
		if cookie != "" && user == user0 {
			return true, nil
		}
	}
	return false, nil
}

func (a *Authority) deleteTx(id uint64) {
	logger.Debug("deleteTx", id)
	a.mu.Lock()
	defer a.mu.Unlock()

	tx := a.txs[id]
	if tx == nil {
		return
	}

	time.AfterFunc(100*time.Millisecond, func() {
		impl := tx.(dbusutil.Implementer)
		err := a.service.StopExport(impl)
		if err != nil {
			logger.Warning(err)
		}
	})
	delete(a.txs, id)
}

func (a *Authority) getUserLocale(username string) (string, error) {
	user, err := a.accounts.FindUserByName(0, username)
	if err != nil {
		return "", err
	}
	userPath := dbus.ObjectPath(user)
	sysBus := a.service.Conn()
	userObj, err := accounts.NewUser(sysBus, userPath)
	if err != nil {
		return "", err
	}

	locale, err := userObj.Locale().Get(0)
	return locale, err
}

func (a *Authority) releaseFprintTransaction(ignoreTxId uint64, devPath dbus.ObjectPath) {
	a.mu.Lock()
	defer a.mu.Unlock()

	for txId, tx := range a.txs {
		if txId == ignoreTxId {
			continue
		}

		fpTx, ok := tx.(*FPrintTransaction)
		if !ok {
			continue
		}

		fpTx.mu.Lock()
		if fpTx.devicePath == devPath {
			fpTx.devicePath = ""
			fpTx.mu.Unlock()
			return
		}
		fpTx.mu.Unlock()
	}
}
