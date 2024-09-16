// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package zoneinfo

// #cgo CFLAGS: -W -Wall -g -fstack-protector-all -fPIC
// #include <stdlib.h>
// #include "timestamp.h"
import "C"

import (
	"time"
	"unsafe"

	"strings"

	. "github.com/linuxdeepin/go-lib/gettext"
)

func getDSTTime(zone string, year int32) (int64, int64, bool) {
	czone := C.CString(zone)
	defer C.free(unsafe.Pointer(czone))
	ret := C.get_dst_time(czone, C.int(year))

	t1 := int64(ret.enter)
	t2 := int64(ret.leave)

	if t1 == 0 || t2 == 0 {
		return 0, 0, false
	}

	return t1, t2, true
}

func getOffsetByUSec(zone string, timestamp int64) int32 {
	czone := C.CString(zone)
	defer C.free(unsafe.Pointer(czone))
	offset := C.get_offset_by_usec(czone, C.longlong(timestamp))

	return int32(offset)
}

func getRawUSec(zone string, timestamp int64) int64 {
	czone := C.CString(zone)
	defer C.free(unsafe.Pointer(czone))
	ret := C.get_rawoffset_usec(czone, C.longlong(timestamp))

	return int64(ret)
}

func newDSTInfo(zone string) *DSTInfo {
	year := time.Now().Year()
	first, second, ok := getDSTTime(zone, int32(year))
	if !ok {
		return nil
	}
	offset := getOffsetByUSec(zone, first)

	return &DSTInfo{
		Enter:  first,
		Leave:  second,
		Offset: offset,
	}
}

func newZoneInfo(zone string) *ZoneInfo {
	var info ZoneInfo

	info.Name = zone
	info.Desc = DGettext("deepin-installer-timezones", zone)

	tokens := strings.Split(info.Desc, "/")
	if len(tokens) == 2 {
		info.Desc = tokens[1]
	}

	dst := newDSTInfo(zone)
	if dst == nil {
		info.Offset = getOffsetByUSec(zone, 0)
	} else {
		info.Offset = getOffsetByUSec(zone,
			getRawUSec(zone, dst.Enter))
		info.DST = *dst
	}

	return &info
}
