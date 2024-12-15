// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

import (
	"encoding/json"
	"sync"

	"github.com/godbus/dbus/v5"
)

type AdapterInfo struct {
	Address             string
	Path                dbus.ObjectPath
	Name                string
	Alias               string
	Powered             bool
	Discovering         bool
	Discoverable        bool
	DiscoverableTimeout uint32
}

func unmarshalAdapterInfo(data string) (*AdapterInfo, error) {
	var adapter AdapterInfo
	err := json.Unmarshal([]byte(data), &adapter)
	if err != nil {
		return nil, err
	}
	return &adapter, nil
}

type AdapterInfos struct {
	mu    sync.Mutex
	infos []AdapterInfo
}

func (a *AdapterInfos) getAdapter(path dbus.ObjectPath) (int, *AdapterInfo) {
	a.mu.Lock()
	defer a.mu.Unlock()
	return a.getAdapterNoLock(path)
}

func (a *AdapterInfos) getAdapterNoLock(path dbus.ObjectPath) (int, *AdapterInfo) {
	for idx, info := range a.infos {
		if info.Path == path {
			return idx, &info
		}
	}
	return -1, nil
}

func (a *AdapterInfos) addOrUpdateAdapter(adapterInfo *AdapterInfo) {
	a.mu.Lock()
	defer a.mu.Unlock()

	idx, _ := a.getAdapterNoLock(adapterInfo.Path)
	if idx != -1 {
		// 更新
		a.infos[idx] = *adapterInfo
		return
	}
	a.infos = append(a.infos, *adapterInfo)
}

func (a *AdapterInfos) removeAdapter(path dbus.ObjectPath) {
	a.mu.Lock()
	defer a.mu.Unlock()

	idx, _ := a.getAdapterNoLock(path)
	if idx == -1 {
		return
	}
	a.infos = append(a.infos[:idx], a.infos[idx+1:]...)
}

func (a *AdapterInfos) toJSON() string {
	a.mu.Lock()
	defer a.mu.Unlock()
	return marshalJSON(a.infos)
}

func (a *AdapterInfos) clear() {
	a.mu.Lock()
	a.infos = nil
	a.mu.Unlock()
}
