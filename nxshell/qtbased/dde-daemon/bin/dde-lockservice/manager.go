// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package main

import (
	"errors"
	"fmt"
	"log"
	"runtime/debug"
	"strconv"
	"sync"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/msteinert/pam"
)

const (
	PromptQuestion uint32 = iota + 1
	PromptSecret
	ErrorMsg
	TextInfo
	Failure
	Success
)

//go:generate dbusutil-gen em -type Manager

type Manager struct {
	service       *dbusutil.Service
	authLocker    sync.Mutex
	authUserTable map[string]chan string // 'pid+user': 'password'

	signals *struct { //nolint
		Event struct {
			eventType uint32
			pid       uint32
			username  string
			message   string
		}

		UserChanged struct {
			username string
		}
	}
}

const (
	dbusServiceName = "org.deepin.dde.LockService1"
	dbusPath        = "/org/deepin/dde/LockService1"
	dbusInterface   = dbusServiceName
)

var _m *Manager

func init() {
	log.SetFlags(log.Lshortfile)
}

func main() {
	service, err := dbusutil.NewSystemService()
	if err != nil {
		log.Fatal("failed to new system service:", err)
	}

	_m = newManager(service)
	err = service.Export(dbusPath, _m)
	if err != nil {
		log.Fatal("failed to export:", err)
	}

	err = service.RequestName(dbusServiceName)
	if err != nil {
		log.Fatal("failed to request name:", err)
	}

	service.SetAutoQuitHandler(time.Minute*2, func() bool {
		_m.authLocker.Lock()
		canQuit := len(_m.authUserTable) == 0
		_m.authLocker.Unlock()
		return canQuit
	})
	service.Wait()
}

func (*Manager) GetInterfaceName() string {
	return dbusInterface
}

func newManager(service *dbusutil.Service) *Manager {
	var m = Manager{
		authUserTable: make(map[string]chan string),
		service:       service,
	}

	return &m
}

func (m *Manager) CurrentUser() (username string, busErr *dbus.Error) {
	username, err := getGreeterUser(greeterUserConfig)
	if err != nil {
		return "", dbusutil.ToError(errors.New("get greeter user failed"))
	}
	return username, nil
}

func (m *Manager) IsLiveCD(username string) (result bool, busErr *dbus.Error) {
	return isInLiveCD(username), nil
}

func (m *Manager) SwitchToUser(username string) *dbus.Error {
	current, _ := getGreeterUser(greeterUserConfig)
	if current == username {
		return nil
	}

	err := setGreeterUser(greeterUserConfig, username)
	if err != nil {
		return dbusutil.ToError(errors.New("set greeter user failed"))
	}
	if current != "" {
		err = m.service.Emit(m, "UserChanged", username)
		if err != nil {
			return dbusutil.ToError(err)
		}
	}
	return nil
}

func (m *Manager) AuthenticateUser(sender dbus.Sender, username string) *dbus.Error {
	if username == "" {
		return dbusutil.ToError(fmt.Errorf("no user to authenticate"))
	}

	m.authLocker.Lock()
	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}

	id := getId(pid, username)
	_, ok := m.authUserTable[id]
	if ok {
		log.Println("In authenticating:", id)
		m.authLocker.Unlock()
		return nil
	}

	m.authUserTable[id] = make(chan string, 1)
	m.authLocker.Unlock()
	go m.doAuthenticate(username, "", pid)
	return nil
}

func (m *Manager) UnlockCheck(sender dbus.Sender, username, password string) *dbus.Error {
	if username == "" {
		return dbusutil.ToError(fmt.Errorf("no user to authenticate"))
	}

	pid, err := m.service.GetConnPID(string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}
	id := getId(pid, username)

	m.authLocker.Lock()
	v, ok := m.authUserTable[id]
	if ok && v != nil {
		log.Println("In authenticating:", id)
		// in authenticate
		v <- password
		m.authLocker.Unlock()
		return nil
	}
	m.authLocker.Unlock()

	go m.doAuthenticate(username, password, pid)
	return nil
}

func getId(pid uint32, username string) string {
	return strconv.Itoa(int(pid)) + username
}

func (m *Manager) doAuthenticate(username, password string, pid uint32) {
	handler, err := pam.StartFunc("lightdm", username, func(style pam.Style, msg string) (string, error) {
		switch style {
		// case pam.PromptEchoOn:
		// 	if msg != "" {
		// 		fmt.Println("Echo on:", msg)
		// 		m.sendEvent(PromptQuestion, pid, username, msg)
		// 	}
		// 	// TODO: read data from input
		// 	return "", nil
		case pam.PromptEchoOff, pam.PromptEchoOn:
			if password != "" {
				tmp := password
				password = ""
				return tmp, nil
			}

			if msg != "" {
				if style == pam.PromptEchoOff {
					log.Println("Echo off:", msg)
					m.sendEvent(PromptSecret, pid, username, msg)
				} else {
					log.Println("Echo on:", msg)
					m.sendEvent(PromptQuestion, pid, username, msg)
				}
			}

			id := getId(pid, username)
			m.authLocker.Lock()
			v, ok := m.authUserTable[id]
			m.authLocker.Unlock()
			if !ok || v == nil {
				log.Println("WARN: no password channel found for", username)
				return "", fmt.Errorf("no passwd channel found for %s", username)
			}
			log.Println("Join select:", id)

			tmp, ok := <-v
			if !ok {
				log.Println("Invalid select channel")
				return "", nil
			}

			m.authLocker.Lock()
			delete(m.authUserTable, id)
			close(v)
			m.authLocker.Unlock()
			return tmp, nil
		case pam.ErrorMsg:
			if msg != "" {
				log.Println("ShowError:", msg)
				m.sendEvent(ErrorMsg, pid, username, msg)
			}
			return "", nil
		case pam.TextInfo:
			if msg != "" {
				log.Println("Text info:", msg)
				m.sendEvent(TextInfo, pid, username, msg)
			}
			return "", nil
		}
		return "", fmt.Errorf("unexpected style: %v", style)
	})
	if err != nil {
		log.Println("Failed to start pam:", err)
		m.sendEvent(Failure, pid, username, err.Error())
		return
	}

	// meet the requirement of pam_unix.so nullok_secure option,
	// allows any user with a blank password to unlock.
	err = handler.SetItem(pam.Tty, "tty1")
	if err != nil {
		log.Println("WARN: failed to set item tty:", err)
	}

	err = handler.Authenticate(0)

	id := getId(pid, username)
	m.authLocker.Lock()
	v, ok := m.authUserTable[id]
	if ok {
		close(v)
		delete(m.authUserTable, id)
	}
	m.authLocker.Unlock()
	if err != nil {
		log.Println("Failed to authenticate:", err)
		m.sendEvent(Failure, pid, username, err.Error())
	} else {
		log.Println("Authenticate success")
		m.sendEvent(Success, pid, username, "Authenticated")
	}
	handler = nil
	debug.FreeOSMemory()
}

func (m *Manager) sendEvent(ty, pid uint32, username, msg string) {
	err := m.service.Emit(m, "Event", ty, pid, username, msg)
	if err != nil {
		log.Println("Failed to emit event:", ty, pid, username, msg)
	}
}
