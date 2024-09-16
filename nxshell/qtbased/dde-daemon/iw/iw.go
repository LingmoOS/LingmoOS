// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package iw

// #cgo CFLAGS: -W -Wall -Wall -g -fstack-protector-all -fPIC
// #cgo pkg-config: libnl-3.0 libnl-genl-3.0
// #include <stdlib.h>
// #include "core.h"
import "C"

import (
	"fmt"
	"io/ioutil"
	"strings"
	"unsafe"

	"github.com/linuxdeepin/go-lib/strv"
)

type WirelessInfo struct {
	Wiphy     string
	HwAddress string
	IFCModes  []string
}
type WirelessInfos []*WirelessInfo

var wirelessSets = make(map[string][]string)

func ListWirelessInfo() (WirelessInfos, error) {
	ret := C.wireless_info_query()
	if int(ret) != 0 {
		fmt.Println("Failed to query wireless info")
		return nil, fmt.Errorf("Query wireless info failed")
	}

	var infos WirelessInfos
	for phy, modes := range wirelessSets {
		infos = append(infos, &WirelessInfo{
			Wiphy:     phy,
			HwAddress: getHwAddressByFile(hwAddressFile(phy)),
			IFCModes:  modes,
		})
	}
	return infos, nil
}

func (infos WirelessInfos) ListMiracastDevice() WirelessInfos {
	var ret WirelessInfos
	for _, info := range infos {
		if !info.SupportedMiracast() {
			continue
		}
		ret = append(ret, info)
	}
	return ret
}

func (infos WirelessInfos) ListHotspotDevice() WirelessInfos {
	var ret WirelessInfos
	for _, info := range infos {
		if !info.SupportedHotspot() {
			continue
		}
		ret = append(ret, info)
	}
	return ret
}

func (infos WirelessInfos) Get(address string) *WirelessInfo {
	for _, info := range infos {
		if strings.EqualFold(info.HwAddress, address) {
			return info
		}
	}
	return nil
}

func (info WirelessInfo) SupportedHotspot() bool {
	return strv.Strv(info.IFCModes).Contains("AP")
}

func (info *WirelessInfo) SupportedMiracast() bool {
	list := strv.Strv(info.IFCModes)
	return list.Contains("P2P-client") &&
		list.Contains("P2P-GO")
	// list.Contains("P2P-device")
}

//export addWirelessInfo
func addWirelessInfo(cname, cmode *C.char) {
	name := C.GoString(cname)
	mode := C.GoString(cmode)
	C.free(unsafe.Pointer(cmode))

	modes, ok := wirelessSets[name]
	if !ok {
		wirelessSets[name] = []string{mode}
		return
	}

	modes = append(modes, mode)
	wirelessSets[name] = modes
}

func hwAddressFile(name string) string {
	return "/sys/class/ieee80211/" + name + "/macaddress"
}

func getHwAddressByFile(file string) string {
	contents, err := ioutil.ReadFile(file)
	if err != nil {
		return ""
	}

	return strings.TrimSpace(string(contents))
}
