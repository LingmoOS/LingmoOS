// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package iso

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/utils"

	"deepin-system-upgrade-daemon/pkg/module/atomic"
	"deepin-system-upgrade-daemon/pkg/module/config"
	"deepin-system-upgrade-daemon/pkg/module/dbustools"
	"deepin-system-upgrade-daemon/pkg/module/disk"
)

// DBus path and DBus interface
const (
	dbusPath      = "/org/deepin/SystemUpgrade1/ISOManager"
	dbusInterface = "org.deepin.SystemUpgrade1.ISOManager"
)

// Dependent configuration file path
const (
	AtomicRepoCfgLocation   = "/persistent/osroot/config/config.json"
	AtomicConfigLocation    = "/etc/deepin-upgrade-manager/config.json"
	UpgradeConfigPath       = "/etc/deepin-system-upgrade"
	UpgradeToolBinaryPath   = "/usr/bin/deepin-system-upgrade-tool"
	UpgradeToolIconPath     = "/usr/share/icons/hicolor/scalable/apps/upgrade-tool.svg"
	PlymouthThemePath       = "/usr/share/plymouth/themes/deepin-upgrade"
	SystemdDBusConf         = "/usr/share/dbus-1/system.d/org.deepin.SystemUpgrade1.conf"
	SystemdDBusService      = "/usr/share/dbus-1/system-services/org.deepin.SystemUpgrade1.service"
	UpgradeDaemonBinaryPath = "/usr/sbin/deepin-system-upgrade-daemon"
	TranslationDirPath      = "/usr/share/deepin-system-upgrade-tool/translations"
	UpgradeDesktopFilePath  = "/usr/share/applications/deepin-system-upgrade-tool.desktop"
	UpgradeAutoStartFile    = "/etc/xdg/autostart/deepin-system-upgrade-tool.desktop"
	MigrateFlagsPath        = "/var/cache/deepin-system-upgrade/migrate.state"

	SYSTEM_VERSION_FILE = "/etc/os-version"

	SquashfsRootPath = ".cache/extract/live/squashfs-root/"
)

var logger = log.NewLogger("deepin-system-upgrade/iso")

type ISOManager struct {
	service *dbusutil.Service

	methods *struct {
		CheckISO      func() `in:"isoPath"`
		GetISOVersion func() `out:"version"`
	}

	signals *struct {
		ISOProgressValue struct {
			value int
		}
		CheckResult struct {
			passed bool
		}
	}
}

func (i *ISOManager) CheckISO(sender dbus.Sender, isoPath string) *dbus.Error {
	uid, err := i.service.GetConnUID(string(sender))
	if err != nil {
		return dbusutil.ToError(err)
	}
	user, err := user.LookupId(fmt.Sprint(uid))
	logger.Warning("Extracting iso:", isoPath)
	root, err := ExtractIsoFile(user.HomeDir, isoPath)
	if err != nil {
		logger.Warning("failed to extract iso file:", err)
		return dbusutil.ToError(err)
	}

	logger.Warning("Extraction done")
	var editionName string
	file, err := os.Open(SYSTEM_VERSION_FILE)
	if err != nil {
		logger.Warning("failed to open system version file:", err)
		return dbusutil.ToError(err)
	} else {
		defer file.Close()
		br := bufio.NewReader(file)
		for {
			s, _, err := br.ReadLine()
			if err == io.EOF {
				break
			}
			if strings.Contains(string(s), "EditionName=") {
				editionName = strings.Split(string(s), "=")[1]
			}
		}
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	}

	var isoEditionName, isoMajorVersion string
	isoVerFile, err := os.Open(filepath.Join(root, "etc/os-version"))
	if err != nil {
		logger.Warning("failed to open iso version file:", err)
	} else {
		defer isoVerFile.Close()
		br := bufio.NewReader(isoVerFile)
		for {
			s, _, err := br.ReadLine()
			if err == io.EOF {
				break
			}
			if strings.Contains(string(s), "EditionName=") {
				isoEditionName = strings.Split(string(s), "=")[1]
			}
			if strings.Contains(string(s), "MajorVersion") {
				isoMajorVersion = strings.Split(string(s), "=")[1]
			}
		}
		if err != nil {
			logger.Warning(err)
			return dbusutil.ToError(err)
		}
	}

	logger.Warning("editionName:", editionName)
	logger.Warning("isoEditionName:", isoEditionName)
	logger.Warning("iso Version:", isoMajorVersion)

	if editionName != isoEditionName || isoMajorVersion < "23" {
		err = i.service.Emit(i, "CheckResult", false)
		if err != nil {
			logger.Warning(err)
		}
		return dbusutil.ToError(err)
	}

	extractPath := filepath.Join(user.HomeDir, ".cache/extract")
	err = os.Chdir(extractPath)
	if err != nil {
		logger.Warning(err)
	}
	_, err = exec.Command("/usr/bin/sha256sum", "-c", "sha256sum.txt").CombinedOutput()
	if err != nil {
		logger.Warning(err)
	}
	err = i.service.Emit(i, "CheckResult", err == nil)
	if err != nil {
		logger.Warning(err)
	}

	return nil
}

func (i *ISOManager) GetISOVersion(sender dbus.Sender) (string, *dbus.Error) {
	// Must be called after CheckISO, or the result will be incorrect.

	uid, err := i.service.GetConnUID(string(sender))
	if err != nil {
		return "N/A", dbusutil.ToError(err)
	}
	user, err := user.LookupId(fmt.Sprint(uid))
	var root = filepath.Join(user.HomeDir, SquashfsRootPath)
	var isoMajorVersion string
	isoVerFile, err := os.Open(filepath.Join(root, "etc/os-version"))
	if err != nil {
		logger.Warning("failed to open iso version file:", err)
	} else {
		defer isoVerFile.Close()
		br := bufio.NewReader(isoVerFile)
		for {
			s, _, err := br.ReadLine()
			if err == io.EOF {
				break
			}
			if strings.Contains(string(s), "MajorVersion") {
				isoMajorVersion = strings.Split(string(s), "=")[1]
			}
		}
		if err != nil {
			logger.Warning(err)
			return "N/A", dbusutil.ToError(err)
		}
	}
	return isoMajorVersion, nil
}

func (*ISOManager) GetInterfaceName() string {
	return dbusInterface
}

func newManager(service *dbusutil.Service) *ISOManager {
	m := &ISOManager{
		service: service,
	}
	return m
}

func isEfi() bool {
	if _, err := os.Stat("/sys/firmware/efi"); err == nil {
		return true
	}
	return false
}

func setMigratePkgsFlags() error {
	f, err := os.OpenFile(MigrateFlagsPath, os.O_CREATE, 0755)
	if err != nil {
		return err
	}
	defer f.Close()
	return nil
}

func setPlymouthTheme() error {
	path := "/var/cache/deepin-system-upgrade"
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
	out, err := exec.Command("/usr/sbin/plymouth-set-default-theme", "-R", "deepin-upgrade").CombinedOutput()
	if err != nil {
		logger.Warning("failed to set upgrade plymouth theme:", string(out))
		return err
	}

	out, err = exec.Command("/usr/sbin/update-initramfs", "-u", "-k", "all").CombinedOutput()
	if err != nil {
		logger.Warning("failed to update initramfs:", string(out))
		return dbusutil.ToError(err)
	}
	return nil
}

func mergeDir(dir, root string) error {
	originalRoot, err := os.Open("/")
	if err != nil {
		logger.Warning("failed to open /:", err)
		return err
	}
	defer originalRoot.Close()
	err = chrootIsoDir(root)
	if err != nil {
		logger.Warning(err)
		return err
	}
	defer exitChroot(originalRoot)
	// copy files from dir to dir.tmp
	err = iterateForCopyFile(dir)
	if err != nil {
		return err
	}
	if _, err := os.Stat(dir); err == nil {
		err = os.RemoveAll(dir)
		if err != nil {
			logger.Warning(err)
			return err
		}
	}
	err = os.Rename(dir+".tmp", dir)
	if err != nil {
		logger.Warning(err)
		return err
	}
	return nil
}

func isFileExist(tmpPath string) error {
	if _, err := os.Stat(tmpPath); err != nil {
		if _, err := os.Lstat(tmpPath); err != nil {
			logger.Warning(err)
			return err
		}
	}
	return nil
}

func iterateForCopyFile(dir string) error {
	filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			logger.Warning(err)
			return err
		}
		tmpPath := strings.Replace(path, dir+"/", dir+".tmp/", -1)
		// copy v23 files that is not exist in dir.tmp(v20 files - filter files).
		if _, err := os.Stat(tmpPath); err != nil {
			_, err := exec.Command("/usr/bin/cp", "-rpf", path, tmpPath).CombinedOutput()
			if err != nil {
				logger.Warning(err)
			}
		}
		return nil
	})
	return nil
}

// Copy the etc directory in V20 system to the V23 system and rename it to the etc.tmp directory
// Traverse the files in the whitelist and delete the files from the etc.tmp directory in V23 system
// Traverse all files under V23 system, if the file exists in V20 system, continue to traverse; if the file does not exist in V20 system, copy it to the V20 system directory
func prepareRootDir(root string) error {
	targets := config.GetTargetList()
	for _, target := range targets {
		tmp := filepath.Join(root, target.MergeDir+".tmp")
		_, err := exec.Command("/usr/bin/cp", "-r", target.MergeDir, tmp).CombinedOutput()
		if err != nil {
			return err
		}
		for _, file := range target.Filter {
			// remove filter files that exists in v20
			absPath := filepath.Join(root, file)
			tmpPath := strings.Replace(absPath, "/etc/", "/etc.tmp/", -1)
			if _, err := os.Stat(tmpPath); err == nil {
				logger.Warning("remove filter file:", tmpPath)
				err := os.RemoveAll(tmpPath)
				if err != nil {
					logger.Warning(err)
				}
			}
			if _, err := os.Lstat(tmpPath); err == nil {
				logger.Warning("remove filter link:", tmpPath)
				err := os.RemoveAll(tmpPath)
				if err != nil {
					logger.Warning(err)
				}
			}
		}
		err = mergeDir(target.MergeDir, root)
		if err != nil {
			logger.Warning(err)
			return err
		}

	}
	return nil
}

func migrateUserInfoForShadow(userNameList []string, root string) error {
	origShaPath := "/etc/shadow"
	newShaPath := filepath.Join(root, "etc/shadow")
	of, err := os.Open(origShaPath)
	if err != nil {
		return err
	}
	defer of.Close()
	nf, err := os.OpenFile(newShaPath, os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		return err
	}
	defer nf.Close()
	scanner := bufio.NewScanner(of)
	for _, userName := range userNameList {
		for scanner.Scan() {
			line := scanner.Text()
			if strings.Split(line, ":")[0] == userName {
				nf.WriteString(line + "\n")
				break
			}
		}
	}
	return nil
}

func getCurrentGroups(userName string) ([]string, error) {
	var groups []string
	out, err := exec.Command("/usr/bin/groups", userName).CombinedOutput()
	if err != nil {
		return nil, errors.New("failed to get groups")
	}
	groups = strings.Split(strings.Split(strings.Trim(string(out), "\n"), ":")[1], " ")[2:]
	return groups, nil
}

func appendUserToGroupFile(userNameList []string, root string) error {
	if userNameList == nil {
		return errors.New("user list is empty")
	}
	groupFile := filepath.Join(root, "etc/group")
	for _, userName := range userNameList {
		groups, err := getCurrentGroups(userName)
		if err != nil {
			return err
		}
		input, err := ioutil.ReadFile(groupFile)
		if err != nil {
			return err
		}

		lines := strings.Split(string(input), "\n")
		for _, groupName := range groups {
			for i, line := range lines {
				items := strings.Split(line, ":")
				if items[0] == groupName {
					if items[len(items)-1] == "" {
						lines[i] = line + userName
					} else {
						lines[i] = line + "," + userName
					}
				}
			}
		}
		output := strings.Join(lines, "\n")
		err = ioutil.WriteFile(groupFile, []byte(output), 0644)
		if err != nil {
			return err
		}
	}
	return nil
}

func writeUserInfoToFile(path string, userList []string, root string) error {
	if userList == nil {
		return errors.New("user list is empty")
	}
	newPath := filepath.Join(root, path)
	f, err := os.OpenFile(newPath, os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		return err
	}
	defer f.Close()
	for _, user := range userList {
		f.WriteString(user + "\n")
	}
	return nil
}

func parseGrpUserFromConfig(path string, userNames []string) ([]string, error) {
	var groupList []string
	fi, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer fi.Close()
	scanner := bufio.NewScanner(fi)
	for scanner.Scan() {
		line := scanner.Text()
		for _, user := range userNames {
			if user == strings.Split(line, ":")[0] {
				groupList = append(groupList, line)
			}
		}
	}
	return groupList, nil
}

func parsePwdUserFromConfig(path string) ([]string, error) {
	var userList []string
	fi, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer fi.Close()
	scanner := bufio.NewScanner(fi)
	for scanner.Scan() {
		line := scanner.Text()
		uid, err := strconv.Atoi(strings.Split(line, ":")[2])
		if err != nil {
			return nil, err
		}
		if uid < 1000 || uid >= 65534 {
			continue
		}
		userList = append(userList, line)
	}
	return userList, nil
}

// Modify the configuration file in the etc directory
// 1.Retrieve users with uid>= 1000 in the original passwd file and copy them to the passwd file of the new system
// 2.Add the user to the new group file and add it after the corresponding group
// 3.Copy the password information of the original shadow file and add it to the new shadow file
func migrateUserInfoToNewSystem(root string) error {
	var psw = "/etc/passwd"
	var userNames []string
	userLists, err := parsePwdUserFromConfig(psw)
	if err != nil {
		return err
	}
	err = writeUserInfoToFile(psw, userLists, root)
	if err != nil {
		return err
	}
	for _, user := range userLists {
		userNames = append(userNames, strings.Split(user, ":")[0])
	}
	var group = "/etc/group"
	groupLists, err := parseGrpUserFromConfig(group, userNames)
	if err != nil {
		return err
	}

	err = writeUserInfoToFile(group, groupLists, root)
	if err != nil {
		return err
	}
	err = appendUserToGroupFile(userNames, root)
	if err != nil {
		return err
	}
	err = migrateUserInfoForShadow(userNames, root)
	if err != nil {
		return err
	}
	return nil
}

func updateGrub() error {
	cmd := exec.Command("/usr/sbin/update-grub")
	if err := cmd.Start(); err != nil {
		return err
	}
	if err := cmd.Wait(); err != nil {
		return err
	}
	out, err := exec.Command("/usr/bin/sh", "-c", "sed -i 's#/boot.v23##g' /boot/grub/grub.cfg").CombinedOutput()
	if err != nil {
		logger.Warning("failed to modify grub config:", string(out))
		return errors.New("failed to modify grub config")
	}
	return nil
}

// func installGrub(root string) (err error) {
// 	curGrub := "/boot/grub"
// 	bakGrub := curGrub + ".bak"
// 	newGrub := filepath.Join(root, "boot/grub")
// 	tmpGrub := curGrub + ".tmp"
// 	curEfi := "/boot/efi"

// 	if _, err := os.Stat(bakGrub); err == nil {
// 		os.Rename(bakGrub, bakGrub+".old")
// 	}
// 	out, err := exec.Command("/usr/bin/mv", newGrub, tmpGrub).CombinedOutput()
// 	if err != nil {
// 		logger.Warningf("failed to mv %s to %s", newGrub, tmpGrub)
// 		return err
// 	}
// 	out, err = exec.Command("/usr/bin/mv", curGrub, bakGrub).CombinedOutput()
// 	if err != nil {
// 		logger.Warningf("failed to mv %s to %s", curGrub, bakGrub)
// 		return err
// 	}
// 	out, err = exec.Command("/usr/bin/mv", tmpGrub, curGrub).CombinedOutput()
// 	if err != nil {
// 		logger.Warningf("failed to mv %s to %s", tmpGrub, curGrub)
// 		return err
// 	}
// 	if _, err := os.Stat(curEfi); err == nil {
// 		out, err = exec.Command("/usr/bin/cp", "-r", curEfi, filepath.Join(root, "boot")).CombinedOutput()
// 		if err != nil {
// 			logger.Warningf("failed to cp %s to %s", curEfi, filepath.Join(root, "boot"))
// 			return err
// 		}
// 	}

// 	// Judgment guide method
// 	var target, bootDisk, bootPartition string
// 	if isEfi() {
// 		target = "x86_64-efi"
// 	} else {
// 		target = "i386-pc"
// 	}

// 	defer func(err *error) {
// 		out, errs := exec.Command("/usr/bin/mv", curGrub, newGrub).CombinedOutput()
// 		if err == nil && errs != nil {
// 			logger.Warningf("failed to mv %s to %s", curGrub, newGrub)
// 			*err = errors.New(string(out))
// 		}
// 		out, errs = exec.Command("/usr/bin/mv", bakGrub, curGrub).CombinedOutput()
// 		if err == nil && errs != nil {
// 			logger.Warningf("failed to mv %s to %s", bakGrub, curGrub)
// 			*err = errors.New(string(out))
// 		}
// 	}(&err)
// 	bootPartition, err = findBootPartition()
// 	if err != nil {
// 		return err
// 	}
// 	bootDisk, err = disk.GetDiskFromPartition(bootPartition)
// 	if err != nil {
// 		return err
// 	}
// 	out, err = exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("/usr/sbin/grub-install --target=%s --boot-directory=/boot %s", target, bootDisk)).CombinedOutput()
// 	if err != nil {
// 		logger.Warning("failed to exec grub install:", string(out))
// 		return err
// 	}
// 	return nil
// }

// func restoreBootDir(path string, mountInfoMap map[string]string) error {
// 	bootV23 := filepath.Join("/mnt", "boot.v23")
// 	err := os.Mkdir("/boot.new", 0700)
// 	if err != nil {
// 		return err
// 	}

// 	out, err := exec.Command("/usr/bin/sh", "-c", "mv /boot/* /boot.new").CombinedOutput()
// 	if err != nil {
// 		logger.Warning("mv boot file failed:", string(out))
// 		return err
// 	}

// 	err = os.Remove(bootV23)
// 	if err != nil {
// 		logger.Warning("failed to remove boot.v23:", err)
// 	}

// 	for _, mp := range mountInfoMap {
// 		out, err := exec.Command("/usr/bin/umount", mp).CombinedOutput()
// 		if err != nil {
// 			logger.Warning("failed to umount dir:", string(out))
// 			return err
// 		}
// 	}

// 	out, err = exec.Command("/usr/bin/umount", "/mnt").CombinedOutput()
// 	if err != nil {
// 		logger.Warning("failed to umount /mnt:", string(out))
// 		return err
// 	}
// 	if _, err := os.Stat("/boot.old"); err == nil {
// 		os.Remove("/boot.old")
// 	}
// 	err = os.Rename("/boot", "/boot.old")
// 	if err != nil {
// 		logger.Warning("failed to rename /boot to /boot.old:", string(out))
// 		return err
// 	}
// 	err = os.Rename("/boot.new", "/boot")
// 	if err != nil {
// 		logger.Warning("failed to rename /boot.new to /boot:", string(out))
// 		return err
// 	}
// 	return nil
// }

// func replaceBootDir(path string) error {
// 	dev, err := disk.GetPartitionByPath("/boot")
// 	if err != nil {
// 		return err
// 	}
// 	mnt := filepath.Join(path, "mnt")
// 	boot := filepath.Join(path, "boot/*")
// 	bootV23 := filepath.Join(mnt, "boot.v23")
// 	// Mount the boot partition to the new mirror mnt directory
// 	out, err := exec.Command("/usr/bin/mount", dev, mnt).CombinedOutput()
// 	if err != nil {
// 		logger.Warning("failed to mount boot to mnt:", string(out))
// 		return err
// 	}

// 	_, err = os.Stat(bootV23)
// 	if err == nil {
// 		err = os.Remove(bootV23)
// 		return err
// 	}
// 	err = os.Mkdir(bootV23, 0700)
// 	if err != nil {
// 		return err
// 	}

// 	// Move the new image boot partition file to the boot partition
// 	out, err = exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("mv %s %s", boot, bootV23)).CombinedOutput()
// 	if err != nil {
// 		logger.Warning("failed to mv boot file:", string(out))
// 		return err
// 	}
// 	return nil
// }

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

func mountInChroot(mountInfoMap map[string]string) error {
	for dev, mp := range mountInfoMap {
		if !strings.Contains(dev, "/dev") {
			out, err := exec.Command("/usr/bin/mount", "-o", "bind", dev, mp).CombinedOutput()
			if err != nil {
				logger.Warningf("failed to mount bind %s to %s because of %s", dev, mp, string(out))
				return err
			}
			continue
		}
		out, err := exec.Command("/usr/bin/mount", dev, mp).CombinedOutput()
		if err != nil {
			logger.Warningf("failed to mount %s to %s because of %s", dev, mp, string(out))
			return err
		}
	}
	return nil
}

func MountSystemDir(path string) error {
	out, err := exec.Command("/usr/bin/mount", "-t", "proc", "/proc", filepath.Join(path, "proc")).CombinedOutput()
	if err != nil {
		logger.Warning("failed to mount proc:", string(out))
		return err
	}
	out, err = exec.Command("/usr/bin/mount", "-t", "sysfs", "/sys", filepath.Join(path, "sys")).CombinedOutput()
	if err != nil {
		logger.Warning("failed to mount sys:", string(out))
		return err
	}
	out, err = exec.Command("/usr/bin/mount", "-o", "bind", "/dev", filepath.Join(path, "dev")).CombinedOutput()
	if err != nil {
		logger.Warning("failed to mount dev:", string(out))
		return err
	}
	// TODO:获取原子更新属性，得到repo目录，然后进行bind
	return nil
}

func UmountSystemDir(root string) {
	var sysMountDirs = []string{"/run", "/", "/proc", "/sys", "/dev"}
	for _, dir := range sysMountDirs {
		if dir == "/" && isBootMountSeparate() == nil {
			continue
		}
		path := filepath.Join(root, dir)
		exec.Command("/usr/bin/umount", "-f", path).CombinedOutput()
	}
	// TODO: 卸载原子更新repo挂载目录
}

func exitChroot(originalRoot *os.File) error {
	err := originalRoot.Chdir()
	if err != nil {
		logger.Warning("failed to chdir:", err)
		return err
	}
	err = syscall.Chroot(".")
	if err != nil {
		logger.Warning("failed to exit chroot:", err)
		return err
	}
	return nil
}

func chrootIsoDir(path string) error {
	return syscall.Chroot(path)
}

func unsquashfsFile(path string) (string, error) {
	squashfsRoot := filepath.Join(filepath.Dir(path), "squashfs-root")
	out, err := exec.Command("/usr/bin/unsquashfs", "-d", squashfsRoot, path).CombinedOutput()
	if err != nil {
		logger.Warning("failed to unsquashfs iso:", string(out))
		return "", err
	}
	return squashfsRoot, nil
}

// Todo(Yutao Meng): Clear existing directories before decompression
func ExtractIsoFile(homeDir, isoPath string) (string, error) {
	extractPath := filepath.Join(homeDir, ".cache/extract")
	// 清理残留解压
	if _, err := os.Stat(extractPath); err == nil {
		var retryTimes int
		// If /run is mounted in squashfs-root, umount it first, if umount fails, the file directory will not be deleted to ensure system security
		for {
			if retryTimes > 20 {
				return "", errors.New("failed to umount for /run")
			}
			retryTimes++
			time.Sleep(time.Duration(time.Millisecond * 500))
			out, err := exec.Command("/usr/bin/sh", "-c", "mount |grep squashfs-root/run").CombinedOutput()
			if err != nil {
				break
			}
			out, err = exec.Command("/usr/bin/umount", filepath.Join(extractPath, "live/squashfs-root/run")).CombinedOutput()
			if err != nil {
				logger.Warning("failed to umount for /run:", string(out))
			}
		}
		os.RemoveAll(extractPath)
	}
	out, err := exec.Command("/usr/bin/7z", "x", isoPath, "-o"+extractPath).CombinedOutput()
	if err != nil {
		logger.Warning("failed to extract iso:", string(out))
		return "", err
	}
	_manager.emitProgressValue(10)
	return unsquashfsFile(filepath.Join(extractPath, "live/filesystem.squashfs"))
}

func installPackage(pkgName string) error {
	cmd := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("DEBIAN_FRONTEND='noninteractive' apt-get -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold' --no-install-recommends --allow-unauthenticated install %s", pkgName))
	stdout, err := cmd.StdoutPipe()
	cmd.Stderr = cmd.Stdout
	if err != nil {
		return err
	}
	if err = cmd.Start(); err != nil {
		return err
	}
	for {
		tmp := make([]byte, 1024)
		_, err := stdout.Read(tmp)
		logger.Debug(string(tmp))
		if err != nil || tmp == nil {
			break
		}
	}
	if err = cmd.Wait(); err != nil {
		return err
	}
	return nil
}

func installPackages(pkgList []string, isEfi bool) error {
	var filterPkg string
	if isEfi {
		filterPkg = "grub-pc"
	} else {
		filterPkg = "grub-efi"
	}
	for _, pkg := range pkgList {
		if strings.Contains(pkg, filterPkg) {
			continue
		}
		logger.Warning("begin to install package:", pkg)
		err := installPackage(pkg)
		if err != nil {
			return err
		}
	}
	return nil
}

func copyFileToNewSystem(root string) error {
	err := utils.CopyDir(UpgradeConfigPath, filepath.Join(root, UpgradeConfigPath))
	if err != nil {
		return err
	}
	// err = os.RemoveAll(filepath.Join(root, "boot"))
	// if err != nil {
	// 	return err
	// }
	err = utils.CopyDir("/boot", filepath.Join(root, "boot"))
	if err != nil {
		return err
	}

	err = utils.CopyDir(PlymouthThemePath, filepath.Join(root, PlymouthThemePath))
	if err != nil {
		return err
	}

	// err = os.RemoveAll(filepath.Join(root, "usr/lib/modules"))
	// if err != nil {
	// 	logger.Warning("failed to remove modules:", err)
	// 	return err
	// }
	out, err := exec.Command("/usr/bin/cp", "-rf", "/usr/lib/modules", filepath.Join(root, "usr/lib/modules")).CombinedOutput()
	if err != nil {
		logger.Warning("failed to copy modules:", string(out))
		return err
	}

	out, err = exec.Command("/usr/bin/cp", "-rf", "/etc/default/locale", filepath.Join(root, "etc/default/locale")).CombinedOutput()
	if err != nil {
		logger.Warning("failed to copy file:", string(out))
		return err
	}

	err = utils.CopyFile(UpgradeDesktopFilePath, filepath.Join(root, UpgradeAutoStartFile))
	if err != nil {
		return err
	}
	return nil
}

func installPkgInNewSystem(extractPath, root string) error {
	originalRoot, err := os.Open("/")
	if err != nil {
		logger.Warning("failed to open /:", err)
		return err
	}
	defer originalRoot.Close()
	var isEfiBoot bool
	if isEfi() {
		isEfiBoot = true
	}
	err = chrootIsoDir(root)
	if err != nil {
		logger.Warning("failed to chroot iso dir:", err)
		return err
	}
	defer exitChroot(originalRoot)
	out, err := exec.Command("/usr/bin/apt-get", "update").CombinedOutput()
	if err != nil {
		logger.Warning(string(out))
	}
	pkgList := config.GetPackageList()
	err = installPackages(pkgList, isEfiBoot)
	if err != nil {
		return err
	}
	return nil
}

func removeDeepinInstaller() error {
	out, err := exec.Command("/usr/bin/apt", "-y", "purge", "deepin-installer").CombinedOutput()
	if err != nil {
		logger.Warning("failed to purge deepin-installer:", string(out))
		return err
	}
	return nil
}

func PrepareNewRootForCommit(homeDir, isoPath string) (string, string, error) {
	// Obtain the root directory after decompression of the new image
	extractPath := filepath.Join(homeDir, ".cache")
	root := filepath.Join(extractPath, "extract/live/squashfs-root")
	_manager.emitProgressValue(20)
	// 构建本地仓库
	err := installPkgInNewSystem(extractPath, root)
	if err != nil {
		logger.Warning("failed to install package:", err)
		return "", "", errors.New("failed to install package in new system")
	}
	// err = installGrub(root)
	// if err != nil {
	// 	logger.Warning("failed to install grub:", err)
	// 	return "", "", errors.New("failed to install grub")
	// }

	err = copyFileToNewSystem(root)
	if err != nil {
		return "", "", err
	}

	err = migrateUserInfoToNewSystem(root)
	if err != nil {
		logger.Warning("failed to migrate user info:", err)
		return "", "", err
	}

	err = prepareRootDir(root)
	if err != nil {
		return "", "", err
	}

	err = MountSystemDir(root)
	if err != nil {
		logger.Warning("mount failed:", err)
	}
	defer UmountSystemDir(root)
	repo, err := atomic.GetOStreeRepoInfo("TARGET")
	if err != nil {
		logger.Warning("failed to get ostree repo location")
		return "", "", errors.New("failed to get ostree repo location")
	}
	// Chroot to new system root dir
	originalRoot, err := os.Open("/")
	if err != nil {
		logger.Warning("failed to open /:", err)
		return "", "", err
	}
	defer originalRoot.Close()
	err = chrootIsoDir(root)
	if err != nil {
		logger.Warning("failed to chroot iso dir:", err)
		return "", "", err
	}
	defer exitChroot(originalRoot)
	_manager.emitProgressValue(30)
	// remove deepin-installer
	err = removeDeepinInstaller()
	if err != nil {
		return "", "", err
	}
	err = setPlymouthTheme()
	if err != nil {
		logger.Warning("failed to set plymouth theme:", err)
		return "", "", err
	}
	err = setMigratePkgsFlags()
	if err != nil {
		return "", "", err
	}
	_manager.emitProgressValue(40)
	return root, repo, nil
}

func HandleAfterCommit(root, repo string) error {
	defer func() {
		err := updateGrub()
		if err != nil {
			logger.Warning(err)
		}
	}()
	out, err := exec.Command("/usr/bin/sh", "-c", "pgrep -f -l deepin-upgrade-manager | awk -F ' ' '{print $1}'").Output()
	if err == nil {
		if len(out) != 0 {
			pids := strings.Split(string(out), "\n")
			for _, pid := range pids {
				if pid == "" {
					continue
				}
				cmdArgs := fmt.Sprintf("/usr/bin/tail --pid=%s -f /dev/null", pid)
				logger.Debug("print cmd arg:", cmdArgs)
				out, err = exec.Command("/usr/bin/sh", "-c", cmdArgs).Output() // #nosec G204
				if err != nil {
					logger.Warning("failed to monitor atomic upgrade process:", string(out))
				}
			}
		}
	} else {
		logger.Warning("failed to pgrep atomic upgrade process:", string(out))
	}
	out, err = exec.Command("/usr/bin/umount", filepath.Join(root, "dev")).CombinedOutput()
	if err != nil {
		logger.Warning("umount dev:", string(out))
	}
	osroot := filepath.Join(root, "persistent/osroot")
	out, err = exec.Command("/usr/bin/mv", osroot, repo).CombinedOutput()
	if err != nil {
		logger.Warningf("failed to mv %s to %s because of %s", osroot, repo, string(out))
		return err
	}
	return dbustools.DBusMethodCaller("org.deepin.AtomicUpgrade1", "SetRepoMount", "/org/deepin/AtomicUpgrade1", repo)
}

func (i *ISOManager) emitProgressValue(value int64) error {
	return i.service.Emit(i, "ISOProgressValue", value)
}
