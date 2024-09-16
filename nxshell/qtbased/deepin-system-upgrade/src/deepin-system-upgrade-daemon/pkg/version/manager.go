// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package version

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"syscall"
	"time"

	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"

	"deepin-system-upgrade-daemon/pkg/iso"
	"deepin-system-upgrade-daemon/pkg/module/dbustools"
	"deepin-system-upgrade-daemon/pkg/module/disk"
	"deepin-system-upgrade-daemon/pkg/module/user"
)

var logger = log.NewLogger("deepin-system-upgrade/version")

// DBus path and DBus interface
const (
	dbusPath      = "/org/deepin/SystemUpgrade1/VersionManager"
	dbusInterface = "org.deepin.SystemUpgrade1.VersionManager"
)

// Configuration files carried by the upgrade tool
const (
	UpgradeConfig = "/etc/deepin-system-upgrade/upgrade.yaml"
	BackupConfig  = "/etc/deepin-system-upgrade/backup.yaml"
)

type StateChangeReply struct {
	operate int32
	state   int32
	version string
	message string
}

type CommitInfo struct {
	SubmissionTime string
	SystemVersion  string
	SubmissionType int
	UUID           string
	Note           string
}

type VersionManager struct {
	service *dbusutil.Service

	isoPath       string
	activeVersion string

	methods *struct {
		PrepareForUpgrade    func() `in:"isoPath"`
		BackupSystem         func()
		StartSystemUpgrade   func()
		RecoverSystem        func()
		SetPlymouthTheme     func() `in:"theme"`
		RestorePlymouthTheme func()
	}

	signals *struct {
		ProgressValue struct {
			value int64
		}
	}
}

func (*VersionManager) GetInterfaceName() string {
	return dbusInterface
}

func newManager(service *dbusutil.Service) *VersionManager {
	m := &VersionManager{
		service: service,
	}
	return m
}

// Prepare for system upgrades
func (v *VersionManager) PrepareForUpgrade(sender dbus.Sender, isoPath string) *dbus.Error {
	homeDir, err := user.GetHomeDirBySender(sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	v.emitProgressValue(10)
	// Submit the new image root directory to the ostree repository.
	root, repo, err := iso.PrepareNewRootForCommit(homeDir, isoPath)
	if err != nil {
		return dbusutil.ToError(err)
	}
	v.isoPath = isoPath
	v.emitProgressValue(30)
	// Submit the current system to the Ostree warehouse for system rollback
	err = commitNewSystemDir(root, repo)
	if err != nil {
		return dbusutil.ToError(err)
	}
	v.emitProgressValue(70)
	err = iso.HandleAfterCommit(root, repo)
	if err != nil {
		return dbusutil.ToError(err)
	}
	v.activeVersion, err = getActiveVersion()
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	// Guarantee that the Linglong application can be used normally after the upgrade
	err = enableLinglongApp(root, repo)
	if err != nil {
		logger.Warning("failed to enable linglong app:", err)
		return dbusutil.ToError(errors.New("failed to enable linglong app"))
	}
	homeDir, err = user.GetHomeDirBySender(sender)
	if err != nil {
		logger.Warning(err)
	}
	err = os.RemoveAll(filepath.Join(homeDir, ".cache/extract"))
	if err != nil {
		logger.Warning(err)
	}
	v.emitProgressValue(100)
	return nil
}

func (v *VersionManager) BackupSystem(sender dbus.Sender) *dbus.Error {
	logger.Warning("begin to backup current system")
	homeDir, err := user.GetHomeDirBySender(sender)
	if err != nil {
		return dbusutil.ToError(err)
	}

	err = modifyBackupConfig(homeDir, BackupConfig)
	if err != nil {
		logger.Warning("failed to modify backup config:", err)
	}
	err = dbustools.DBusMethodCaller("org.deepin.AtomicUpgrade1", "SetDefaultConfig", "/org/deepin/AtomicUpgrade1", BackupConfig)
	if err != nil {
		return dbusutil.ToError(err)
	}

	out, err := exec.Command("/usr/sbin/update-initramfs", "-u", "-k", "all").CombinedOutput()
	if err != nil {
		logger.Warning("failed to update initramfs:", string(out))
	}

	replyCh := make(chan StateChangeReply, 20)
	err = monitorStateChange(replyCh)
	if err != nil {
		return dbusutil.ToError(err)
	}
	systemInfo := getSystemVersionInfo()
	commitInfo := &CommitInfo{
		SubmissionTime: fmt.Sprintf("%v", time.Now().Unix()),
		SystemVersion:  systemInfo,
		SubmissionType: 1,
		UUID:           "72sb92gf-4f35-48o0-b8v9-096g3a650526",
		Note:           "V20 Backup",
	}
	commitInfoStr, err := json.Marshal(commitInfo)
	if err != nil {
		logger.Warning("failed to marshal commit info")
	}
	err = dbustools.DBusMethodCaller("org.deepin.AtomicUpgrade1", "Commit", "/org/deepin/AtomicUpgrade1", string(commitInfoStr))
	if err != nil {
		return dbusutil.ToError(err)
	}
	for {
		select {
		case reply := <-replyCh:
			if reply.state < 0 {
				goto failed
			}
			if reply.state == 0 {
				err := v.setUpgradeVersion()
				if err != nil {
					logger.Warning(err)
					return dbusutil.ToError(err)
				}
				logger.Warning("receive  commit state 0")
				v.emitProgressValue(100)
				goto success
			}
			v.emitProgressValue(int64(reply.operate) - 100)
		}
	}
failed:
	logger.Warning("failed to backup system")
	return dbusutil.ToError(errors.New("failed to backup system"))

success:
	logger.Debug("success to backup system")
	return nil
}

func (v *VersionManager) StartSystemUpgrade() *dbus.Error {
	return dbusutil.ToError(syscall.Reboot(syscall.LINUX_REBOOT_CMD_RESTART))
}

func (v *VersionManager) RecoverSystem() *dbus.Error {
	v.emitProgressValue(100)
	return nil
}

func (v *VersionManager) SetPlymouthTheme(theme string) *dbus.Error {
	themeCacheFile := filepath.Join("/var/cache/deepin-system-upgrade", "plymouth-default-theme")
	// Storage old plymouth theme.
	out, err := exec.Command("/usr/sbin/plymouth-set-default-theme").CombinedOutput()
	if err != nil {
		logger.Warning("failed to set default plymouth theme")
		return dbusutil.ToError(err)
	}
	defaultTheme := strings.Trim(string(out), "\n")
	f, err := os.OpenFile(themeCacheFile, os.O_TRUNC|os.O_CREATE|os.O_WRONLY, 0755)
	if err != nil {
		return dbusutil.ToError(err)
	}
	_, err = f.WriteString(defaultTheme)
	if err != nil {
		return dbusutil.ToError(err)
	}
	out, err = exec.Command("/usr/sbin/plymouth-set-default-theme", "-R", theme).CombinedOutput()
	if err != nil {
		logger.Warning("failed to set upgrade plymouth theme")
		return dbusutil.ToError(err)
	}
	out, err = exec.Command("/usr/sbin/update-initramfs", "-u", "-k", "all").CombinedOutput()
	if err != nil {
		logger.Warning("failed to update initramfs:", string(out))
		return dbusutil.ToError(err)
	}
	return nil
}

func (v *VersionManager) RestorePlymouthTheme() *dbus.Error {
	themeCacheFile := filepath.Join("/var/cache/deepin-system-upgrade", "plymouth-default-theme")
	if _, err := os.Stat(themeCacheFile); err != nil {
		return dbusutil.ToError(errors.New("cannot find default theme file"))
	}
	// Get default plymouth theme.
	f, err := os.Open(themeCacheFile)
	if err != nil {
		return dbusutil.ToError(err)
	}
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		if scanner.Text() != "" {
			_, err := exec.Command("/usr/sbin/plymouth-set-default-theme", "-R", strings.Trim(scanner.Text(), "\n")).CombinedOutput()
			if err != nil {
				logger.Warning("failed to set default theme")
				return dbusutil.ToError(err)
			}
		}
	}
	err = os.Remove(themeCacheFile)
	if err != nil {
		logger.Warning("failed to remove theme cache file:", err)
	}
	return nil
}

func installGrub() error {
	var target, bootDisk, bootPartition string
	if isEfi() {
		target = "x86_64-efi"
	} else {
		target = "i386-pc"
	}

	bootPartition, err := findBootPartition()
	if err != nil {
		return err
	}

	bootDisk, err = disk.GetDiskFromPartition(bootPartition)
	if err != nil {
		return err
	}
	out, err := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("/usr/sbin/grub-install --target=%s --boot-directory=/boot %s", target, bootDisk)).CombinedOutput()
	if err != nil {
		logger.Warning("failed to exec grub install:", string(out))
		return err
	}
	return nil
}

func findBootPartition() (string, error) {
	if isBootMountSeparate() == nil {
		// The boot partition is mounted separately, and the /boot directory is parsed to mount the partition
		return disk.GetPartitionByPath("/boot")
	}
	return disk.GetPartitionByPath("/")
}

func isBootMountSeparate() error {
	_, err := exec.Command("/usr/bin/findmnt", "/boot").CombinedOutput()
	return err
}

func isEfi() bool {
	if _, err := os.Stat("/sys/firmware/efi"); err == nil {
		return true
	}
	return false
}

func modifyFstabForLinglong(repoPath string) error {
	if repoPath == "/" {
		return nil
	}
	fstabPath := filepath.Join("/etc/fstab")
	fstabEntry := fmt.Sprintf("\n%s /persistent none defaults,bind 0 0\n", filepath.Join(repoPath, "persistent"))
	f, err := os.OpenFile(fstabPath, os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		return err
	}
	defer f.Close()
	_, err = f.WriteString(fstabEntry)
	return err
}

func moveLinglongRepo(root, repoPath string) error {
	_, err := exec.Command("/usr/bin/mv", filepath.Join(root, "persistent"), repoPath).CombinedOutput()
	if err != nil {
		logger.Warningf("failed to mv persistent to %s", repoPath)
		return err
	}
	if repoPath == "/" {
		return nil
	}
	// fix #7324
	if _, err := os.Stat("/persistent"); err == nil {
		return nil
	}
	err = os.Mkdir("/persistent", 0755)
	if err != nil {
		return err
	}
	return nil
}

func enableLinglongApp(root, repoPath string) error {
	if _, err := os.Stat("/persistent/linglong"); err == nil {
		logger.Warning("linglong repo is exist")
		return nil
	}
	err := moveLinglongRepo(root, repoPath)
	if err != nil {
		return err
	}
	return modifyFstabForLinglong(repoPath)
}

func commitNewSystemDir(root, repo string) error {
	err := iso.MountSystemDir(root)
	if err != nil {
		logger.Warning("failed to mount system dir:", err)
	}
	defer iso.UmountSystemDir(root)
	originalRoot, err := os.Open("/")
	if err != nil {
		logger.Warning("failed to open /:", err)
		return err
	}
	defer originalRoot.Close()
	err = syscall.Chroot(root)
	defer func() {
		err := originalRoot.Chdir()
		if err != nil {
			logger.Warning("failed to chdir:", err)
			return
		}
		err = syscall.Chroot(".")
		if err != nil {
			logger.Warning("failed to exit chroot:", err)
			return
		}
	}()
	out, err := exec.Command("/usr/sbin/deepin-upgrade-manager", "--action=setdefaultconfig", "--data="+UpgradeConfig).CombinedOutput()
	if err != nil {
		logger.Warning("failed to set default config:", string(out))
		return err
	}
	systemInfo := getSystemVersionInfo()
	commitInfo := &CommitInfo{
		SubmissionTime: fmt.Sprintf("%v", time.Now().Unix()),
		SystemVersion:  systemInfo,
		SubmissionType: 2,
		UUID:           "79sb92gf-4fg5-48o0-b0v9-096g3b650529",
		Note:           "V23 Commit",
	}
	commitInfoStr, err := json.Marshal(commitInfo)
	if err != nil {
		logger.Warning("failed to marshal commit info")
	}
	out, err = exec.Command("/usr/sbin/deepin-upgrade-manager", "--action=init", "--subject="+string(commitInfoStr)).CombinedOutput()
	if err != nil {
		logger.Warning("failed to commit new system root:", string(out))
	}
	return nil
}

func getSystemVersionInfo() string {
	var systemInfo string
	f, err := os.Open("/etc/os-version")
	if err != nil {
		logger.Warning("failed to open version file:", err)
		return "default"
	}
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if line != "" {
			if strings.HasPrefix(line, "SystemName=") {
				systemName := strings.Split(line, "=")[1]
				logger.Warning(systemName)
				if strings.HasPrefix(systemName, "U") {
					systemName = "UOS"
				} else {
					systemName = "Deepin"
				}
				systemInfo = systemName
			}
			if strings.HasPrefix(line, "MajorVersion") {
				maVersion := strings.Trim(strings.Split(line, "=")[1], "\n")
				systemInfo += "-V" + maVersion
			}
			if strings.HasPrefix(line, "MinorVersion") {
				miVersion := strings.Trim(strings.Split(line, "=")[1], "\n")
				systemInfo += "-" + miVersion
			}
			if strings.HasPrefix(line, "OsBuild") {
				buVersion := strings.Split(strings.Trim(strings.Split(line, "=")[1], "\n"), ".")[1]
				systemInfo += "-" + buVersion
			}
		}
	}
	if systemInfo == "" {
		systemInfo = "default"
	}
	return systemInfo
}

func modifyBackupConfig(homeDir string, configPath string) error {
	input, err := ioutil.ReadFile(configPath)
	if err != nil {
		return dbusutil.ToError(err)
	}

	lines := strings.Split(string(input), "\n")
	for i, line := range lines {
		if strings.Contains(line, "/home/xxx/Desktop") {
			lines[i] = fmt.Sprintf("      - \"%s\"", filepath.Join(homeDir, "Desktop"))
		}
	}
	output := strings.Join(lines, "\n")
	err = ioutil.WriteFile(configPath, []byte(output), 0644)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func monitorStateChange(replyCh chan StateChangeReply) error {
	systemBus, err := dbus.SystemBus()
	err = systemBus.BusObject().AddMatchSignal("org.deepin.AtomicUpgrade1", "StateChanged",
		dbus.WithMatchObjectPath("/org/deepin/AtomicUpgrade1")).Err

	signalCh := make(chan *dbus.Signal, 100)
	systemBus.Signal(signalCh)
	go func() {
		defer func() {
			err = systemBus.BusObject().RemoveMatchSignal("org.deepin.AtomicUpgrade1", "StateChanged",
				dbus.WithMatchObjectPath("/org/deepin/AtomicUpgrade1")).Err
			systemBus.RemoveSignal(signalCh)
		}()
		for {
			select {
			case sig := <-signalCh:
				if sig.Path == "/org/deepin/AtomicUpgrade1" &&
					sig.Name == "org.deepin.AtomicUpgrade1.StateChanged" {
					// Int32 operate, Int32 state, String version,String message
					var reply StateChangeReply
					err = dbus.Store(sig.Body, &reply.operate, &reply.state, &reply.version, &reply.message)
					if err != nil {
						logger.Warning("failed to store dbus signal:", err)
					}
					if reply.state <= 0 {
						replyCh <- reply
						return
					}
					replyCh <- reply
				}
			}
		}
	}()
	return nil
}

func getActiveVersion() (string, error) {
	version, err := dbustools.GetProperty("org.deepin.AtomicUpgrade1", "org.deepin.AtomicUpgrade1", "ActiveVersion", "/org/deepin/AtomicUpgrade1")
	if err != nil {
		logger.Warning("failed to get active version:", err)
		return "", err
	}

	return version.Value().(string), nil
}

func (v *VersionManager) setUpgradeVersion() error {

	err := installGrub()
	if err != nil {
		logger.Warning("failed to install grub:", err)
		return err
	}
	err = dbustools.DBusMethodCaller("org.deepin.AtomicUpgrade1", "Rollback", "/org/deepin/AtomicUpgrade1", v.activeVersion)
	if err != nil {
		return err
	}

	v.emitProgressValue(90)
	replyCh := make(chan StateChangeReply, 20)
	err = monitorStateChange(replyCh)
	if err != nil {
		return err
	}
	for {
		select {
		case reply := <-replyCh:
			if reply.state < 0 {
				goto error
			}
			if reply.state == 0 {
				logger.Debug("success to set upgrade version")
				goto success
			}
		}
	}
error:
	return errors.New("failed to set upgrade version")

success:
	return nil
}

func (v *VersionManager) emitProgressValue(value int64) error {
	return v.service.Emit(v, "ProgressValue", value)
}
