/**
 * Copyright (C) 2015 Lingmo Technology Co., Ltd.
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
	"pkg.lingmo.io/lib/dbus"
	. "pkg.lingmo.io/lib/gettext"
	"pkg.lingmo.io/lib/utils"
	"strconv"
	"strings"
	"sync"
	"time"
)

var globalRequstID uint64 = 0

var (
	dbusDest             = "com.lingmo.Feedback"
	dbusObjectPath       = "/com/lingmo/Feedback"
	dbusInterface        = "com.lingmo.Feedback"
	lingmoFeedbackCliExe = "/usr/bin/lingmo-feedback-cli"
)

type category struct {
	Value           string
	BugzillaProject string
	Name            string
}

var categories []category

func initCategories() {
	categories = []category{
		{Value: "ocean", BugzillaProject: "深度桌面环境", Name: Tr("Lingmo Desktop Environment")},
		{Value: "ocean-control-center", BugzillaProject: "深度控制中心", Name: Tr("Lingmo Control Center")},
		{Value: "system", BugzillaProject: "系统配置(启动/仓库/驱动)", Name: Tr("System Configuration (startup / repository / drive)")},
		{Value: "lingmo-installer", BugzillaProject: "系统安装", Name: Tr("Lingmo Installer")},
		{Value: "lingmo-store", BugzillaProject: "深度商店", Name: Tr("Lingmo Store")},
		{Value: "lingmo-music", BugzillaProject: "深度音乐", Name: Tr("Lingmo Music")},
		{Value: "lingmo-movie", BugzillaProject: "深度影院", Name: Tr("Lingmo Movie")},
		{Value: "lingmo-screenshot", BugzillaProject: "深度截图", Name: Tr("Lingmo Screenshot")},
		{Value: "lingmo-terminal", BugzillaProject: "深度终端", Name: Tr("Lingmo Terminal")},
		{Value: "lingmo-translator", BugzillaProject: "深度翻译", Name: Tr("Lingmo Translator")},
        {Value: "lingmo-cloud-print", BugzillaProject: "深度云打印", Name: Tr("Lingmo Cloud Print")},
        {Value: "lingmo-cloud-scan", BugzillaProject: "深度云扫描", Name: Tr("Lingmo Cloud Scan")},
        {Value: "lingmo-file-manager", BugzillaProject: "深度文件管理器", Name: Tr("Lingmo File Manager")},
        {Value: "lingmo-image-viewer", BugzillaProject: "深度看图", Name: Tr("Lingmo Image Viewer")},
        {Value: "lingmo-remote-assistance", BugzillaProject: "远程协助", Name: Tr("Lingmo Remote Assistance")},
		{Value: "none", BugzillaProject: "深度网站", Name: Tr("Lingmo Web")},
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
	distroName, errMsg, err := utils.ExecAndWait(10, lingmoFeedbackCliExe, args...)
	if err != nil {
		logger.Error(errMsg)
		logger.Error(err)
	}
	args = []string{"--distro-release"}
	distroRelease, errMsg, err = utils.ExecAndWait(10, lingmoFeedbackCliExe, args...)
	if err != nil {
		logger.Error(errMsg)
		logger.Error(err)
	}
	return
}

// GetCategories return all categories that lingmo-bug-reporter
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
	outputFilename := fmt.Sprintf("lingmo-feedback-%s-%s-%s-%s.tar.gz", distroName, distroRelease, category, dateFormat)
	outputFilepath := filepath.Join(os.TempDir(), outputFilename)
	args := []string{"--username", username, "--output", outputFilepath, category}
	if !allowPrivacy {
		args = append(args, "--privacy-mode")
	}
	logger.Info("generate report begin", lingmoFeedbackCliExe, args)
	fd.addWorkingRequest(requstID)

	go func() {
		_, errMsg, err := utils.ExecAndWait(600, lingmoFeedbackCliExe, args...)
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
		logger.Info("generate report end", lingmoFeedbackCliExe, args)
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
