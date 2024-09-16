// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

// #cgo CFLAGS: -W -Wall -D_GNU_SOURCE
// #include "reminder_info.h"
// #include <string.h>
// #include <utmpx.h>
// #include <sys/time.h>
// #include <shadow.h>
// #include <stdio.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
import "C"
import (
	"fmt"
	"time"
	"unsafe"
)

type LoginUtmpx struct {
	InittabID string
	Line      string
	Host      string
	Address   string
	Time      string
}

type LoginReminderInfo struct {
	Username string
	Spent    struct {
		LastChange int
		Min        int
		Max        int
		Warn       int
		Inactive   int
		Expire     int
	}
	CurrentLogin            LoginUtmpx
	LastLogin               LoginUtmpx
	FailCountSinceLastLogin int
}

func getLoginReminderInfo(user string) (res LoginReminderInfo) {
	res.Username = user

	C.setspent()
	var spent C.struct_spwd = *C.getspnam(C.CString(user))
	C.endspent()

	res.Spent.LastChange = int(spent.sp_lstchg)
	res.Spent.Min = int(spent.sp_min)
	res.Spent.Max = int(spent.sp_max)
	res.Spent.Warn = int(spent.sp_warn)
	res.Spent.Inactive = int(spent.sp_inact)
	res.Spent.Expire = int(spent.sp_expire)

	var current C.struct_utmpx
	var last C.struct_utmpx

	C.count_utmpx(C.CString(C.WTMPX_FILE), C.CString(user), nil, &current, &last)

	var last_tv C.struct_timeval
	var last_tv_p *C.struct_timeval
	if last.ut_type != C.EMPTY {
		last_tv.tv_sec = C.long(last.ut_tv.tv_sec)
		last_tv.tv_usec = C.long(last.ut_tv.tv_usec)
		last_tv_p = &last_tv
	}
	res.FailCountSinceLastLogin = int(C.count_utmpx(C.CString(C.BTMPX_FILE), C.CString(user), last_tv_p, nil, nil))

	res.CurrentLogin = genLoginUtmpx(current)
	res.LastLogin = genLoginUtmpx(last)

	return
}

func genLoginUtmpx(last C.struct_utmpx) (res LoginUtmpx) {
	res.InittabID = fmt.Sprintf("%.4s", C.GoString(&(last.ut_id[0])))
	res.Line = C.GoString(&(last.ut_line[0]))
	res.Host = C.GoString(&(last.ut_host[0]))

	var buffer [C.INET6_ADDRSTRLEN]C.char
	if last.ut_addr_v6[1] != 0 || last.ut_addr_v6[2] != 0 || last.ut_addr_v6[3] != 0 {
		res.Address = C.GoString(C.inet_ntop(C.AF_INET6, unsafe.Pointer(&(last.ut_addr_v6[0])), &buffer[0], C.INET6_ADDRSTRLEN))
	} else {
		res.Address = C.GoString(C.inet_ntop(C.AF_INET, unsafe.Pointer(&(last.ut_addr_v6[0])), &buffer[0], C.INET6_ADDRSTRLEN))
	}

	res.Time = time.Unix(int64(last.ut_tv.tv_sec), 0).Local().String()

	return
}
