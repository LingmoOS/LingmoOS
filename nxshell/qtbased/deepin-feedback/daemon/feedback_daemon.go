/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"os/user"
	"path/filepath"
	"pkg.deepin.io/lib/dbus"
	. "pkg.deepin.io/lib/gettext"
	"pkg.deepin.io/lib/utils"
	"strconv"
	"strings"
	"sync"
	"time"
)

var globalRequstID uint64 = 0

var (
	dbusDest             = "com.deepin.Feedback"
	dbusObjectPath       = "/com/deepin/Feedback"
	dbusInterface        = "com.deepin.Feedback"
	deepinFeedbackCliExe = "/usr/bin/deepin-feedback-cli"
)

type category struct {
	Value           string
	BugzillaProject string
	Name            string
}

var categories []category

func initCategories() {
	categories = []category{
		{Value: "dde", BugzillaProject: "深度桌面环境", Name: Tr("Deepin Desktop Environment")},
		{Value: "dde-control-center", BugzillaProject: "深度控制中心", Name: Tr("Deepin Control Center")},
		{Value: "system", BugzillaProject: "系统配置(启动/仓库/驱动)", Name: Tr("System Configuration (startup / repository / drive)")},
		{Value: "deepin-installer", BugzillaProject: "系统安装", Name: Tr("Deepin Installer")},
		{Value: "deepin-store", BugzillaProject: "深度商店", Name: Tr("Deepin Store")},
		{Value: "deepin-music", BugzillaProject: "深度音乐", Name: Tr("Deepin Music")},
		{Value: "deepin-movie", BugzillaProject: "深度影院", Name: Tr("Deepin Movie")},
		{Value: "deepin-screenshot", BugzillaProject: "深度截图", Name: Tr("Deepin Screenshot")},
		{Value: "deepin-terminal", BugzillaProject: "深度终端", Name: Tr("Deepin Terminal")},
		{Value: "deepin-translator", BugzillaProject: "深度翻译", Name: Tr("Deepin Translator")},
        {Value: "deepin-cloud-print", BugzillaProject: "深度云打印", Name: Tr("Deepin Cloud Print")},
        {Value: "deepin-cloud-scan", BugzillaProject: "深度云扫描", Name: Tr("Deepin Cloud Scan")},
        {Value: "deepin-file-manager", BugzillaProject: "深度文件管理器", Name: Tr("Deepin File Manager")},
        {Value: "deepin-image-viewer", BugzillaProject: "深度看图", Name: Tr("Deepin Image Viewer")},
        {Value: "deepin-remote-assistance", BugzillaProject: "远程协助", Name: Tr("Deepin Remote Assistance")},
		{Value: "none", BugzillaProject: "深度网站", Name: Tr("Deepin Web")},
		{Value: "all", BugzillaProject: "我不清楚", Name: Tr("I don't know")},
	}
}

type FeedbackDaemon struct {
	WorkingSet             map[uint64]bool
	sorkingSetMutex        sync.Mutex
	GenerateReportFinished func(requstID uint64, filesJSON string)
}

func NewFeedbackDaemon() (fd *FeedbackDaemon) {
	fd = &FeedbackDaemon{}
	fd.WorkingSet = make(map[uint64]bool)
	return
}

func (fd *FeedbackDaemon) GetDBusInfo() dbus.DBusInfo {
	return dbus.DBusInfo{
		Dest:       dbusDest,
		ObjectPath: dbusObjectPath,
		Interface:  dbusInterface,
	}
}

func (fd *FeedbackDaemon) addWorkingRequest(requestID uint64) {
	fd.sorkingSetMutex.Lock()
	defer fd.sorkingSetMutex.Unlock()
	fd.WorkingSet[requestID] = true
	dbus.NotifyChange(fd, "WorkingSet")
}
func (fd *FeedbackDaemon) removeWorkingRequest(requestID uint64) {
	fd.sorkingSetMutex.Lock()
	defer fd.sorkingSetMutex.Unlock()
	delete(fd.WorkingSet, requestID)
	dbus.NotifyChange(fd, "WorkingSet")
}
func (fd *FeedbackDaemon) isInWorking() bool {
	fd.sorkingSetMutex.Lock()
	defer fd.sorkingSetMutex.Unlock()
	return len(fd.WorkingSet) > 0
}

// GetDistroInfo return current distributor name and release version.
func (fd *FeedbackDaemon) GetDistroInfo() (distroName, distroRelease string, err error) {
	args := []string{"--distro-name"}
	distroName, errMsg, err := utils.ExecAndWait(10, deepinFeedbackCliExe, args...)
	if err != nil {
		logger.Error(errMsg)
		logger.Error(err)
	}
	args = []string{"--distro-release"}
	distroRelease, errMsg, err = utils.ExecAndWait(10, deepinFeedbackCliExe, args...)
	if err != nil {
		logger.Error(errMsg)
		logger.Error(err)
	}
	return
}

// GetCategories return all categories that deepin-bug-reporter
// supported, each category contains several keywords to help to
// searching in front-end.
func (fd *FeedbackDaemon) GetCategories() (jsonCategories string, err error) {
	jsonCategories = marshalJSON(categories)
	return
}

// GenerateBugReport notify to generate bug report for target category.
func (fd *FeedbackDaemon) GenerateReport(dmsg dbus.DMessage, category string, allowPrivacy bool) (requstID uint64, err error) {
	username, err := getDBusCallerUsername(dmsg)
	if err != nil {
		logger.Error(err)
		return
	}

	defer func() {
		if err := recover(); err != nil {
			logger.Error(err)
			fd.removeWorkingRequest(requstID)
		}
	}()

	globalRequstID++
	requstID = globalRequstID

	distroName, distroRelease, _ := fd.GetDistroInfo()
	dateFormat := time.Now().Format("20060102-15:04:05")
	dateFormat = strings.Replace(dateFormat, ":", "", -1)
	outputFilename := fmt.Sprintf("deepin-feedback-%s-%s-%s-%s.tar.gz", distroName, distroRelease, category, dateFormat)
	outputFilepath := filepath.Join(os.TempDir(), outputFilename)
	args := []string{"--username", username, "--output", outputFilepath, category}
	if !allowPrivacy {
		args = append(args, "--privacy-mode")
	}
	logger.Info("generate report begin", deepinFeedbackCliExe, args)
	fd.addWorkingRequest(requstID)

	go func() {
		_, errMsg, err := utils.ExecAndWait(600, deepinFeedbackCliExe, args...)
		if err != nil {
			logger.Error(errMsg)
			logger.Error(err)
		}

		// found the result file(s)
		files := make([]string, 0)
		fileInfos, err := ioutil.ReadDir(os.TempDir())
		if err != nil {
			logger.Error(err)
		} else {
			for _, f := range fileInfos {
				if !f.IsDir() && strings.HasPrefix(f.Name(), outputFilename) {
					files = append(files, filepath.Join(os.TempDir(), f.Name()))
				}
			}
		}

		dbus.Emit(fd, "GenerateReportFinished", requstID, marshalJSON(files))
		logger.Info("generate report end", deepinFeedbackCliExe, args)
		fd.removeWorkingRequest(requstID)
	}()
	return
}

func getDBusCallerUsername(dmsg dbus.DMessage) (username string, err error) {
	if dbusDaemon == nil {
		err = fmt.Errorf("intialize dbus daemon failed")
		return
	}
	uid, err := dbusDaemon.GetConnectionUnixUser(dmsg.GetSender())
	if err != nil {
		return
	}
	user, err := user.LookupId(strconv.Itoa(int(uid)))
	if err != nil {
		return
	}
	username = user.Username
	return
}

func marshalJSON(v interface{}) (str string) {
	bytes, err := json.Marshal(v)
	if err != nil {
		logger.Error(err)
	}
	str = string(bytes)
	return
}
