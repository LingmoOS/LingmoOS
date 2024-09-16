// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package lastore

/*
#include <stdlib.h>
#include <sys/statvfs.h>
*/
import "C"

import (
	"sort"
	"unsafe"
)

func strSliceSetEqual(a, b []string) bool {
	if len(a) != len(b) {
		return false
	}
	sort.Strings(a)
	sort.Strings(b)
	for i, va := range a {
		if va != b[i] {
			return false
		}
	}
	return true
}

func queryVFSAvailable(path string) (uint64, error) {
	var vfsStat C.struct_statvfs
	path0 := C.CString(path)
	_, err := C.statvfs(path0, &vfsStat)
	C.free(unsafe.Pointer(path0))
	if err != nil {
		return 0, err
	}
	avail := uint64(vfsStat.f_bavail) * uint64(vfsStat.f_bsize)
	return avail, nil
}
