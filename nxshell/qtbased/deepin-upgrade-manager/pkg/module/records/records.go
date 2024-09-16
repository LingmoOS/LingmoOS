// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package records

import (
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/util"
	"encoding/json"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
)

type RecoredState int

const (
	_UNKNOW_STATE         RecoredState = -1
	_ROLLBACK_READY_START RecoredState = iota
	_ROLLBACK_MAIN_RINNING
	_ROLLBACK_RESTORE
	_ROLLBACK_AFTEROPER

	_ROLLBACK_SUCCESSED RecoredState = 100
	_ROLLBACK_FAILED    RecoredState = 101
)

type RecordsInfo struct {
	CurrentState    RecoredState `json:"CurrentState"`
	RollbackVersion string       `json:"RollbackVersion"`
	RepoMount       string       `json:"Repo_Mount_Point"`
	AferRun         string       `json:"AfterRun"`

	TimeOut     uint   `json:"GrubTimeout"`
	GrubDefault string `json:"GrubDefault"`

	filename string
	locker   sync.RWMutex
}

func toRecoredState(state int) RecoredState {
	switch RecoredState(state) {
	case _ROLLBACK_READY_START:
		return _ROLLBACK_READY_START
	case _ROLLBACK_MAIN_RINNING:
		return _ROLLBACK_MAIN_RINNING
	case _ROLLBACK_RESTORE:
		return _ROLLBACK_RESTORE
	case _ROLLBACK_SUCCESSED:
		return _ROLLBACK_SUCCESSED
	case _ROLLBACK_FAILED:
		return _ROLLBACK_FAILED
	default:
		return _UNKNOW_STATE
	}
}

func readFile(recordsfile string, info interface{}) error {
	content, err := ioutil.ReadFile(filepath.Clean(recordsfile))
	if err != nil {
		return err
	}
	return json.Unmarshal(content, info)
}

func LoadRecords(rootfs, recordsfile, repoMount string, needcreated bool) *RecordsInfo {
	info := new(RecordsInfo)
	path := filepath.Join(rootfs, recordsfile)
	info.CurrentState = _UNKNOW_STATE
	info.TimeOut = 2
	info.RepoMount = repoMount
	defer func() {
		info.filename = path
		if needcreated {
			info.save()
		}
	}()
	if util.IsExists(path) {
		var record RecordsInfo
		err := readFile(path, &record)
		if err != nil || len(record.RollbackVersion) == 0 {
			record.CurrentState = _UNKNOW_STATE
		} else {
			info = &record
			return info
		}
	} else {
		dir := filepath.Dir(path)
		_ = os.MkdirAll(dir, 0644)
	}
	return info
}

func (info *RecordsInfo) save() error {
	info.locker.RLock()
	data, err := json.Marshal(info)
	info.locker.RUnlock()
	if err != nil {
		return err
	}
	tmpFile := info.filename + "-" + util.MakeRandomString(util.MinRandomLen)
	// deepin-upgrade-manager-tool need load file
	f, err := os.OpenFile(filepath.Clean(tmpFile), os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0600|0064)
	if err != nil {
		return err
	}
	n, err := f.Write(data)
	if err == nil && n < len(data) {
		err = io.ErrShortWrite
	}
	if err == nil {
		err = f.Sync()
	}
	if err1 := f.Close(); err == nil {
		err = err1
	}

	if err != nil {
		logger.Warning("failed save the records info, err:", err)
	}
	_, err = util.Move(info.filename, tmpFile, true)
	if err != nil {
		logger.Warning("failed move the records info, err:", err)
	}
	return err
}

func (info *RecordsInfo) SetRecoredState(state int) {
	records := toRecoredState(state)
	info.CurrentState = records
}

func (info *RecordsInfo) IsNeedMainRunning() bool {
	if int(info.CurrentState) > int(_ROLLBACK_MAIN_RINNING) {
		return false
	} else {
		info.CurrentState = _ROLLBACK_MAIN_RINNING
		info.save()
		return true
	}
}

func (info *RecordsInfo) IsAfterOper() bool {
	if int(info.CurrentState) > int(_ROLLBACK_AFTEROPER) {
		return false
	} else {
		info.CurrentState = _ROLLBACK_AFTEROPER
		info.save()
		return true
	}
}

func (info *RecordsInfo) IsReadyOper() bool {
	return info.CurrentState >= _ROLLBACK_AFTEROPER
}

func (info *RecordsInfo) IsOper() bool {
	return info.CurrentState == _ROLLBACK_AFTEROPER
}

func (info *RecordsInfo) SetRollbackInfo(version, newdefault, oldhead string, newtime uint) {
	info.RollbackVersion = version
	info.save()
	if newtime == 0 {
		info.TimeOut = 2
	} else {
		info.TimeOut = newtime
	}
	if len(newdefault) == 0 {
		info.GrubDefault = "0"
	} else if strings.Contains(newdefault, oldhead) {
		//do nothing
	} else {
		info.GrubDefault = newdefault
	}
	logger.Infof("Success to save old grub info, old time %v, old grub default %v", info.TimeOut, info.GrubDefault)
}

func (info *RecordsInfo) Reset(version string) {
	if len(info.RollbackVersion) == 0 {
		info.RollbackVersion = version
		info.TimeOut = 2
		info.GrubDefault = "0"
		info.save()
	}
}

func (info *RecordsInfo) SetReady() {
	info.CurrentState = _ROLLBACK_READY_START
	info.save()
}

func (info *RecordsInfo) IsReady() bool {
	return info.CurrentState == _ROLLBACK_READY_START
}

func (info *RecordsInfo) SetRestore() {
	info.CurrentState = _ROLLBACK_RESTORE
	info.save()
}

func (info *RecordsInfo) IsRestore() bool {
	return info.CurrentState == _ROLLBACK_RESTORE
}

func (info *RecordsInfo) Version() string {
	return info.RollbackVersion
}

func (info *RecordsInfo) IsFailed() bool {
	return info.CurrentState == _ROLLBACK_FAILED
}

func (info *RecordsInfo) IsSucceeded() bool {
	return info.CurrentState == _ROLLBACK_SUCCESSED
}

func (info *RecordsInfo) IsReadyRollback() bool {
	return info.CurrentState >= _ROLLBACK_READY_START
}

func (info *RecordsInfo) SetSuccessfully() {
	info.CurrentState = _ROLLBACK_SUCCESSED
	info.save()
}

func (info *RecordsInfo) SetFailed(version string) {
	info.CurrentState = _ROLLBACK_FAILED
	info.RollbackVersion = version
	info.save()
}

func (info *RecordsInfo) Remove() {
	os.Remove(info.filename)
	logger.Debugf("remove records rollback state file: %s", info.filename)
}

func (info *RecordsInfo) SetAfterRun(cmd string) {
	info.AferRun = cmd
	info.save()
}

func (info *RecordsInfo) SaveResult(root string) (err error) {
	res := filepath.Join(root, SelfRecordResultPath)
	if util.IsExists(res) {
		os.RemoveAll(res)
	}
	file, err := os.OpenFile(res, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0777)
	if err != nil {
		return
	}
	defer func(){
		if closeErr := file.Close(); err != nil {
			err = closeErr
		}
	}()
	bt := []byte(strconv.Itoa(int(info.CurrentState)) + "," + info.AferRun)
	if _, err = file.Write(bt); err != nil {
		return err
	}
	if err = file.Sync(); err != nil {
		return err
	}
	return
}
