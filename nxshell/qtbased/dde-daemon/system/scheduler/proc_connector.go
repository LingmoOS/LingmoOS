// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package scheduler

import (
	"bytes"
	"encoding/binary"
	"os"
	"sync"
	"syscall"
	"time"

	"github.com/mdlayher/netlink"
)

// #include <linux/connector.h>
// #include <linux/cn_proc.h>
import "C"

// 给 proc connector 发送请求消息, 根据 body 不同可开始或停止监控。
func sendRequestMsg(conn *netlink.Conn, body uint32) error {
	var cnMsg = CnMsg{
		Id: CbId{
			Idx: C.CN_IDX_PROC,
			Val: C.CN_VAL_PROC,
		},
		Seq:  1,
		Ack:  0,
		Len:  uint16(binary.Size(body)),
		Flag: 0,
	}

	var buf bytes.Buffer
	// write cnMsg
	err := binary.Write(&buf, binary.LittleEndian, cnMsg)
	if err != nil {
		logger.Warning("write cnMsg failed, err:", err)
		return err
	}
	// write body
	err = binary.Write(&buf, binary.LittleEndian, body)
	if err != nil {
		logger.Warning("write body failed, err:", err)
		return err
	}

	_, err = conn.Send(netlink.Message{
		Header: netlink.Header{
			Type:     netlink.Done,
			Flags:    0,
			Sequence: 1,
			PID:      uint32(os.Getpid()),
		},
		Data: buf.Bytes(),
	})
	return err
}

// 监控进程事件
func (pm *procMonitor) listenProcEvents() error {
	conn, err := netlink.Dial(syscall.NETLINK_CONNECTOR, &netlink.Config{
		Groups:              C.CN_IDX_PROC,
		NetNS:               0,
		DisableNSLockThread: false,
		PID:                 0,
		Strict:              false,
	})
	if err != nil {
		return err
	}

	err = sendRequestMsg(conn, uint32(C.PROC_CN_MCAST_LISTEN))
	if err != nil {
		return err
	}

	defer func() {
		err := sendRequestMsg(conn, uint32(C.PROC_CN_MCAST_IGNORE))
		if err != nil {
			logger.Warning(err)
		}
	}()

	for {
		// 循环接收进程事件
		msgs, err := conn.Receive()
		if err != nil {
			logger.Warning("connector conn receive failed, err:", err)
			time.Sleep(10 * time.Second)
			continue
		}
		for _, msg := range msgs {
			err := pm.parseMsg(msg)
			if err != nil {
				logger.Warning(err)
			}
		}
	}

}

type procMonitor struct {
	mu           sync.Mutex
	exitPids     []uint32
	execPids     []uint32
	timer        *time.Timer
	timerStarted bool
	timerCb      func()
}

func newProcMonitor(cb func()) *procMonitor {
	return &procMonitor{
		timerCb: cb,
	}
}

const eventsSizeLimit = 1024

// 重置定时器
func (pm *procMonitor) resetTimer() {
	if pm.timerStarted {
		return
	}
	// logger.Debug("reset timer")
	if pm.timer == nil {
		pm.timer = time.AfterFunc(time.Second*handleProcEventsIntervalSec, pm.onTimerExpired)
	} else {
		pm.timer.Reset(time.Second * handleProcEventsIntervalSec)
	}
	pm.timerStarted = true
}

// 处理当定时器到时间了
func (pm *procMonitor) onTimerExpired() {
	// logger.Debug("timer expired")
	pm.mu.Lock()
	pm.timerStarted = false
	pm.mu.Unlock()

	if pm.timerCb != nil {
		pm.timerCb()
	}
}

// 处理 exec 事件
func (pm *procMonitor) handleExecEvent(ev *ExecProcEvent) {
	if ev.ProcPid != ev.ProcTGid {
		// 暂不处理这种情况
		return
	}

	pm.mu.Lock()
	if len(pm.execPids) <= eventsSizeLimit {
		if !u32SliceContains(pm.execPids, ev.ProcPid) {
			pm.execPids = append(pm.execPids, ev.ProcPid)
			pm.resetTimer()
		}
	}
	pm.mu.Unlock()
}

func u32SliceContains(slice []uint32, val uint32) bool {
	for _, v := range slice {
		if v == val {
			return true
		}
	}
	return false
}

// 处理 exit 退出事件
func (pm *procMonitor) handleExitEvent(ev *ExitProcEvent) {
	if ev.ProcessPid != ev.ProcessTgid {
		// 暂不处理这种情况
		return
	}

	pm.mu.Lock()
	if len(pm.exitPids) <= eventsSizeLimit {
		pm.exitPids = append(pm.exitPids, ev.ProcessPid)
		// NOTE: exit 事件发生时不用重置定时器
	}
	pm.mu.Unlock()
}

// 获取活着的进程列表
func (pm *procMonitor) getAlivePids() []uint32 {
	logger.Debug("proc monitor handle events")

	pm.mu.Lock()
	defer pm.mu.Unlock()

	// logger.Debug("proc monitor execPids:", pm.execPids)
	// logger.Debug("proc monitor exitPids:", pm.exitPids)

	var pids []uint32
	for _, execPid := range pm.execPids {
		// 取出掉已经退出的
		exited := false
		for _, exitPid := range pm.exitPids {
			if execPid == exitPid {
				exited = true
				break
			}
		}
		if !exited {
			pids = append(pids, execPid)
		}
	}
	logger.Debug("proc monitor need handle pids:", pids)

	// 清空，但保持容量
	pm.execPids = pm.execPids[0:0:cap(pm.execPids)]
	pm.exitPids = pm.exitPids[0:0:cap(pm.exitPids)]
	return pids
}

// 解析含有进程事件的消息
func (pm *procMonitor) parseMsg(msg netlink.Message) (err error) {
	bytBuf := bytes.NewBuffer(msg.Data)

	cnMsg := &CnMsg{}
	header := &ProcEventHeader{}
	// read binary message
	err = binary.Read(bytBuf, binary.LittleEndian, cnMsg)
	if err != nil {
		logger.Warning("read CnMsg failed, err:", err)
		return
	}
	err = binary.Read(bytBuf, binary.LittleEndian, header)
	if err != nil {
		logger.Warning("read ProcEventHeader failed, err:", err)
		return
	}
	switch header.What {
	// proc exec
	case C.PROC_EVENT_EXEC:
		event := &ExecProcEvent{}
		err = binary.Read(bytBuf, binary.LittleEndian, event)
		if err != nil {
			logger.Warningf("read ExecProcEvent failed, err: %v\n", err)
			return
		}
		logger.Debugf("recv message: proc exec %+v", event)
		pm.handleExecEvent(event)

	// proc exit
	case C.PROC_EVENT_EXIT:
		event := &ExitProcEvent{}
		err = binary.Read(bytBuf, binary.LittleEndian, event)
		if err != nil {
			logger.Warningf("read ExitProcEvent failed, err: %v", err)
			return
		}
		logger.Debugf("recv message: proc exit %+v", event)
		pm.handleExitEvent(event)
	}
	return nil
}
