// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package upgrader

import (
	"bufio"
	config "deepin-upgrade-manager/pkg/config/upgrader"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/bootkitinfo"
	"deepin-upgrade-manager/pkg/module/chroot"
	"deepin-upgrade-manager/pkg/module/dirinfo"
	"deepin-upgrade-manager/pkg/module/fstabinfo"
	"deepin-upgrade-manager/pkg/module/generator"
	"deepin-upgrade-manager/pkg/module/grub"
	"deepin-upgrade-manager/pkg/module/langselector"
	"deepin-upgrade-manager/pkg/module/mountinfo"
	"deepin-upgrade-manager/pkg/module/mountpoint"
	"deepin-upgrade-manager/pkg/module/notify"
	"deepin-upgrade-manager/pkg/module/plymouth"
	"deepin-upgrade-manager/pkg/module/records"
	"deepin-upgrade-manager/pkg/module/repo"
	"deepin-upgrade-manager/pkg/module/repo/branch"
	"deepin-upgrade-manager/pkg/module/util"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/godbus/dbus/v5"
)

var msgSuccessRollBack = util.Tr("Your system is successfully rolled back to %s.")
var msgFailRollBack = util.Tr("Rollback failed. The system is reverted to %s.")
var msgRollBack = util.Tr("System Recovery")

type (
	opType    int32
	stateType int32
)

const (
	SelfMountPath          = "/proc/self/mounts"
	SelfRecordStatePath    = "/etc/deepin-upgrade-manager/state.records"
	LocalNotifyDesktopPath = "/usr/share/deepin-upgrade-manager/deepin-upgrade-manager-tool.desktop"
	AutoStartDesktopPath   = "/etc/xdg/autostart/deepin-upgrade-manager-tool.desktop"
	DefaultGrubConfig      = "/etc/default/grub"
	LessKeepSize           = 5 * 1024 * 1024 * 1024
)

const (
	_OP_TY_COMMIT_START opType = iota*10 + 100
	_OP_TY_COMMIT_PREPARE_DATA
	_OP_TY_COMMIT_REPO_SUBMIT
	_OP_TY_COMMIT_REPO_CLEAN
	_OP_TY_COMMIT_GRUB_UPDATE
	_OP_TY_COMMIT_END opType = 199
)

const (
	_OP_TY_ROLLBACK_PREPARING_START opType = iota*10 + 200
	_OP_TY_ROLLBACK_PREPARING_SET_CONFIG
	_OP_TY_ROLLBACK_PREPARING_SET_WAITTIME
	_OP_TY_ROLLBACK_PREPARING_END opType = 299
)

const (
	_OP_TY_DELETE_START opType = iota*10 + 300
	_OP_TY_DELETE_GRUB_UPDATE
	_OP_TY_DELETE_END opType = 399
)

type CurrentState struct {
	CurOp      opType
	CurVersion string
}

var currentState CurrentState

const (
	_STATE_TY_SUCCESS stateType = -iota
	_STATE_TY_FAILED_NO_REPO
	_STATE_TY_FAILED_NO_SPACE
	_STATE_TY_FAILED_UPDATE_GRUB
	_STATE_TY_FAILED_HANDLING_MOUNTS
	_STATE_TY_FAILED_OSTREE_INIT
	_STATE_TY_FAILED_OSTREE_COMMIT
	_STATE_TY_FAILED_OSTREE_ROLLBACK
	_STATE_TY_FAILED_VERSION_DELETE
	_STATE_TY_FAILED_NO_VERSION
	_STATE_TY_FAILED_EXIT_SIGNAL
	_STATE_TY_FAILED_UPDATE_INITRD
	_STATE_TY_RUNING stateType = 1
)

type (
	progressTheme int32
)

const (
	_NO_THEME progressTheme = iota
	_UPGRADE_TOOL
	_BOOT_KIT
)

func (state stateType) String() string {
	switch state {
	case _STATE_TY_RUNING:
		return "running"
	case _STATE_TY_SUCCESS:
		return "success"
	case _STATE_TY_FAILED_NO_SPACE:
		return "not enough space"
	case _STATE_TY_FAILED_NO_REPO:
		return "repo does not exist"
	case _STATE_TY_FAILED_HANDLING_MOUNTS:
		return "failed handling mounts"
	case _STATE_TY_FAILED_UPDATE_GRUB:
		return "failed update grub"
	case _STATE_TY_FAILED_OSTREE_COMMIT:
		return "failed ostree commit"
	case _STATE_TY_FAILED_OSTREE_ROLLBACK:
		return "failed ostree rollback"
	case _STATE_TY_FAILED_OSTREE_INIT:
		return "failed ostree init"
	case _STATE_TY_FAILED_VERSION_DELETE:
		return "version not allowed to delete"
	case _STATE_TY_FAILED_NO_VERSION:
		return "version does not exist"
	case _STATE_TY_FAILED_EXIT_SIGNAL:
		return "receiving kill signal		"
	}
	return "unknown"
}

func (op opType) String() string {
	switch op {
	case _OP_TY_COMMIT_START:
		return "start version submited"
	case _OP_TY_COMMIT_PREPARE_DATA:
		return "start to submit data preparation"
	case _OP_TY_COMMIT_REPO_SUBMIT:
		return "start to submit data"
	case _OP_TY_COMMIT_REPO_CLEAN:
		return "start to repo cleaning"
	case _OP_TY_COMMIT_GRUB_UPDATE:
		return "start to grub updating"
	case _OP_TY_COMMIT_END:
		return "end version submited"
	case _OP_TY_ROLLBACK_PREPARING_START:
		return "start preparing rollback"
	case _OP_TY_ROLLBACK_PREPARING_SET_CONFIG:
		return "start set preparing rollback configuration file"
	case _OP_TY_ROLLBACK_PREPARING_SET_WAITTIME:
		return "start set the grub waiting time "
	case _OP_TY_ROLLBACK_PREPARING_END:
		return "end preparing rollback"
	case _OP_TY_DELETE_START:
		return "start remove the repo version"
	case _OP_TY_DELETE_GRUB_UPDATE:
		return "start to grub updating"
	case _OP_TY_DELETE_END:
		return "end remove the repo version"
	}
	return "unknown"
}

type Upgrader struct {
	conf *config.Config

	mountInfos mountinfo.MountInfoList

	recordsInfo *records.RecordsInfo

	fsInfo fstabinfo.FsInfoList

	repoSet map[string]repo.Repository

	rootMP string
}

func NewUpgraderTool() *Upgrader {
	info := Upgrader{}
	return &info
}

func NewUpgrader(conf *config.Config,
	rootMP string) (*Upgrader, error) {
	mountInfos, err := mountinfo.Load(SelfMountPath)
	if err != nil {
		return nil, err
	}
	fstabDir := filepath.Clean(filepath.Join(rootMP, "/etc/fstab"))
	fsInfo, err := fstabinfo.Load(fstabDir, rootMP)
	if err != nil {
		return nil, err
	}
	info := Upgrader{
		conf:       conf,
		mountInfos: mountInfos,
		repoSet:    make(map[string]repo.Repository),
		rootMP:     rootMP,
		fsInfo:     fsInfo,
	}
	for _, v := range conf.RepoList {
		handler, err := repo.NewRepo(repo.REPO_TY_OSTREE, filepath.Join(rootMP, v.Repo))
		if err != nil {
			return nil, err
		}
		info.repoSet[v.Repo] = handler
	}
	return &info, nil
}

func setPlymouthTheme(theme string) error {
	path := "/var/cache/system-rollback-theme"
	if _, err := os.Stat(path); err != nil {
		err = os.Mkdir(path, 0755)
		if err != nil {
			return err
		}
	}
	themeCacheFile := filepath.Join(path, "plymouth-default-theme")
	var defaultTheme string
	plymouthConfig := "/etc/plymouth/plymouthd.conf"
	pf, err := os.Open(plymouthConfig)
	if err != nil {
		return err
	}
	scanner := bufio.NewScanner(pf)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.Contains(line, "Theme") {
			defaultTheme = strings.Trim(strings.Split(line, "=")[1], "\n")
		}
	}
	f, err := os.OpenFile(themeCacheFile, os.O_TRUNC|os.O_CREATE|os.O_WRONLY, 0755)
	if err != nil {
		return err
	}
	_, err = f.WriteString(defaultTheme)
	if err != nil {
		return err
	}
	out, err := exec.Command("/usr/sbin/plymouth-set-default-theme", "-R", theme).CombinedOutput()
	if err != nil {
		logger.Warning("failed to set upgrade plymouth theme:", string(out))
		return err
	}
	return nil
}

func restorePlymouthTheme() error {
	themeCacheFile := filepath.Join("/var/cache/system-rollback-theme", "plymouth-default-theme")
	if _, err := os.Stat(themeCacheFile); err != nil {
		return errors.New("cannot find default theme file")
	}
	// Get default plymouth theme.
	f, err := os.Open(themeCacheFile)
	if err != nil {
		return err
	}
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		if scanner.Text() != "" {
			_, err := exec.Command("/usr/sbin/plymouth-set-default-theme", "-R", strings.Trim(scanner.Text(), "\n")).CombinedOutput()
			if err != nil {
				logger.Warning("failed to set default theme")
				return err
			}
		}
	}
	return nil
}

func (c *Upgrader) ResetRepo() {
	for key := range c.repoSet {
		delete(c.repoSet, key)
	}
	for _, v := range c.conf.RepoList {
		handler, err := repo.NewRepo(repo.REPO_TY_OSTREE, filepath.Join(c.rootMP, v.Repo))
		if err != nil {
			logger.Warning("failed reset repo, err:", err)
		}
		c.repoSet[v.Repo] = handler
	}
}

func (c *Upgrader) Init() (int, error) {
	exitCode := _STATE_TY_SUCCESS
	var repoMountPoint string

	repoMountPoint, _, err := c.RepoMountpointAndUUID()
	if err != nil {
		exitCode = _STATE_TY_FAILED_OSTREE_INIT
		return int(exitCode), err
	}
	if len(repoMountPoint) != 0 {
		point := strings.TrimSpace(repoMountPoint)
		c.conf.ChangeRepoMountPoint(point)
		c.ResetRepo()
		logger.Debugf("find present system max partition is %s ,changed the repo mount point", point)
	}

	osVersion, err := util.GetOSInfo("", "MajorVersion")
	if nil != err {
		logger.Error("failed get new version, err:", err)
	} else {
		c.conf.SetDistribution(osVersion)
	}

	err = c.conf.Prepare()
	if err != nil {
		exitCode = _STATE_TY_FAILED_OSTREE_INIT
		return int(exitCode), errors.New("failed to initialize config")
	}
	if c.IsExistRepo() {
		exitCode = _STATE_TY_FAILED_OSTREE_INIT
		return int(exitCode), errors.New("failed to initialize because repository exists")
	}
	for _, handler := range c.repoSet {
		err := handler.Init()
		if err != nil {
			exitCode = _STATE_TY_FAILED_OSTREE_INIT
			return int(exitCode), err
		}
	}
	return int(exitCode), nil
}

func (c *Upgrader) SaveActiveVersion(version string) {
	c.conf.ActiveVersion = version
	err := c.conf.Save()
	if err != nil {
		logger.Infof("failed update version to %q: %v", version, err)
	}
}

func (c *Upgrader) Commit(newVersion, subject string, useSysData bool,
	evHandler func(op, state int32, target, desc string)) (excode int, err error) {
	exitCode := _STATE_TY_SUCCESS
	var isClean bool
	var theme string
	c.SendingSignal(evHandler, _OP_TY_COMMIT_START, _STATE_TY_RUNING, newVersion, "")

	if len(newVersion) == 0 {
		newVersion, err = bootkitinfo.NewVersion()
		if err != nil {
			logger.Warning("failed add version, from deepin boot kit, err:", err)
			newVersion, err = c.GenerateBranchName()
			if err != nil {
				exitCode = _STATE_TY_FAILED_NO_REPO
				goto failure
			}
		}
	}
	c.conf.SetVersionConfig(newVersion)
	c.conf.LoadReadyData()
	if len(subject) == 0 {
		subject = fmt.Sprintf("Release %s", newVersion)
	}
	logger.Info("the version number of this submission is:", newVersion)
	theme = c.conf.RepoList[0].PlymouthTheme
	if len(theme) == 0 {
		theme = "deepin-recovery"
	}
	err = setPlymouthTheme(theme)
	if err != nil {
		logger.Warning("failed to set plymouth theme:", err)
	}
	for _, v := range c.conf.RepoList {
		err = c.repoCommit(v, newVersion, subject, useSysData, evHandler)
		if err != nil {
			exitCode = _STATE_TY_FAILED_OSTREE_COMMIT
			goto failure
		}
	}

	c.SaveActiveVersion(newVersion)

	// automatically clear redundant versions
	if c.IsAutoClean() {
		c.SendingSignal(evHandler, _OP_TY_COMMIT_REPO_CLEAN, _STATE_TY_RUNING, newVersion, "")
		isClean, err = c.RepoAutoCleanup()
		if err != nil {
			logger.Error("failed auto cleanup repo, err:", err)
		}
	}
	// prevent another update grub
	if !isClean {
		c.SendingSignal(evHandler, _OP_TY_COMMIT_GRUB_UPDATE, _STATE_TY_RUNING, newVersion, "")
		exitCode, err = c.UpdateGrub()
		if err != nil {
			exitCode = _STATE_TY_FAILED_UPDATE_GRUB
			goto failure
		}
	}
	err = restorePlymouthTheme()
	if err != nil {
		logger.Warning("failed to restore plymouth theme:", err)
	}

	c.SendingSignal(evHandler, _OP_TY_COMMIT_END, _STATE_TY_SUCCESS, newVersion, "")
	return int(exitCode), nil
failure:
	c.SendingSignal(evHandler, _OP_TY_COMMIT_END, exitCode, newVersion, err.Error())
	return int(exitCode), err
}

func (c *Upgrader) IsExistRepo() bool {
	for _, v := range c.conf.RepoList {
		if !util.IsExists(v.Repo) {
			logger.Debugf("%s does not exist", v.Repo)
			return false
		}

		handler := c.repoSet[v.Repo]
		list, err := handler.List()
		if err != nil {
			logger.Debugf("%s does not exist", v.Repo)
			return false
		}
		if len(list) == 0 {
			logger.Debugf("%s does not exist", v.Repo)
			return false
		}
	}
	return true
}

func (c *Upgrader) UpdateGrub() (stateType, error) {
	exitCode := _STATE_TY_SUCCESS
	logger.Info("start update grub")
	cmd := exec.Command("update-grub")
	cmd.Env = append(cmd.Env, langselector.LocalLangEnv()...)
	// need save
	lgfd := logger.LoggerFD()
	if lgfd != nil {
		cmd.Stderr = lgfd
	}
	err := cmd.Run()
	if err != nil {
		exitCode = _STATE_TY_FAILED_UPDATE_GRUB
		return exitCode, err
	}
	util.ExecCommand("/usr/bin/sync", []string{})
	return exitCode, nil
}

func (c *Upgrader) EnableBoot(version string) (stateType, error) {
	exitCode := _STATE_TY_SUCCESS
	err := c.Snapshot(version)
	if err != nil {
		exitCode = _STATE_TY_FAILED_NO_REPO
		return exitCode, err
	}
	for _, v := range c.conf.RepoList {
		dataDir := filepath.Join(c.rootMP, v.SnapshotDir, version)
		err := c.enableSnapshotBoot(dataDir, version)
		if err != nil {
			exitCode = _STATE_TY_FAILED_NO_REPO
			return exitCode, err
		}
	}
	return exitCode, nil
}

func (c Upgrader) GrubTitle(version string) string {
	var title, titleTail string
	target := filepath.Join(c.rootMP, c.conf.RepoList[0].SnapshotDir, version)
	if !util.IsExists(target) {
		c.EnableBoot(version)
	}
	systemName, err := util.GetOSInfo(target, "SystemName")
	if err != nil {
		logger.Warning("failed get system name, err:", err)
	}
	MinorVersion, err := util.GetOSInfo(target, "MinorVersion")
	if err != nil {
		logger.Warning("failed get minor version, err:", err)
	}
	handler, _ := repo.NewRepo(repo.REPO_TY_OSTREE,
		filepath.Join(c.rootMP, c.conf.RepoList[0].Repo))
	content, err := handler.Subject(version)
	if err == nil {
		sub, err := config.LoadSubject(content)
		if err == nil {
			if sub.IsIntall() {
				titleTail = "initital backup"
			} else if len(sub.Time()) > 0 {
				t, err := strconv.ParseInt(sub.Time(), 10, 64)
				if err == nil {
					timeTemplate1 := "2006/01/02 15:04:05"
					titleTail = time.Unix(t, 0).Format(timeTemplate1)
				}
			}
		}
	}
	if len(titleTail) == 0 {
		time, err := handler.CommitTime(version)
		if err == nil {
			titleTail = strings.Replace(time, "-", "/", -1)
		}
	}
	if len(titleTail) == 0 {
		title = fmt.Sprintf("Rollback to %s", version)
		logger.Warning("failed get commit time, err:", err)
	} else {
		title = systemName + " " + MinorVersion + " " + "(" + titleTail + ")"
	}
	return title
}

func (c *Upgrader) EnableBootList() (string, int, error) {
	var exitCode int
	list, exitCode, err := c.ListVersion()
	if err != nil {
		return "", exitCode, err
	}
	bootSnapDir := filepath.Join(c.rootMP, "/boot/snapshot")
	if util.IsExists(bootSnapDir) {
		os.RemoveAll(bootSnapDir)
	}
	var showList []string
	osVersion, err := util.GetOSInfo("", "MajorVersion")
	if nil != err {
		osVersion = "23"
	}
	osVersion = "v" + osVersion
	err = c.LoadRollbackRecords(false)
	if err == nil && branch.IsValid(c.recordsInfo.RollbackVersion) && c.recordsInfo.IsReady() {
		c.EnableBoot(c.recordsInfo.RollbackVersion)
		showList = append(showList, c.recordsInfo.RollbackVersion)
	} else {
		for _, v := range list {
			head := strings.Split(v, ".")[0]
			if head == osVersion && generator.Less(v, c.conf.ActiveVersion) {
				continue
			}
			c.EnableBoot(v)
			showList = append(showList, v)
		}
	}

	diskInfo := c.fsInfo.MatchDestPoint(c.conf.RepoList[0].RepoMountPoint)
	listInfo := bootkitinfo.Load(showList, diskInfo.DiskUUID)
	for _, v := range showList {
		commitName := c.GrubTitle(v)
		listInfo.SetVersionName(v, commitName)
	}

	return listInfo.ToJson(), exitCode, nil
}

func (c *Upgrader) Snapshot(version string) error {
	for _, v := range c.conf.RepoList {
		err := c.repoSnapShot(v, version)
		if err != nil {
			return err
		}
	}
	return nil
}

func (c *Upgrader) UpdateMount(repoConf *config.RepoConfig, version string) (mountpoint.MountPointList, error) {
	dataDir := filepath.Join(c.rootMP, repoConf.SnapshotDir, version)
	mountedPointList, err := c.updateLoaclMount(dataDir)
	if err != nil {
		return mountedPointList, err
	}
	// need to get mount information again
	mountinfos, err := mountinfo.Load(SelfMountPath)
	logger.Infof("to update the local mount, you need to reload the mount information")
	if err != nil {
		return nil, err
	}
	c.mountInfos = mountinfos
	return mountedPointList, nil
}

func (c *Upgrader) IsExistVersion(version string) bool {
	list, _, err := c.ListVersion()
	if err != nil {
		return false
	}
	for _, v := range list {
		if v == version {
			return true
		}
	}
	return false
}

func (c *Upgrader) getRollbackInfo(version, rootdir string) (string, bool, error) {
	if len(version) != 0 {
		if !c.IsExistVersion(version) {
			return "", false, errors.New("version does not exist")
		}
		var isCanRollback bool
		if len(c.rootMP) != 1 {
			logger.Info("start rollback a old version in initramfs")
			isCanRollback = true
		} else {
			logger.Info("start ready a old version to rollback")
			isCanRollback = false
		}
		if !c.recordsInfo.IsReadyRollback() || c.recordsInfo.IsSucceeded() || c.recordsInfo.IsFailed() {
			c.recordsInfo.SetReady()
		}
		logger.Debugf("set rollback info, version:%s, repo mount point:%s", version, c.conf.RepoList[0].RepoMountPoint)
		return version, isCanRollback, nil
	}

	if len(version) == 0 && !c.recordsInfo.IsSucceeded() && !c.recordsInfo.IsFailed() && c.recordsInfo.IsOper() {
		return c.recordsInfo.RollbackVersion, true, nil
	}

	return "", false, nil
}

func (c *Upgrader) Rollback(version string,
	evHandler func(op, state int32, target, desc string)) (excode int, err error) {
	exitCode := _STATE_TY_SUCCESS
	c.SendingSignal(evHandler, _OP_TY_ROLLBACK_PREPARING_START, _STATE_TY_RUNING, version, "")

	c.LoadRollbackRecords(true)
	logger.Debugf("status code for the current state file, %v", c.recordsInfo.CurrentState)
	c.SendingSignal(evHandler, _OP_TY_ROLLBACK_PREPARING_SET_CONFIG, _STATE_TY_RUNING, version, "")
	backVersion, isCanRollback, err := c.getRollbackInfo(version, c.rootMP)
	if err != nil {
		exitCode = _STATE_TY_FAILED_NO_REPO
		goto failure
	}
	if isCanRollback && len(backVersion) != 0 {
		c.UpdateProgress(0)
		logger.Infof("start rollback a old version: %s, state: %v.", backVersion, c.recordsInfo.CurrentState)
		var mountedPointList mountpoint.MountPointList

		if len(c.recordsInfo.RollbackVersion) == 0 {
			c.recordsInfo.Reset(backVersion)
		}
		// checkout specified version file
		err = c.Snapshot(backVersion)
		if err != nil {
			exitCode = _STATE_TY_FAILED_OSTREE_ROLLBACK
			goto failure
		}

		c.UpdateProgress(20)
		// need load rollback version config
		err := c.conf.LoadVersionData(backVersion, c.rootMP)
		if err != nil {
			exitCode = _STATE_TY_FAILED_OSTREE_ROLLBACK
			goto failure
		}
		c.recordsInfo.SetAfterRun(c.conf.RepoList[0].AfterRun)
		// update the mount of the first repo
		mountedPointList, err = c.UpdateMount(c.conf.RepoList[0], backVersion)
		if err != nil {
			exitCode = _STATE_TY_FAILED_HANDLING_MOUNTS
			if err != nil {
				logger.Warning(err)
			}
			goto failure
		}
		c.UpdateProgress(30)
		// rollback system files
		for _, v := range c.conf.RepoList {
			err = c.repoRollback(v, backVersion)
			if err != nil {
				exitCode = _STATE_TY_FAILED_OSTREE_ROLLBACK
				goto failure
			}
		}
		// before umount the partiton operations
		err = c.AfterRollbackOper(backVersion, true)
		if err != nil {
			logger.Warning("failed run after rollback operation, err:", err)
		}

		if len(c.rootMP) != 1 {
			var needUmountList []string
			for _, v := range mountedPointList {
				needUmountList = append(needUmountList, v.Dest)
			}
			needUmountList = util.SortSubDir(needUmountList)
			for _, v := range needUmountList {
				err = util.ExecCommand("umount", []string{v})
				logger.Info("restore system mount, will umount:", v)
				if err != nil {
					logger.Warning("failed umount, err:", err)
				}
			}
		}
		c.UpdateProgress(100)
	} else {
		c.SendingSignal(evHandler, _OP_TY_ROLLBACK_PREPARING_SET_WAITTIME, _STATE_TY_RUNING, version, "")
		out, err := exec.Command("/usr/sbin/deepin-boot-kit", "--action=mkinitrd").CombinedOutput()
		if err != nil {
			logger.Warning("update initrd failed:", string(out))
			exitCode = _STATE_TY_FAILED_UPDATE_INITRD
			goto failure
		}
		if len(c.rootMP) == 1 {
			grubManager, _ := grub.LoadGrubParams()
			newTitle := c.GrubTitle(backVersion)
			newHead, _ := util.GetBootKitText(msgRollBack, langselector.LocalLangEnv())
			newDef := newHead + ">" + newTitle
			oldT, err := grubManager.TimeOut()
			if err != nil {
				oldT = 2
			}
			oldDf, err := grubManager.GrubDefault()
			if err != nil {
				oldDf = "0"
			}
			//Before setting the configuration needs to be saved
			c.recordsInfo.SetRollbackInfo(backVersion, oldDf, newHead, oldT)
			err = grubManager.SetTimeOut(0)
			if err != nil {
				exitCode = _STATE_TY_FAILED_UPDATE_GRUB
				goto failure
			}
			grubManager.SetGrubDefault(newDef)
			exitCode, err = c.UpdateGrub()
			if err != nil {
				exitCode = _STATE_TY_FAILED_UPDATE_GRUB
				goto failure
			}
			logger.Infof("Success to set the default rollback configuratio, timeout 1, default grub %s", newDef)
		}
		logger.Info("start set rollback a old version:", backVersion)
	}
	c.SendingSignal(evHandler, _OP_TY_ROLLBACK_PREPARING_END, _STATE_TY_SUCCESS, version, "")
	logger.Info("successed run rollback action")
	return int(exitCode), nil
failure:
	//failed mount -2 < 0, running must in initramfs
	if int(exitCode) < int(_STATE_TY_FAILED_NO_REPO) && len(c.rootMP) != 1 {
		err = c.AfterRollbackOper(backVersion, false)
		if err != nil {
			logger.Warning("failed run after rollback operation, err:", err)
		}
		c.UpdateProgress(100)
	}
	c.SendingSignal(evHandler, _OP_TY_ROLLBACK_PREPARING_END, exitCode, version, err.Error())
	return int(exitCode), err
}

func (c *Upgrader) repoCommit(repoConf *config.RepoConfig, newVersion, subject string,
	useSysData bool, evHandler func(op, state int32, target, desc string)) error {
	handler := c.repoSet[repoConf.Repo]
	var dataDir, usrDir string
	defer func() {
		// remove tmp dir
		_ = os.RemoveAll(filepath.Join(c.rootMP, c.conf.CacheDir))
	}()
	if useSysData {
		c.SendingSignal(evHandler, _OP_TY_COMMIT_PREPARE_DATA, _STATE_TY_RUNING, newVersion, "")
		if chroot.IsEnv() {
			usrDir = "/usr"
		} else {
			usrDir = c.getMostSpaceDir(c.rootMP, repoConf.SubscribeList)
		}
		c.conf.SetCacheDir(filepath.Join(usrDir, ".osrepo-cache"))
		logger.Infof("to reset the temporary directory %s", c.conf.CacheDir)
		// need to delete the repo to take up space, if a repo in the subscribeList
		var extraSize int64
		for _, v := range repoConf.SubscribeList {
			if repoConf.RepoMountPoint == v {
				extraSize += dirinfo.GetDirSize(repoConf.Repo)
				extraSize += dirinfo.GetDirSize(repoConf.SnapshotDir)
				extraSize += dirinfo.GetDirSize(repoConf.StageDir)
				break
			}
		}
		isEnough, err := c.isDirSpaceEnough(usrDir, c.rootMP, repoConf.SubscribeList, 0-extraSize, true)
		if err != nil || !isEnough {
			c.SendingSignal(evHandler, _OP_TY_COMMIT_PREPARE_DATA, _STATE_TY_FAILED_NO_SPACE, newVersion, "")
			return err
		}
		// need handle filter dirs
		repoConf.FilterList = append(repoConf.FilterList, c.getFilterList(repoConf.FilterList, repoConf.SubscribeList)...)
		repoConf.FilterList = util.RemoveSameItemInSlice(repoConf.FilterList)
		dataDir = filepath.Join(c.rootMP, c.conf.CacheDir, c.conf.Distribution)
		err = c.copyRepoData(c.rootMP, dataDir, repoConf.SubscribeList, repoConf.FilterList)
		if err != nil {
			return err
		}
	}
	c.SendingSignal(evHandler, _OP_TY_COMMIT_REPO_SUBMIT, _STATE_TY_RUNING, newVersion, "")
	logger.Debugf("will submitted version to the repo, version:%s, sub:%s, dataDir:%s", newVersion, subject, dataDir)
	err := handler.Commit(newVersion, subject, dataDir)
	if err != nil {
		return err
	}
	return nil
}

func (c *Upgrader) getFilterList(filterlist, sublist []string) []string {
	var filterList []string
	if util.IsItemInList("/", sublist) {
		filterList = append(filterList, util.FullNeedFilters()...)
	}
	for _, fs := range c.fsInfo {
		for _, filter := range filterlist {
			// ex: '/pesistent/home /home' '/pesistent/home/uos/A'
			// ex: '/pesistent/home /home' '/home/uos/A'
			// ex: '/pesistent/home /home' '/persistent'
			// ex: '/boot/persistent /home/pesistent' '/home'
			if fs.Bind {
				src := util.TrimRootdir(c.rootMP, fs.SrcPoint)
				dst := util.TrimRootdir(c.rootMP, fs.DestPoint)
				if strings.HasPrefix(filter, src) {
					filterList = append(filterList, filepath.Join(dst, strings.TrimPrefix(filter, src)))
				}
				if strings.HasPrefix(filter, dst) {
					filterList = append(filterList, filepath.Join(src, strings.TrimPrefix(filter, dst)))
				}
				if strings.HasPrefix(src, filter) || strings.HasPrefix(dst, filter) {
					filterList = append(filterList, src, dst)
				}
			}
			if fs.Remote {
				dst := util.TrimRootdir(c.rootMP, fs.DestPoint)
				filterList = append(filterList, dst)
			}
		}
	}
	filtersMountInfo, err := mountinfo.GetFilterInfo(SelfMountPath)
	if err == nil {
		for _, sub := range sublist {
			info := filtersMountInfo.Query(sub)
			for _, v := range info {
				if !util.IsRootSame(filterList, v.MountPoint) {
					filterList = append(filterList, v.MountPoint)
				}
			}
		}
	}
	return filterList
}

func (c *Upgrader) repoSnapShot(repoConf *config.RepoConfig, version string) error {
	handler := c.repoSet[repoConf.Repo]
	dataDir := filepath.Join(c.rootMP, repoConf.SnapshotDir, version)
	_ = os.RemoveAll(dataDir)
	return handler.Snapshot(version, dataDir)
}

func (c *Upgrader) enableSnapshotBoot(snapDir, version string) error {
	bootDir := filepath.Join(snapDir, "boot")
	fiList, err := ioutil.ReadDir(bootDir)
	if err != nil {
		return err
	}
	dstDir := filepath.Join(c.rootMP, "boot/snapshot", version)
	localBootDir := filepath.Join(c.rootMP, "/boot")
	err = os.MkdirAll(dstDir, 0700)
	if err != nil {
		return err
	}

	found := false
	for _, fi := range fiList {
		if fi.IsDir() {
			continue
		}
		if strings.HasPrefix(fi.Name(), "vmlinuz-") ||
			strings.HasPrefix(fi.Name(), "kernel-") ||
			strings.HasPrefix(fi.Name(), "vmlinux-") ||
			strings.HasPrefix(fi.Name(), "initrd.img-") {

			snapFile := filepath.Join(bootDir, fi.Name())
			localFile := filepath.Join(localBootDir, fi.Name())
			dstFile := filepath.Join(dstDir, fi.Name())
			isSame, err := util.IsFileSame(localFile, snapFile)
			if isSame && err == nil {
				// create file hard link
				err = util.CopyFile(localFile, dstFile, true)
			} else {
				err = util.CopyFile(snapFile, dstFile, false)
			}

			if err != nil {
				_ = os.Remove(dstDir)
				return err
			}
			found = true
		}
	}
	if !found {
		_ = os.Remove(dstDir)
	}
	return nil
}

// @title    handleRepoRollbak
// @description   handling files on rollback
// @param     realDir         	string         		"original system file path, ex:/etc"
// @param     snapDir         	string         		"snapshot file path, ex:/persitent/osroot"
// @param     version         	string         		"snapshot version, ex:v23.0.0.1"
// @param     rollbackDirList   *[]string      		"rollback produces tmp files, ex:/etc/.old"
// @param     HandlerDir   		function pointer    "file handler function pointer"
func (c *Upgrader) handleRepoRollbak(realDir, snapDir, version string, filterMountedList []string,
	rollbackDirList *[]string, HandlerDir func(src, dst, version, rootDir string, filter []string) (string, error)) error {
	var filterDirs []string
	var rollbackDir string
	var err error
	if util.IsItemInList(realDir, filterMountedList) {
		logger.Debugf("need filter mount point %s", realDir)
		return nil
	}

	// need trim root dir
	realDir = util.TrimRootdir(c.rootMP, realDir)
	list := c.mountInfos.Query(filepath.Join(c.rootMP, realDir))
	logger.Debugf("start rolling back, realDir:%s, snapDir:%s, version:%s, list len:%d",
		realDir, snapDir, version, len(list))
	if len(list) > 0 {
		rootPartition, err := dirinfo.GetDirPartition(filepath.Join(c.rootMP, realDir))
		if err != nil {
			return err
		}
		for _, l := range list {
			if l.MountPoint == filepath.Join(c.rootMP, realDir) {
				continue
			}
			if util.IsRootSame(filterDirs, l.MountPoint) {
				continue
			}
			if rootPartition != l.Partition {
				filterDirs = append(filterDirs, l.MountPoint)
			}
		}
		logger.Debugf("the filter directory path is %s", filterDirs)
	}

	rollbackDir, err = HandlerDir(filepath.Join(snapDir+realDir), realDir, version, c.rootMP, filterDirs)
	if err != nil {
		logger.Warningf("fail rollback dir:%s,err:%v", realDir, err)
		return err
	} else {
		*rollbackDirList = append(*rollbackDirList, rollbackDir)
		logger.Debug("rollbackDir:", rollbackDir)
	}

	for _, l := range filterDirs {
		err = c.handleRepoRollbak(l, snapDir, version, filterMountedList, rollbackDirList, HandlerDir)
		if err != nil {
			return err
		}
	}
	return nil
}

func (c *Upgrader) isSameFilterPartDir(filterdir string, subscribeList []string) bool {
	filterPart, _ := dirinfo.GetDirPartition(filepath.Join(c.rootMP, filterdir))
	for _, dir := range subscribeList {
		if strings.HasPrefix(filepath.Join(c.rootMP, filterdir), filepath.Join(c.rootMP, dir)) {
			dirPart, _ := dirinfo.GetDirPartition(filepath.Join(c.rootMP, dir))
			if dirPart != filterPart {
				return false
			}
		}
	}
	return true
}

func (c *Upgrader) repoRollback(repoConf *config.RepoConfig, version string) error {
	var FilterPartMountedList, rollbackDirList []string
	repoConf.FilterList = append(repoConf.FilterList, c.getFilterList(repoConf.FilterList, repoConf.SubscribeList)...)
	repoConf.FilterList = util.RemoveSameItemInSlice(repoConf.FilterList)
	logger.Debugf("need filter dir list %v", repoConf.FilterList)
	for _, v := range repoConf.FilterList {
		//to determine whether filter dirs is in fstab
		if !c.fsInfo.IsInFstabPoint(c.rootMP, v) {
			continue
		}
		//to determine whether a rollback partition and filter partitions are the same
		if c.isSameFilterPartDir(v, repoConf.SubscribeList) {
			continue
		}
		if util.IsItemInList(v, FilterPartMountedList) {
			continue
		}
		FilterPartMountedList = append(FilterPartMountedList, filepath.Join(c.rootMP, v))
	}
	logger.Debugf("need filter part mount list %v", FilterPartMountedList)

	snapDir := filepath.Join(repoConf.SnapshotDir, version)
	realDirSubscribeList, realFileSubcribeList := util.GetRealDirList(repoConf.SubscribeList, c.rootMP, snapDir)
	logger.Debugf("will recovery dirs %v, files %v", realDirSubscribeList, realFileSubcribeList)
	var err error
	defer func() {
		c.UpdateProgress(70)
		// if failed update, restoring the system
		if err != nil || c.recordsInfo.IsRestore() {
			c.recordsInfo.SetRestore()
			logger.Warning("failed rollback, recover rollback action")
			for _, dir := range realDirSubscribeList {
				err := c.handleRepoRollbak(dir, snapDir, version, FilterPartMountedList, &rollbackDirList, util.HandlerDirRecover)
				if err != nil {
					logger.Error("failed recover rollback, err:", err)
				}
			}
			if err == nil {
				err = errors.New("failed rollback dir")
			}
		}
		logger.Debug("need to be deleted tmp dirs:", rollbackDirList)
		// remove all tmp dir and compatible rollback
		for _, v := range rollbackDirList {
			oldDir := filepath.Join(path.Dir(v), string("/.old")+version)
			newDir := filepath.Join(path.Dir(v), string("/.")+version)
			if util.IsExists(oldDir) {
				err = os.RemoveAll(oldDir)
				if err != nil {
					// When failure to delete extended attributes 'i' delete again
					util.RemoveDirAttr(oldDir)
					err = os.RemoveAll(oldDir)
					if err != nil {
						logger.Warning("failed remove dir, err:", err)
					}
				}
			}
			if util.IsExists(newDir) {
				err = os.RemoveAll(newDir)
				if err != nil {
					// When failure to delete extended attributes 'i' delete again
					util.RemoveDirAttr(newDir)
					err = os.RemoveAll(newDir)
					if err != nil {
						logger.Warning("failed remove dir, err:", err)
					}
				}
			}
		}
	}()
	if c.recordsInfo.IsNeedMainRunning() {
		// prepare the repo file under the system path
		for _, dir := range realDirSubscribeList {
			err = c.handleRepoRollbak(dir, snapDir, version, FilterPartMountedList, &rollbackDirList, util.HandlerDirPrepare)
			if err != nil {
				return err
			}
		}

		c.UpdateProgress(40)
		// hardlink need to filter file or dir to prepare dir
		for _, dir := range rollbackDirList {
			dirRoot := filepath.Dir(dir)
			filterDirs, filterFiles := util.HandlerFilterList(c.rootMP, dirRoot, repoConf.FilterList)
			rootPartition, err := dirinfo.GetDirPartition(dirRoot)
			if err != nil {
				logger.Warningf("failed get %s partition", dirRoot)
				continue
			}
			for _, v := range filterDirs {
				dirPartition, err := dirinfo.GetDirPartition(v)
				if err != nil {
					logger.Warningf("failed get %s partition", v)
					continue
				}
				if dirPartition != rootPartition {
					continue
				}
				dest := filepath.Join(dir, strings.TrimPrefix(v, dirRoot))
				util.CopyDir(v, dest, nil, nil, true)
				logger.Debugf("ignore dir path:%s", dest)
			}
			for _, v := range filterFiles {
				filePartition, err := dirinfo.GetDirPartition(v)
				if err != nil {
					logger.Warningf("failed get %s partition", v)
					continue
				}
				if filePartition != rootPartition {
					continue
				}
				dest := filepath.Join(dir, strings.TrimPrefix(v, dirRoot))
				util.CopyFile(v, dest, true)
				logger.Debugf("ignore file path:%s", dest)
			}
		}
		var bootDir string
		// repo files replace system files
		c.UpdateProgress(60)
		for _, dir := range realDirSubscribeList {
			logger.Debug("start replacing the dir:", dir)
			if strings.HasSuffix(filepath.Join(c.rootMP, "/boot"), dir) {
				logger.Debugf("the %s needs to be replaced last", dir)
				bootDir = dir
				continue
			}
			err = c.handleRepoRollbak(dir, snapDir, version, FilterPartMountedList, &rollbackDirList, util.HandlerDirRollback)
			if err != nil {
				return err
			}
		}
		// last replace /boot dir, protect system boot
		if len(bootDir) != 0 {
			err = c.handleRepoRollbak(bootDir, snapDir, version, FilterPartMountedList, &rollbackDirList, util.HandlerDirRollback)
			if err != nil {
				return err
			}
		}

		// replace file is fast
		if len(realFileSubcribeList) != 0 {
			for _, v := range realFileSubcribeList {
				realFile := util.TrimRootdir(c.rootMP, v)
				snapFile := filepath.Join(snapDir, realFile)
				logger.Debugf("start rolling back file, realfile:%s, snapFile:%s",
					realFile, snapFile)
				err := util.CopyFile(filepath.Join(c.rootMP, snapFile), filepath.Join(c.rootMP, realFile), false)
				if err != nil {
					return err
				}
			}
		}
	}
	return nil
}

func (c *Upgrader) copyRepoData(rootDir, dataDir string,
	subscribeList []string, filterList []string) error {
	//need filter '/usr/.v23'
	repoCacheDir := filepath.Join(c.rootMP, c.conf.CacheDir)
	os.Mkdir(repoCacheDir, 0755)
	filterList = append(filterList, repoCacheDir)

	for _, dir := range subscribeList {
		srcDir := filepath.Join(rootDir, dir)
		filterDirs, filterFiles := util.HandlerFilterList(rootDir, srcDir, filterList)
		logger.Debugf("the filter directory path is %s, the filter file path is %s", filterDirs, filterFiles)
		if !util.IsExists(srcDir) {
			logger.Info("[copyRepoData] src dir empty:", srcDir)
			continue
		}
		fi, err := os.Stat(srcDir)
		if err != nil {
			continue
		}
		if fi.IsDir() {
			logger.Info("[copyRepoData] src dir:", srcDir)
			err := util.CopyDir(srcDir, filepath.Join(dataDir, dir), filterDirs, filterFiles, true)
			if err != nil {
				return err
			}
		} else {
			logger.Info("[copyRepoData] src file:", srcDir)
			err := util.CopyFile2(srcDir, filepath.Join(dataDir, dir), fi, true)
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func (c *Upgrader) getMostSpaceDir(rootDir string, subscribeList []string) string {
	mp := make(map[string]int64)
	var maxUsePart string
	var max, total int64
	for _, dir := range subscribeList {
		srcDir := filepath.Join(rootDir, dir)
		part, err := dirinfo.GetDirPartition(srcDir)
		if err != nil {
			continue
		}
		v, ok := mp[part]
		if !ok {
			mp[part] = dirinfo.GetDirSize(srcDir)
		} else {
			mp[part] += v + dirinfo.GetDirSize(srcDir)
		}
		if max < mp[part] {
			max = mp[part]
			maxUsePart = part
		}
	}
	info := c.mountInfos.MatchPartition(maxUsePart)
	free, _ := dirinfo.GetPartitionFreeSize(info.MountPoint)
	for v := range mp {
		total += mp[v]
	}
	logger.Debugf("maximum use space partition %s,", maxUsePart)
	if int64(free) > (total - max) {
		return info.MountPoint
	}
	return c.mountInfos.MaxPartition(subscribeList)
}

func (c *Upgrader) isDirSpaceEnough(mountpoint, rootDir string, subscribeList []string,
	extraSize int64, isFilterPartiton bool) (bool, error) {
	var needSize int64

	mountPart, err := dirinfo.GetDirPartition(mountpoint)
	logger.Debugf("the dir is:%s, the partiton is:%s", mountpoint, mountPart)
	if err != nil {
		return false, err
	}
	for _, dir := range subscribeList {
		srcDir := filepath.Join(rootDir, dir)
		if !util.IsExists(srcDir) {
			continue
		}

		part, err := dirinfo.GetDirPartition(srcDir)
		logger.Debugf("the dir is:%s, the partiton is:%s", srcDir, part)
		if err != nil {
			continue
		}

		if isFilterPartiton && part == mountPart {
			continue
		}
		//the repo is full submission, so need add hard link size
		needSize += dirinfo.GetDirSize(srcDir)
	}
	GB := 1024 * 1024 * 1024
	free, _ := dirinfo.GetPartitionFreeSize(mountPart)
	if (needSize + extraSize) > 0 {
		needSize = needSize + extraSize
	}
	logger.Debugf("the %s partition free size:%.2f GB, extra size:%.2f GB, the need size is:%.2f GB", mountPart,
		float64(free)/float64(GB), float64(extraSize)/float64(GB), float64(needSize)/float64(GB)+float64(extraSize)/float64(GB))
	if needSize > int64(free) {
		return false, errors.New("the current partition is out of space")
	}
	return true, nil
}

func (c *Upgrader) updateLoaclMount(snapDir string) (mountpoint.MountPointList, error) {
	fstabDir := filepath.Clean(filepath.Join(snapDir, "/etc/fstab"))
	if !util.IsExists(fstabDir) || util.IsEmptyFile(fstabDir) {
		fstabDir = filepath.Clean(filepath.Join(c.rootMP, "/etc/fstab"))
	}
	_, err := ioutil.ReadFile(fstabDir)
	var mountedPointList mountpoint.MountPointList
	if err != nil {
		return mountedPointList, err
	}
	c.fsInfo, err = fstabinfo.Load(fstabDir, c.rootMP)
	if err != nil {
		logger.Debugf("the %s file does not exist in the snapshot, read the local fstabl", fstabDir)
		return mountedPointList, err
	}
	rootPartition, err := dirinfo.GetDirPartition(c.rootMP)
	if err != nil {
		return mountedPointList, err
	}
	for _, info := range c.fsInfo {
		if info.SrcPoint == rootPartition || info.DestPoint == "/" || info.Remote {
			logger.Debugf("ignore mount point %s", info.DestPoint)
			continue
		}
		logger.Debugf("bind:%v,SrcPoint:%v,DestPoint:%v", info.Bind,
			info.SrcPoint, filepath.Clean(filepath.Join(c.rootMP, info.DestPoint)))
		m := c.mountInfos.Match(filepath.Clean(filepath.Join(c.rootMP, info.DestPoint)))
		if m != nil && !info.Bind {
			if m.Partition != info.SrcPoint || strings.Contains(m.Options, "ro") {
				logger.Infof("the %s is mounted %s, not mounted correctly and needs to be unmouted",
					m.Partition, m.MountPoint)
				newInfo := &mountpoint.MountPoint{
					Src:     m.Partition,
					Dest:    m.MountPoint,
					FSType:  m.FSType,
					Options: m.Options,
				}
				err := newInfo.Umount()
				if err != nil {
					continue
				}
				err = os.RemoveAll(newInfo.Dest)
				if err != nil {
					return mountedPointList, err
				}
			} else {
				continue
			}
		}
		mp := filepath.Join(c.rootMP, info.DestPoint)
		logger.Infof("the %s is not mounted and needs to be mouted", mp)
		oldInfo := &mountpoint.MountPoint{
			Src:     info.SrcPoint,
			Dest:    mp,
			FSType:  info.FSType,
			Options: info.Options,
			Bind:    info.Bind,
		}
		err := oldInfo.Mount()
		mountedPointList = append(mountedPointList, oldInfo)
		if err != nil {
			logger.Error("failed to mount dir", mp)
			err = oldInfo.Umount()
			if err != nil {
				logger.Error("failed to umount dir:", err)
			}
			return mountedPointList, err
		}
	}
	return mountedPointList, nil
}

func (c *Upgrader) RepoAutoCleanup() (bool, error) {
	handler, err := repo.NewRepo(repo.REPO_TY_OSTREE,
		filepath.Join(c.rootMP, c.conf.RepoList[0].Repo))
	if err != nil {
		return false, err
	}
	maxVersion := int(c.conf.MaxVersionRetention)
	list, err := handler.List()
	if err != nil {
		return false, err
	}
	if len(list) <= maxVersion {
		logger.Infof("current version is less than %d, no need for auto cleanup", maxVersion)
		return false, nil
	}
	logger.Infof("current version is more than %d, need for cleanup repo", maxVersion)

	for i, v := range list {
		if i == len(list)-1 {
			continue
		}
		if i < maxVersion-1 {
			continue
		}
		_, err = c.Delete(v, nil)
		if err != nil {
			logger.Warning(err)
			break
		}
	}
	return true, nil
}

func (c *Upgrader) Delete(version string,
	evHandler func(op, state int32, target, desc string)) (excode int, err error) {
	exitCode := _STATE_TY_SUCCESS
	var bootDir, snapshotDir, fisrt string
	var handler repo.Repository
	c.SendingSignal(evHandler, _OP_TY_DELETE_START, _STATE_TY_RUNING, version, "")
	if len(c.conf.RepoList) == 0 || len(version) == 0 {
		err = errors.New("wrong version number")
		exitCode = _STATE_TY_FAILED_NO_REPO
		goto failure
	}
	handler, err = repo.NewRepo(repo.REPO_TY_OSTREE,
		filepath.Join(c.rootMP, c.conf.RepoList[0].Repo))
	if err != nil {
		exitCode = _STATE_TY_FAILED_NO_REPO
		goto failure
	}
	fisrt, err = handler.First()
	if err != nil {
		exitCode = _STATE_TY_FAILED_NO_REPO
		goto failure
	}
	if fisrt == version || c.conf.ActiveVersion == version {
		err = errors.New("the current activated version does not allow deletion")
		exitCode = _STATE_TY_FAILED_VERSION_DELETE
		goto failure
	}
	err = handler.Delete(version)
	if err != nil {
		exitCode = _STATE_TY_FAILED_NO_VERSION
		goto failure
	}
	snapshotDir = filepath.Join(c.rootMP, c.conf.RepoList[0].SnapshotDir, version)
	logger.Debug("delete tmp snapshot directory:", snapshotDir)
	_ = os.RemoveAll(snapshotDir)
	bootDir = filepath.Join(c.rootMP, "boot/snapshot", version)
	logger.Debug("delete kernel snapshot directory:", bootDir)
	_ = os.RemoveAll(bootDir)

	c.SendingSignal(evHandler, _OP_TY_DELETE_GRUB_UPDATE, _STATE_TY_RUNING, version, "")
	exitCode, err = c.UpdateGrub()
	if err != nil {
		exitCode = _STATE_TY_FAILED_UPDATE_GRUB
		goto failure
	}
	c.SendingSignal(evHandler, _OP_TY_DELETE_END, exitCode, version, "")
	return int(exitCode), nil
failure:
	c.SendingSignal(evHandler, _OP_TY_DELETE_END, exitCode, version, err.Error())
	return int(exitCode), err
}

func (c *Upgrader) IsAutoClean() bool {
	if len(c.conf.RepoList) == 0 {
		return true
	}
	return c.conf.AutoCleanup
}

func (c *Upgrader) GenerateBranchName() (string, error) {
	if len(c.conf.RepoList) != 0 {
		handler, err := repo.NewRepo(repo.REPO_TY_OSTREE,
			c.conf.RepoList[0].Repo)
		if err != nil {
			return "", err
		}
		name, err := handler.Last()
		if err != nil {
			return "", err
		}
		return branch.Increment(name)
	}
	return branch.GenInitName(c.conf.Distribution), nil
}

func (c *Upgrader) ListVersion() ([]string, int, error) {
	exitCode := _STATE_TY_SUCCESS
	if len(c.conf.RepoList) == 0 {
		exitCode = _STATE_TY_FAILED_NO_REPO
		return nil, int(exitCode), nil
	}

	handler, err := repo.NewRepo(repo.REPO_TY_OSTREE,
		filepath.Join(c.rootMP, c.conf.RepoList[0].Repo))
	if err != nil {
		exitCode = _STATE_TY_FAILED_NO_REPO
		return nil, int(exitCode), err
	}
	list, err := handler.List()
	if err != nil {
		exitCode = _STATE_TY_FAILED_NO_REPO
		return nil, int(exitCode), err
	}
	return list, int(exitCode), err
}

func (c *Upgrader) DistributionName() string {
	return c.conf.Distribution
}

func (c *Upgrader) Subject(version string) (string, error) {
	var sub string
	handler, err := repo.NewRepo(repo.REPO_TY_OSTREE,
		filepath.Join(c.rootMP, c.conf.RepoList[0].Repo))
	if err != nil {
		return sub, err
	}
	if !handler.Exist(version) {
		return sub, errors.New("failed get subject, the current version does not exist version")
	}
	return handler.Subject(version)
}

func (c *Upgrader) RepoMountpointAndUUID() (string, string, error) {
	list, _, _ := c.ListVersion()
	if len(list) != 0 {
		diskInfo := c.fsInfo.MatchDestPoint(c.conf.RepoList[0].RepoMountPoint)
		return diskInfo.SrcPoint, diskInfo.DiskUUID, nil
	}
	repoMountPoint, uuid := c.fsInfo.MaxFreePartitionPoint()
	for _, v := range c.conf.RepoList {
		// need keep 5GB free space
		isEnough, err := c.isDirSpaceEnough(repoMountPoint, c.rootMP, v.SubscribeList, LessKeepSize, false)
		if err != nil {
			logger.Warning(err)
			continue
		}
		if !isEnough {
			return "", "", err
		}
	}
	return repoMountPoint, uuid, nil
}

func (c *Upgrader) SetReadyData(path string) error {
	return c.conf.SetReadyData(path)
}

func (c *Upgrader) ReadyDataPath() string {
	return c.conf.ReadyDataPath()
}

func (c *Upgrader) ResetGrub() {
	m := grub.Init()
	err := m.Reset()
	if err != nil {
		logger.Warning("failed reset grub, err:", err)
	} else {
		m.Join()
		util.ExecCommand("/usr/bin/sync", []string{})
		time.Sleep(2 * time.Second)
		c.UpdateGrub()
	}
	c.recordsInfo.Remove()
}

func (c *Upgrader) AfterRollbackOper(backVersion string, isSuccessful bool) error {
	// rollback ending and need notify
	defer func() {
		if isSuccessful {
			c.recordsInfo.SetSuccessfully()
		} else {
			c.recordsInfo.SetFailed(backVersion)
		}
		logger.Debugf("save result records and remove state records, backVersion:%s, state: %v, after run:%s",
			backVersion, int(c.recordsInfo.CurrentState), c.recordsInfo.AferRun)
		err := c.recordsInfo.SaveResult(c.rootMP)
		if err != nil {
			logger.Warning(err)
		}
		record := filepath.Join(c.rootMP, SelfRecordStatePath)
		if util.IsExists(record) {
			os.RemoveAll(record)
		}
	}()
	dst := filepath.Join(c.rootMP, AutoStartDesktopPath)
	err := util.CopyFile(filepath.Join(c.rootMP, LocalNotifyDesktopPath), dst, false)
	if err != nil {
		logger.Warning(err)
	}
	err = os.Chmod(dst, 0644)
	if err != nil {
		logger.Warning(err)
	}
	c.UpdateProgress(90)
	// restore mount points under initramfs and save action version
	c.SaveActiveVersion(backVersion)
	if c.recordsInfo.IsAfterOper() {
		m, err := chroot.Start(c.rootMP)
		if err != nil {
			return err
		}
		logger.Debug("start update grub config")
		//need restore default grub config
		gb, err := grub.LoadGrubParams()
		if err != nil {
			logger.Warning("failed get grub config")
		} else {
			t, err := gb.TimeOut()
			logger.Debugf("current grub time out %v, old grub time out %v", t, c.recordsInfo.TimeOut)
			if t == 0 && err == nil {
				gb.SetTimeOut(uint(c.recordsInfo.TimeOut))
				gb.SetGrubDefault((c.recordsInfo.GrubDefault))
			}
		}
		c.UpdateGrub()
		err = m.Exit()
		if err != nil {
			return err
		}
	}
	// save log to /var/log/deepin-upgrade-manager
	logger.CopyLogFile(c.rootMP)
	return nil
}

func (c *Upgrader) UpdateProgress(progress int) {
    if progress == 0 {
    	logger.Debugf("activate the upgrade roll back progress theme")
    	util.ExecCommand("/usr/bin/plymouth", []string{"change-mode", "--system-upgrade"})
    }
    logger.Infof("update progress %d", progress)
    plymouth.UpdateProgress(progress)
	fmt.Println("update progress:", progress)
}

func (c *Upgrader) SetRepoMount(repoMount string) (*config.Config, error) {
	var err error
	if !util.IsExists(repoMount) {
		return nil, errors.New("repo mount isn't exist")
	}
	c.conf.ChangeRepoMountPoint(repoMount)
	c.conf, err = c.conf.ReLoadConfig(c.rootMP, repoMount)
	return c.conf, err
}

func (c *Upgrader) SendingExitSignal(evHandler func(op, state int32, target, desc string)) {
	end := (currentState.CurOp/100+1)*100 - 1
	if end > 100 {
		c.SendingSignal(evHandler, end, _STATE_TY_FAILED_EXIT_SIGNAL, currentState.CurVersion, "")
	}
}

func (c *Upgrader) SendingSignal(evHandler func(op, state int32, target, desc string),
	op opType, exitCode stateType, version, err string) {
	if evHandler == nil {
		return
	}
	currentState.CurOp = op
	currentState.CurVersion = version
	if len(err) != 0 {
		evHandler(int32(op), int32(exitCode), (version),
			fmt.Sprintf("%s: %s: %s", op.String(), exitCode.String(), err))
	} else {
		evHandler(int32(op), int32(exitCode), (version),
			fmt.Sprintf("%s: %s", op.String(), exitCode.String()))
	}
}

func (c *Upgrader) SendSystemNotice() error {
	var backMsg, grubTitle, mode string
	const atomicUpgradeDest = "org.deepin.AtomicUpgrade1"
	const atomicUpgradePath = "/org/deepin/AtomicUpgrade1"

	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}
	grubServiceObj := sysBus.Object(atomicUpgradeDest,
		atomicUpgradePath)
	res, afterRun, err := records.ReadResult()
	logger.Debugf("current self run %s, res %v", afterRun, res)
	var ret dbus.Variant
	dbusErr := grubServiceObj.Call("org.freedesktop.DBus.Properties.Get", 0, atomicUpgradeDest, "ActiveVersion").Store(&ret)
	activeVersion := ret.Value().(string)
	if err != nil || dbusErr != nil {
		return fmt.Errorf("failed to send os notify, err: %v, dbusErr: %v", err, dbusErr)
	} else {
		metho := atomicUpgradeDest + ".GetGrubTitle"
		var ret dbus.Variant
		grubServiceObj.Call(metho, 0, activeVersion).Store(&ret)
		grubTitle = ret.Value().(string)
	}
	defer func() {
		metho := atomicUpgradeDest + ".CancelRollback"
		err := grubServiceObj.Call(metho, 0).Store()
		if err != nil {
			logger.Warning("failed send system notice, err:", err)
		}
	}()
	if res == 1 {
		text, err := util.GetUpgradeText(msgSuccessRollBack, []string{})
		if err != nil {
			logger.Warningf("run gettext error: %v", err)
		}
		msg := fmt.Sprintf(" %s", grubTitle)
		backMsg = fmt.Sprintf(text, msg)
		mode = "100"
	}

	if res == 0 {
		text, err := util.GetUpgradeText(msgFailRollBack, []string{})
		if err != nil {
			logger.Warningf("run gettext error: %v", err)
		}
		msg := fmt.Sprintf(" %s", grubTitle)
		backMsg = fmt.Sprintf(text, msg)
		mode = "101"
	}
	if len(backMsg) != 0 {
		time.Sleep(5 * time.Second) // wait for osd dbus
		const selfRuning = "/usr/bin/deepin-upgrade-manager-tool --action=notify"
		if len(afterRun) > 0 && afterRun != selfRuning {
			context := strings.Fields(afterRun)
			var arg []string
			action := context[0]
			if len(context) > 1 {
				arg = context[1:]
			}
			arg = append(arg, "--mode="+mode)
			logger.Debugf("exec command %s,action %s", afterRun, arg)
			return util.ExecCommand(action, arg)
		} else {
			fmt.Println(backMsg)
			return notify.SetNotifyText(backMsg)
		}
	}
	return nil
}

func (c *Upgrader) LoadRollbackRecords(needcreated bool) error {
	var recordsInfo *records.RecordsInfo

	// save rollback records state
	if needcreated || util.IsExists(filepath.Join(c.rootMP, SelfRecordStatePath)) {
		var repoPart string
		//prevent user power can't read config
		if c.conf != nil {
			repoPart = c.conf.RepoList[0].RepoMountPoint
		}
		recordsInfo = records.LoadRecords(c.rootMP, SelfRecordStatePath, repoPart, needcreated)
	} else {
		return errors.New("failed load rollback records, the file does not exist")
	}
	c.recordsInfo = recordsInfo
	return nil
}

func (c *Upgrader) ClearResult() bool {
	// need remove auto start desktop
	if util.IsExists(AutoStartDesktopPath) && records.RemoveResult() {
		os.RemoveAll(AutoStartDesktopPath)
		return true
	}
	return false
}
