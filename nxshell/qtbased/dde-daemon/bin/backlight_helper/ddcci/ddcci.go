// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package ddcci

//#cgo CFLAGS: -W -Wall -fstack-protector-all -fPIC
//#cgo LDFLAGS:-ldl
//#cgo pkg-config: ddcutil
//#include <ddcutil_c_api.h>
//#include <ddcutil_types.h>
//#include <stdlib.h>
//#include "ddcci_wrapper.h"
import "C"
import (
	"bytes"
	"encoding/base64"
	"fmt"
	"os/exec"
	"path/filepath"
	"reflect"
	"strings"
	"sync"
	"unsafe"

	"github.com/linuxdeepin/go-lib/utils"
)

type ddcci struct {
	listPointer *C.DDCA_Display_Info_List
	listMu      sync.Mutex

	displayHandleMap map[string]*displayHandle //index -> handle
	//displayMap       map[string]int         //edidBase64 --> ddcutil monitor index
}

type displayHandle struct {
	ddcci  *ddcci
	idx    int
	edid   [128]byte
	info   *C.DDCA_Display_Info
	handle C.DDCA_Display_Handle
	val    int
	state  int
}

func (d *ddcci) newDisplayHandle(idx int) *displayHandle {
	dh := &displayHandle{
		idx: idx,
	}
	dh.info = d.getDisplayInfoByIdx(idx)
	return dh
}

func (d *displayHandle) Open() error {
	if d.state > 0 {
		return nil
	}
	status := C.ddca_open_display2(d.info.dref, C.bool(true), &d.handle)
	if status != C.int(0) {
		logger.Info("doing ddca_open_display2 error")
		return fmt.Errorf("brightness,init: failed to open monitor=%d", status)
	}
	d.state = 1
	return nil
}

func (d *displayHandle) Close() error {
	if d.state == 0 {
		return nil
	}
	C.ddca_close_display(d.handle)
	d.state = 0
	return nil
}

func (d *displayHandle) getState() int {
	return d.state
}

func (d *displayHandle) initEdid() {
	edid := C.GoBytes(unsafe.Pointer(&d.info.edid_bytes), 128)
	copy(d.edid[:], edid)
}

func (d *displayHandle) getEdidBase64() string {
	return base64.StdEncoding.EncodeToString(d.edid[:])
}

func (d *displayHandle) setBrightness(percent int) error {
	var val C.DDCA_Non_Table_Vcp_Value
	status := C.ddca_get_non_table_vcp_value(d.handle, brightnessVCP, &val)
	if status != C.int(0) {
		return fmt.Errorf("brightness: failed to get brightness: %d", status)
	}

	if int(val.sl) == percent {
		return nil
	}
	status = C.ddca_set_non_table_vcp_value(d.handle, brightnessVCP, 0, C.uchar(percent))
	if status != C.int(0) {
		return fmt.Errorf("brightness: failed to set brightness via DDC/CI: %d", status)
	}
	return nil
}

const (
	brightnessVCP = 0x10
)

func newDDCCI() (*ddcci, error) {
	ddc := &ddcci{
		displayHandleMap: make(map[string]*displayHandle),
	}

	status := C.ddca_init2((*C.char)(unsafe.Pointer(nil)), C.DDCA_SYSLOG_NOTICE, C.DDCA_INIT_OPTIONS_CLIENT_OPENED_SYSLOG, (***C.char)(unsafe.Pointer(nil)))
	if status < C.int(0) {
		return nil, fmt.Errorf("brightness: Error ddcci init: %d", status)
	}

	err := ddc.RefreshDisplays()
	if err != nil {
		return nil, err
	}

	content, err := exec.Command("/usr/bin/dpkg-architecture", "-qDEB_HOST_MULTIARCH").Output()
	if err != nil {
		// use dlopen search library when dpkg-architecture not available
		cStr := C.CString("libddcutil.so.5")
		defer C.free(unsafe.Pointer(cStr))
		ret := C.InitDDCCISo(cStr)
		if ret == -2 {
			logger.Debug("failed to initialize ddca_free_all_displays sym")
		}
	} else {
		path := filepath.Join("/usr/lib", strings.TrimSpace(string(content)), "libddcutil.so.5")
		logger.Debug("so path:", path)
		cStr := C.CString(path)
		defer C.free(unsafe.Pointer(cStr))
		ret := C.InitDDCCISo(cStr)
		if ret == -2 {
			logger.Debug("failed to initialize ddca_free_all_displays sym")
		}
	}

	return ddc, nil
}

func (d *ddcci) freeList() {
	//logger.Debug("brightness: freeList, clear all display cache d.lisPointer", d.listPointer)
	for _, handle := range d.displayHandleMap {
		handle.Close()
	}

	d.displayHandleMap = make(map[string]*displayHandle)

	if d.listPointer != nil {
		C.ddca_free_display_info_list(d.listPointer)
		_ = C.freeAllDisplaysWrapper()
		d.listPointer = nil
	}
}

func (d *ddcci) RefreshDisplays() error {
	d.listMu.Lock()
	defer d.listMu.Unlock()

	d.freeList()

	status := C.ddca_get_display_info_list2(C.bool(true), &d.listPointer)
	if status != C.int(0) {
		return fmt.Errorf("brightness: failed to get display info list: %d", status)
	}
	logger.Debug("brightness: display-number=", int(d.listPointer.ct))
	for i := 0; i < int(d.listPointer.ct); i++ {
		err := d.initDisplay(i)
		if err != nil {
			logger.Warning(err)
		}
	}
	return nil
}

func (d *ddcci) initDisplay(idx int) error {
	logger.Debug("brightness: initDisplay enter, index=", idx)
	dh := d.newDisplayHandle(idx)
	dh.Open()
	dh.initEdid()
	d.displayHandleMap[dh.getEdidBase64()] = dh
	return nil
}

func (d *ddcci) SupportBrightness(edidBase64 string) bool {
	d.listMu.Lock()
	defer d.listMu.Unlock()

	index, find := d.findMonitorIndex(edidBase64)
	logger.Debug("brightness: SupportBrightness", index, find, edidBase64)
	return find
}

func (d *ddcci) GetBrightness(edidBase64 string) (brightness int, err error) {
	d.listMu.Lock()
	defer d.listMu.Unlock()

	handle, ok := d.displayHandleMap[edidBase64]
	if !ok || handle == nil {
		//ignore any bytes
		idx, find := d.findMonitorIndex(edidBase64)
		if find {
			handle = d.getDisplayHandleByIdx(idx)
			logger.Info("get display handle by index:", idx)
		} else {
			err = fmt.Errorf("brightness: failed to find monitor")
			return
		}
	}

	var val C.DDCA_Non_Table_Vcp_Value
	status := C.ddca_get_non_table_vcp_value(handle.handle, brightnessVCP, &val)
	if status != C.int(0) {
		err = fmt.Errorf("brightness: failed to get brightness: %d", status)
		return
	}

	brightness = int(val.sl)
	return
}

func (d *ddcci) SetBrightness(edidBase64 string, percent int) error {
	d.listMu.Lock()
	defer d.listMu.Unlock()

	// 开启结果验证，防止返回设置成功，但实际上没有生效的情况
	// 此方法仅对当前线程生效
	C.ddca_enable_verify(false)
	dh, ok := d.displayHandleMap[edidBase64]
	if !ok || dh == nil {
		//ignore any bytes
		idx, find := d.findMonitorIndex(edidBase64)
		if find {
			dh = d.getDisplayHandleByIdx(idx)
			logger.Info("get display handle by index---------", idx)
		} else {
			return fmt.Errorf("brightness: failed to find monitor")
		}
	}

	if dh.getState() == 0 {
		dh.Open()
	}
	return dh.setBrightness(percent)
}

func (d *ddcci) getDisplayHandleByIdx(idx int) *displayHandle {
	for _, handle := range d.displayHandleMap {
		if handle.idx == idx {
			logger.Debugf("getDisplayHandleByIdx:=== %d\n", idx)
			return handle
		}
	}
	return nil
}

func zeroEdidMonitorName(byBuf []byte) bool {
	if len(byBuf) < 128 {
		logger.Infof("zeroEdidMonitorName: len error, %d\n", len(byBuf))
		return false
	}
	hasModify := false
	//解释几个数字: edid长度128,头部长54，扩展部分18字节一段，54 + 18 * 4 = 126
	for i := 0; i < 4; i++ {
		offset := 0x36 + i*18
		//fmt.Printf("zeroEdidMonitorName: idx=%d, %x\n", i, byBuf[offset: offset + 5])
		if reflect.DeepEqual(byBuf[offset:offset+3], []byte{0, 0, 0}) {
			if byBuf[offset+3] == 0xfc {
				//logger.Debugf("brightness: zeroEdidMonitorName, block index=%d\n", i)
				hasModify = true
				zeroStart := offset + 5
				zeroLen := 13
				for j := 0; j < zeroLen; j++ {
					byBuf[zeroStart+j] = 0
				}
			}
		}
	}
	return hasModify
}

func (d *ddcci) findMonitorIndex(edidBase64 string) (int, bool) {
	logger.Debug("edidBase64:", edidBase64)
	dh, find := d.displayHandleMap[edidBase64]
	if find {
		logger.Info("find df for edidBase64:", edidBase64)
		return dh.idx, find
	}

	idx := 0
	bufInput, _ := base64.StdEncoding.DecodeString(edidBase64)
	zeroEdidMonitorName(bufInput)
	for edidIter, idxIter := range d.displayHandleMap {
		//logger.Debugf("edid:", edidIter)
		bufIter, _ := base64.StdEncoding.DecodeString(edidIter)
		zeroEdidMonitorName(bufIter)
		// ddcci只存储128个字节，所以之比较前128个字节
		if bytes.HasPrefix(bufInput, bufIter) {
			idx = idxIter.idx
			find = true
			//logger.Debugf("brightness: DeepEqual\n")
		}
	}
	return idx, find
}

func (d *ddcci) getDisplayInfoByIdx(idx int) *C.DDCA_Display_Info {
	start := unsafe.Pointer(uintptr(unsafe.Pointer(d.listPointer)) + uintptr(C.sizeof_DDCA_Display_Info_List))
	size := uintptr(C.sizeof_DDCA_Display_Info)

	return (*C.DDCA_Display_Info)(unsafe.Pointer(uintptr(start) + size*uintptr(idx)))
}

func getEDIDChecksum(edid []byte) string {
	if len(edid) < 128 {
		return ""
	}

	id, _ := utils.SumStrMd5(string(edid[:128]))
	return id
}
