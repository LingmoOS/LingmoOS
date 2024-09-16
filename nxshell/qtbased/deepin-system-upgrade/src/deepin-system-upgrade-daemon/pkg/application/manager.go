// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//The main function is to include software evaluation and software migration
package application

import (
	"bufio"
	"context"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"reflect"
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
	"deepin-system-upgrade-daemon/pkg/module/user"
)

var logger = log.NewLogger("deepin-system-upgrade/application")

// DBus path and DBus interface
const (
	dbusPath      = "/org/deepin/SystemUpgrade1/AppManager"
	dbusInterface = "org.deepin.SystemUpgrade1.AppManager"
)

// Dependent file paths
const (
	StatusFilePath           = "/var/lib/dpkg/status"
	BackupConfig             = "/etc/deepin-system-upgrade/backup.yaml"
	RepackDebStorageDir      = "/tmp/deepin-system-upgrade"
	AtomicUpgradeVersionData = "/var/lib/deepin-boot-kit/version.data"
	UpgradeAutoStartFile     = "/etc/xdg/autostart/deepin-system-upgrade-tool.desktop"
	UpgradeDesktopFile       = "/usr/share/applications/deepin-system-upgrade-tool.desktop"
	MigrateFlagsPath         = "/var/cache/deepin-system-upgrade/migrate.state"
	MigrateListPath          = "/var/cache/deepin-system-upgrade/migrate.list"
	SourceSettingsPath       = "extract/oem/settings.ini"
	SouceListPath            = "/etc/apt/sources.list"
	DesktopFilesLocation     = "/usr/share/applications"
	SquashfsRootPath         = ".cache/extract/live/squashfs-root/"
)

// Package installation state value when migration is applied
const (
	PkgInstallFailed = iota
	PkgInstallSuccess
)

// Network state
const (
	NetworkConnected = 70
)

// The state value when applying the migration
const (
	MigrateSuccessful = iota // Migrate successfuk
	NetworkError             // Network error
	Error                    // Other
)

// Instruction code for package install
const (
	PkgNotAllowInstall = iota // Not be allowed to install
	PkgAllowInstall           // Allow to install
	PkgBeenReplaced           // Not need to install
)

type MigrateStatus struct {
	Code        int
	Description string
}

const (
	_KEY_PACKAGE      = "Package"
	_KEY_ARCHITECTURE = "Architecture"
	_KEY_CONFLICTS    = "Conflicts"
	_KEY_REPLACES     = "Replaces"
	_KEY_BREAKS       = "Breaks"
)

const (
	_STATUS_DELIM = ": "
)

const (
	ARCH_I386   = ":i386"
	ARCH_X86_64 = ":amd64"
	ARCH_ALL    = ":all"
)

type PackageStatus struct {
	Package      string
	Architecture string
	Conflicts    string
	Replaces     string
	Breaks       string

	Contents []string
}

type PackageStatusList []*PackageStatus

type AppManager struct {
	service      *dbusutil.Service
	originalRoot *os.File

	packageFilter        map[string][]string
	integratedPkgs       map[string]string
	repalcedPkgs         map[string][]string
	origSysCommitVersion string
	newSystemRoot        string

	methods *struct {
		Assess             func() `in:"isoPath"`
		CancelBackupApp    func() `in:"cancel"`
		MigratePackages    func() `out:"status"`
		SetMigrateAppsList func() `in:"apps"`
	}

	signals *struct {
		AppsAvailable struct {
			apps map[string][]string
		}

		MigrateStatus struct {
			pkg    string
			status int
		}

		ProgressValue struct {
			value int
		}
	}
}

func (*AppManager) GetInterfaceName() string {
	return dbusInterface
}

func newManager(service *dbusutil.Service) *AppManager {
	m := &AppManager{
		service:              service,
		packageFilter:        make(map[string][]string),
		integratedPkgs:       make(map[string]string),
		repalcedPkgs:         make(map[string][]string),
		origSysCommitVersion: getOrigSysCommitVersion(),
	}
	return m
}

func (a *AppManager) Assess(sender dbus.Sender, isoPath string) *dbus.Error {
	logger.Debug("Begin to assess apps in", isoPath)

	homeDir, err := user.GetHomeDirBySender(sender)
	if err != nil {
		return dbusutil.ToError(err)
	}

	var root = filepath.Join(homeDir, SquashfsRootPath)

	var (
		appInfoMap map[string][]string
		appList    []string
	)

	appInfoMap = getGuiPkgNames()
	logger.Debug("Finished to scan desktop files, print info together:", appInfoMap)
	err = a.service.Emit(a, "AppsAvailable", appInfoMap)
	if err != nil {
		logger.Warning(err)
	}
	for app := range appInfoMap {
		appList = append(appList, app)
	}
	return a.checkAppsCompatibility(appList, filepath.Join(homeDir, ".cache"), root)
}

func (a *AppManager) SetMigrateAppsList(apps []string) *dbus.Error {
	_, err := os.Stat(filepath.Dir(MigrateListPath))
	if err != nil {
		os.MkdirAll(filepath.Dir(MigrateListPath), 0755)
	}
	f, err := os.Create(MigrateListPath)
	if err != nil {
		return dbusutil.ToError(err)
	}
	defer f.Close()
	for _, app := range apps {
		f.WriteString(app + "\n")
	}
	return nil
}

func getMigrateList() ([]string, error) {
	var migrateList []string
	f, err := os.OpenFile(MigrateListPath, os.O_RDONLY, 0755)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	br := bufio.NewReader(f)
	for {
		s, _, err := br.ReadLine()
		if err == io.EOF {
			break
		}
		migrateList = append(migrateList, string(s))
	}
	logger.Warning(migrateList)
	return migrateList, nil
}

func getGuiPkgNames() map[string][]string {
	var appInfoMap = make(map[string][]string)
	filepath.Walk(DesktopFilesLocation, func(path string, info os.FileInfo, err error) error {
		// Skip subdirectory
		if info.IsDir() && info.Name() != "applications" {
			return filepath.SkipDir
		}
		if strings.HasSuffix(info.Name(), ".desktop") {
			logger.Debug("Scanned desktop file:", info.Name())
			out, err := exec.Command("/usr/bin/dpkg", "-S", info.Name()).CombinedOutput()
			if err != nil {
				logger.Warning("failed to search package:", string(out))
				return nil
			}
			var pkgName = strings.Split(string(out), ": ")[0]
			logger.Warning("Found package:", string(pkgName))
			if !strings.Contains(pkgName, ARCH_I386) {
				pkgName = pkgName + ARCH_X86_64
			}
			appInfoMap[pkgName] = append(appInfoMap[pkgName], info.Name())
		}
		return nil
	})
	return appInfoMap
}

func (a *AppManager) CancelBackupApp(cancel bool) *dbus.Error {
	// Modify config file
	input, err := ioutil.ReadFile(BackupConfig)
	if err != nil {
		return dbusutil.ToError(err)
	}

	lines := strings.Split(string(input), "\n")
	for i, line := range lines {
		if strings.Contains(line, "/opt") {
			lines[i] = "      #- \"/opt\""
			if !cancel {
				lines[i] = "      - \"/opt\""
			}
		}
	}
	output := strings.Join(lines, "\n")
	err = ioutil.WriteFile(BackupConfig, []byte(output), 0644)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

// Migrate all deb packages that existed in the original system but not in the new system
func (a *AppManager) MigratePackages(sender dbus.Sender) (MigrateStatus, *dbus.Error) {
	// The filter will prevent some packages that break the system from being migrated
	err := a.buildPkgFilter()
	if err != nil {
		logger.Warning("failed to build pkg filter:", err)
		return makeMigrateStatus(Error, err.Error()), nil
	}
	logger.Debug("show filtered packages:", a.packageFilter)
	logger.Debug("show replaced packages:", a.repalcedPkgs)

	repo, err := atomic.GetOStreeRepoInfo("TARGET")
	if err != nil {
		return makeMigrateStatus(Error, err.Error()), nil
	}
	ostreeRepo := filepath.Join(repo, "osroot/snapshot/"+a.origSysCommitVersion)
	logger.Debug("get ostree repo path:", ostreeRepo)
	statusFilePath := filepath.Join(ostreeRepo, "/var/lib/dpkg/status")
	if _, err = os.Stat(statusFilePath); err != nil {
		return makeMigrateStatus(Error, "cannot find repack source dir"), nil
	}
	statusList, err := getStatusList(StatusFilePath)
	if err != nil {
		logger.Warning("failed to get status list:", err)
		return makeMigrateStatus(Error, err.Error()), nil
	}
	// Build a new system integration package map，this also protects the system from being corrupted during migration
	for _, status := range statusList {
		a.integratedPkgs[status.Package] = ""
	}

	prepareMigrateEnv()

	fullMigrate, err := config.GetIsFullMigrate()
	if err != nil {
		logger.Warning("failed to get full migrate info")
	}
	var apps []string
	if fullMigrate {
		apps, err := diffStatusFile(statusFilePath, StatusFilePath)
		if err != nil {
			logger.Warning("failed to get diff pkg info")
		}
		logger.Debugf("will migrate %d program, they are %v", len(apps), apps)
	} else {
		apps, err = getMigrateList()
		if err != nil {
			return makeMigrateStatus(Error, err.Error()), nil
		}
	}
	code, desc := a.installPackages(apps, false /* repack */, false /* isSimulate */, fullMigrate)
	if code == MigrateSuccessful {
		// After the migration is successful, some cleanup will be done
		a.handleAfterMigrate(sender)
	}
	return makeMigrateStatus(code, desc), nil
}

func makeMigrateStatus(code int, description string) MigrateStatus {
	var migrateStatus MigrateStatus
	migrateStatus.Code = code
	migrateStatus.Description = description
	return migrateStatus
}

func prepareMigrateEnv() {
	out, err := exec.Command("/usr/bin/dpkg", "--configure", "-a").CombinedOutput()
	if err != nil {
		logger.Warning("failed to run dpkg --configure -a:", string(out))
	}
	out, err = exec.Command("/usr/bin/apt-get", "update").CombinedOutput()
	if err != nil {
		logger.Warning(string(out))
	}
}

func (a *AppManager) handleAfterMigrate(sender dbus.Sender) {
	err := os.Remove(UpgradeAutoStartFile)
	if err != nil {
		logger.Warning("failed to remove file:", UpgradeAutoStartFile)
	}
	err = os.Remove(MigrateFlagsPath)
	if err != nil {
		logger.Warning("failed to remove file:", MigrateFlagsPath)
	}
	// out, err := exec.Command("/usr/bin/apt", "-y", "purge", "deepin-system-upgrade").CombinedOutput()
	// if err != nil {
	// 	logger.Warning("failed to remove upgrade tool:", string(out))
	// }
	err = os.Remove(UpgradeDesktopFile)
	if err != nil {
		logger.Warning("failed to remove file:", UpgradeDesktopFile)
	}

	err = a.cleanDesktopFile(sender)
	if err != nil {
		logger.Warning("failed to clean up desktop:", err)
	}
}

func diffStatusFile(origFile, newFile string) ([]string, error) {
	origList, err := getStatusList(origFile)
	if err != nil {
		return nil, err
	}
	newList, err := getStatusList(newFile)
	if err != nil {
		return nil, err
	}
	var retList []string = []string{"dpkg-repack:all"}
	for _, info := range origList {
		if newList.Exist(info) != nil {
			continue
		}
		pkgName := info.Package + ":" + info.Architecture
		retList = append(retList, pkgName)
	}
	return retList, nil
}

// Filters when building package migrations
func (a *AppManager) buildPkgFilter() error {
	// filter blacklist
	var filterList = []string{"grub-efi-amd64-signed", "dde", "dde-calendar", "deepin-album", "deepin-calculator", "deepin-camera", "deepin-compressor", "deepin-draw", "deepin-editor", "com.deepin.gomoku", "deepin-image-viewer", "com.deepin.lianliankan", "deepin-mail", "deepin-movie", "deepin-music", "deepin-reader", "deepin-screen-recorder", "org.deepin.browser"}
	for _, pkg := range filterList {
		pkgX86 := pkg + ARCH_X86_64
		a.packageFilter[pkgX86] = nil
		pkgI386 := pkg + ARCH_I386
		a.packageFilter[pkgI386] = nil
		pkgAll := pkg + ARCH_ALL
		a.packageFilter[pkgAll] = nil
	}
	err := a.addPkgFilter(StatusFilePath)
	if err != nil {
		return err
	}
	return nil
}

func getStatusList(filename string) (PackageStatusList, error) {
	fr, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer fr.Close()

	var statusList PackageStatusList
	scanner := bufio.NewScanner(fr)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) == 0 {
			continue
		}
		statusList = append(statusList, newPackageStatus(line, scanner))
	}
	return statusList, nil
}

// The core function of application migration, responsible for the installation of migration packages
// When the simulate parameter is False, it means that this function performs package migration on the new system;
// when simulate is True, it means that this function performs software compatibility judgment on the upgraded system
// When the repack parameter is True, it means that installPackages is in the recursive process at this time
func (a *AppManager) installPackages(pkgList []string, repack, simulate, fullMigrate bool) (code int, description string) {
	// Traverse the packages that need to be migrated
	logger.Warning("Start traversing the packages that need to be migrated:", pkgList)
	for index, pkg := range pkgList {
		if !repack {
			if getNetworkState() != NetworkConnected {
				return NetworkError, "Application migration failed, the network is interrupted, please check the network!"
			}
			a.emitProgressValue(100 * (index + 1) / len(pkgList))
		}
		if _, ok := a.repalcedPkgs[pkg]; ok {
			if a.repalcedPkgs[pkg] == nil {
				logger.Warningf("pkg  %s have been replaced:", pkg)
				if !repack {
					a.emitMigrateStatus(pkg, PkgInstallSuccess)
				}
				continue
			}
		}
		if _, ok := a.packageFilter[pkg]; ok {
			if a.packageFilter[pkg] == nil {
				logger.Warningf("pkg  %s have been filtered:", pkg)
				if repack {
					return Error, fmt.Sprintf("pkg  %s have been filtered:", pkg)
				}
				if simulate {
					// If it is filtered directly during the simulated installation.
					// it means that the application is replaced by Linglong package, which meets the compatibility
					a.emitMigrateStatus(pkg, PkgInstallSuccess)
				}
				continue
			}
		}
		// Determine if the package is already installed on the new system
		if queryPkgInstalled(pkg) == true {
			logger.Warningf("pkg %s has been installed", pkg)
			continue
		}

		// Find packages to install from the new system deb repository
		if queryPkgExistInRepo(pkg) == true {
			logger.Debug("install package from repository:", pkg)
			// For the stability of the system.
			// the newly installed package cannot uninstall the application package integrated in the original system
			if a.isPkgDestrCurSystem(pkg) {
				logger.Warningf("filter pkg %s that will destroy current system", pkg)
				if repack {
					return Error, fmt.Sprintf("pkg  %s have been filtered:", pkg)
				}
				a.emitMigrateStatus(pkg, PkgInstallFailed)
				continue
			}
			// Parse the packages that need to be installed together
			depends := parsePkgInstalledTogether(pkg)
			if depends == nil {
				if repack {
					return Error, fmt.Sprintf("pkg  %s have been filtered:", pkg)
				}
				a.emitMigrateStatus(pkg, PkgInstallFailed)
				continue
			}
			logger.Debugf("install pkg %s together install %v:", pkg, depends)

			// Use filters to determine whether a package can be installed.
			// This condition requires a deep comparison of the version number of the package
			state := a.checkPackages(depends, "")
			if state == PkgNotAllowInstall {
				if repack {
					return Error, fmt.Sprintf("pkg  %s have been filtered:", pkg)
				}
				a.emitMigrateStatus(pkg, PkgInstallFailed)
				continue
			} else if state == PkgBeenReplaced {
				a.repalcedPkgs[pkg] = nil
				if !repack {
					a.emitMigrateStatus(pkg, PkgInstallSuccess)
				}
				continue
			}
			err := a.installPkg(pkg, "", simulate)
			if err != nil {
				logger.Warning("append pkg %s to filter:", pkg)
				a.packageFilter[pkg] = nil
				a.emitMigrateStatus(pkg, PkgInstallFailed)
				if repack {
					return Error, fmt.Sprintf("failed to install packages from repo %s", pkg)
				}
			} else {
				a.emitMigrateStatus(pkg, PkgInstallSuccess)
				continue
			}
		}
		if !fullMigrate {
			// Just need to migrate GUI program
			a.emitMigrateStatus(pkg, PkgInstallFailed)
			continue
		}
		if pkgPath := tmpDirExistSameRepackPkg(pkg); pkgPath != "" {
			// This branch is used as one of the conditions for the end of recursion to directly install the package generated by dpkg-repack
			defer os.Remove(pkgPath)
			if err := a.installPkg(pkgPath, pkg, simulate); err != nil {
				logger.Warning("append filter:", pkg)
				a.packageFilter[pkg] = nil
				if repack {
					return Error, fmt.Sprintf("failed to install packages %s by repack", pkg)
				}
			} else {
				a.emitMigrateStatus(pkg, PkgInstallSuccess)
			}
		} else {
			// When the package that needs to be migrated cannot be found in the new system warehouse, enter this branch
			logger.Warning("begin to dpkg-repack packages:", pkg)
			path, err := a.repackPkgFromOrigSystem(pkg, a.origSysCommitVersion, simulate)
			if err != nil {
				a.emitMigrateStatus(pkg, PkgInstallFailed)
				logger.Warning("failed to repack package:", err)
				if repack {
					return Error, fmt.Sprintf("failed to repack packages %s", path)
				}
				continue
			}
			if a.isPkgDestrCurSystem(path) {
				os.Remove(path)
				logger.Warningf("filter pkg %s that will destroy current system", pkg)
				if repack {
					return Error, fmt.Sprintf("pkg  %s have been filtered:", pkg)
				}
				a.emitMigrateStatus(pkg, PkgInstallFailed)
				continue
			}
			state := a.checkPackages([]string{path}, pkg)
			logger.Warning("print check state[repack]:", state)
			if state == PkgNotAllowInstall {
				os.Remove(path)
				if repack {
					return Error, fmt.Sprintf("pkg  %s have been filtered:", pkg)
				}
				a.emitMigrateStatus(pkg, PkgInstallFailed)
				continue
			} else if state == PkgBeenReplaced {
				a.repalcedPkgs[pkg] = nil
				os.Remove(path)
				if !repack {
					a.emitMigrateStatus(pkg, PkgInstallSuccess)
				}
				continue
			}
			// Obtain the dependency information tree of the repack package
			if depends, err := queryPkgDepends(path, pkg); err == nil {
				// Recursively process the dependencies of the generated package, and the repack parameter is passed in true
				code, _ := a.installPackages(depends, true, simulate, true)
				if code == Error {
					os.Remove(path)
					a.emitMigrateStatus(pkg, PkgInstallFailed)
					if repack {
						return Error, fmt.Sprintf("failed to install packages %s", path)
					}
				}
			}
		}
	}
	return MigrateSuccessful, ""
}

func (a *AppManager) cleanDesktopFile(sender dbus.Sender) error {
	homeDir, err := user.GetHomeDirBySender(sender)
	if err != nil {
		return dbusutil.ToError(err)
	}
	curDesktopPath := filepath.Join(homeDir, "Desktop")
	filepath.Walk(curDesktopPath, func(path string, info os.FileInfo, err error) error {
		logger.Warning("Scanning", info.Name())
		if !info.IsDir() && strings.HasSuffix(info.Name(), ".desktop") {
			// If the desktop file is not in the /usr/share/applications directory after the migration is complete,
			// it means that the application does not meet the compatibility
			if _, err := os.Stat(filepath.Join(DesktopFilesLocation, info.Name())); err != nil {
				logger.Warning("remove desktop file:", path)
				err = os.Remove(path)
				if err != nil {
					logger.Warning("failed to remove desktop file:", err)
				}
			}
		}
		return nil
	})
	return nil
}

func (list PackageStatusList) Exist(info *PackageStatus) *PackageStatus {
	for _, v := range list {
		if v.Package == info.Package && v.Architecture == info.Architecture {
			return v
		}
	}
	return nil
}

func (info *PackageStatus) Equal(target *PackageStatus) bool {
	return reflect.DeepEqual(info, target)
}

func parseStatusLine(line string, info *PackageStatus) {
	items := strings.SplitN(line, _STATUS_DELIM, 2)
	if len(items) != 2 {
		info.Contents = append(info.Contents, line)
		return
	}

	switch items[0] {
	case _KEY_PACKAGE:
		info.Package = items[1]
	case _KEY_ARCHITECTURE:
		info.Architecture = items[1]
	case _KEY_CONFLICTS:
		info.Conflicts = items[1]
	case _KEY_REPLACES:
		info.Replaces = items[1]
	default:
		info.Contents = append(info.Contents, line)
	}
}

func newPackageStatus(line string, scanner *bufio.Scanner) *PackageStatus {
	var info PackageStatus

	parseStatusLine(line, &info)
	for scanner.Scan() {
		line = scanner.Text()
		if len(line) == 0 {
			break
		}

		parseStatusLine(line, &info)
	}
	return &info
}

func initSourceList(suffix string, extractPath string, root string) error {
	origin := filepath.Join(extractPath, suffix)
	fi, err := os.Open(origin)
	if err != nil {
		return err
	}
	f, err := os.OpenFile(filepath.Join(root, SouceListPath), os.O_WRONLY|os.O_TRUNC|os.O_CREATE, 0644)
	if err != nil {
		return err
	}
	defer fi.Close()
	defer f.Close()
	buf := bufio.NewScanner(fi)
	for {
		if !buf.Scan() {
			break
		}
		line := buf.Text()
		if strings.Contains(line, "apt_source_deb") {
			source := strings.Trim(strings.Split(line, "= ")[1], "\"") + "\n"
			f.WriteString(source)
			continue
		}
		if strings.Contains(line, "apt_source_deb_src") {
			source := strings.Trim(strings.Split(line, "= ")[1], "\"")
			f.WriteString(source)
			continue
		}
	}
	return nil
}

// Check the compatibility of software that needs to be migrated on the new system.
func (a *AppManager) checkAppsCompatibility(apps []string, extractPath, root string) *dbus.Error {
	// Configure sourcelist for new system environment
	err := initSourceList(SourceSettingsPath, extractPath, root)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	// Enable the new system to access the repository normally
	err = utils.CopyFile("/etc/resolv.conf", filepath.Join(root, "etc/resolv.conf"))
	if err != nil {
		return dbusutil.ToError(err)
	}
	out, err := exec.Command("/usr/bin/mount", "-o", "bind", "/run", filepath.Join(root, "run")).CombinedOutput()
	if err != nil {
		logger.Warning("failed to mount /run:", string(out))
		return dbusutil.ToError(err)
	}
	defer func() {
		out, err := exec.Command("/usr/bin/umount", filepath.Join(root, "run")).CombinedOutput()
		if err != nil {
			logger.Warning("failed to mount /run:", string(out))
		}
	}()

	originalRoot, err := os.Open("/")
	if err != nil {
		return dbusutil.ToError(err)
	}
	a.originalRoot = originalRoot
	a.newSystemRoot = root
	defer a.originalRoot.Close()
	err = syscall.Chroot(root)
	if err != nil {
		return dbusutil.ToError(err)
	}
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

	err = a.buildPkgFilter()
	if err != nil {
		return dbusutil.ToError(err)
	}
	logger.Debug(a.packageFilter)
	logger.Debug(a.repalcedPkgs)
	statusList, err := getStatusList(StatusFilePath)
	if err != nil {
		logger.Warning("failed to get status list:", err)
		return dbusutil.ToError(err)
	}

	for _, status := range statusList {
		a.integratedPkgs[status.Package] = ""
	}
	out, err = exec.Command("/usr/bin/apt-get", "update").CombinedOutput()
	if err != nil {
		logger.Warning(string(out))
	}
	fullMigrate, err := config.GetIsFullMigrate()
	if err != nil {
		logger.Warning("failed to get full migrate info")
	}
	logger.Warning("begin to install packages")
	code, desc := a.installPackages(apps, false, true, fullMigrate)
	if code != MigrateSuccessful {
		return dbusutil.ToError(errors.New(desc))
	}
	return nil
}

// Get the V20 branch name in the atomic update repository
func getOrigSysCommitVersion() string {
	var curVersion string
	f, err := os.Open(AtomicUpgradeVersionData)
	if err != nil {
		return ""
	}
	defer f.Close()
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := strings.Trim(scanner.Text(), "\n")
		curVersion = line
	}
	return curVersion
}

func queryPkgDepends(pkgPath string, pkg string) (depends []string, err error) {
	f, err := os.Open("/")
	f.Chdir()
	defer f.Close()
	out, err := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("dpkg --info %s |grep Depends", pkgPath)).CombinedOutput()
	if err != nil && len(string(out)) != 0 {
		logger.Warningf("failed to query depends for %s because of %s", pkgPath, string(out))
		return nil, err
	}
	if len(string(out)) == 0 {
		depends = append(depends, pkg)
		return depends, nil
	}
	for _, depend := range strings.Split(strings.Split(strings.Trim(string(out), "\n"), "Depends: ")[1], ", ") {
		if strings.Contains(depend, "|") {
			depend = strings.Split(depend, " |")[0]
		}
		if strings.Contains(depend, "(") {
			depend = strings.Split(depend, " (")[0]
		}
		// Add structural information i386/amd64
		depends = append(depends, depend+":"+strings.Split(pkg, ":")[1])
	}
	depends = append(depends, pkg)
	return depends, nil
}

// Use dpkg-repack to generate deb packages from the original system
func (a *AppManager) repackPkgFromOrigSystem(pkg, version string, simulate bool) (string, error) {
	var (
		repackOriginRepo    string
		pkgPath             string
		repackDebStorageDir = RepackDebStorageDir
		stopWalk            = errors.New("stop walking")
	)

	if !simulate {
		repo, err := atomic.GetOStreeRepoInfo("TARGET")
		if err != nil {
			return "", err
		}
		repackOriginRepo = filepath.Join(repo, "osroot/snapshot/"+version)
		logger.Warning(repackOriginRepo)
		if _, err = os.Stat(repackOriginRepo); err != nil {
			return "", errors.New("cannot find repack source dir")
		}
	} else {
		err := a.originalRoot.Chdir()
		if err != nil {
			logger.Warning("failed to chdir:", err)
			return "", err
		}
		err = syscall.Chroot(".")
		if err != nil {
			logger.Warning("failed to exit chroot:", err)
			return "", err
		}
		repackOriginRepo = "/"
		repackDebStorageDir = filepath.Join(a.newSystemRoot, repackDebStorageDir)
	}

	if _, err := os.Stat(repackDebStorageDir); err != nil {
		err = os.Mkdir(repackDebStorageDir, 0600)
		if err != nil {
			return "", err
		}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
	defer cancel()
	cmd := exec.CommandContext(ctx, "/usr/bin/dpkg-repack", "--root="+repackOriginRepo, pkg)
	cmd.Dir = repackDebStorageDir
	if err := cmd.Start(); err != nil {
		syscall.Chroot(a.newSystemRoot)
		return "", err
	}
	if err := cmd.Wait(); err != nil {
		syscall.Chroot(a.newSystemRoot)
		return "", err
	}
	if simulate {
		err := syscall.Chroot(a.newSystemRoot)
		if err != nil {
			return "", err
		}
	}
	pkg = strings.Split(pkg, ":")[0]
	filepath.Walk(RepackDebStorageDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			logger.Warning("filepath walk error:", err)
		}
		if info != nil {
			if strings.HasPrefix(info.Name(), pkg+"_") {
				pkgPath = path
				return stopWalk
			}
		}
		return nil
	})
	logger.Debug("print repacked pkg path:", pkgPath)
	return pkgPath, nil
}

func tmpDirExistSameRepackPkg(pkg string) string {
	pkg = strings.Split(pkg, ":")[0]
	var pkgPath string
	if _, err := os.Stat(RepackDebStorageDir); err != nil {
		return ""
	}
	filepath.Walk(RepackDebStorageDir, func(path string, info os.FileInfo, err error) error {
		if strings.HasPrefix(info.Name(), pkg+"_") {
			pkgPath = path
		}
		return nil
	})
	return pkgPath
}

func filterPackages(filterPkgVersions []string, pkgVersion string) bool {
	// As long as one condition is met, filter
	for _, filterPkgVersion := range filterPkgVersions {
		operator := strings.Split(filterPkgVersion, " ")[0] // << 1:10

		filterVersion := strings.Split(filterPkgVersion, " ")[1]
		version := strings.TrimSpace(pkgVersion)

		switch operator {
		case "<<":
			logger.Warningf("%s<<%s", version, filterVersion)
			if comparePkgVersion(version, filterVersion, "<") {
				return true
			}
		case "<=":
			logger.Warningf("%s<=%s", version, filterVersion)
			if comparePkgVersion(version, filterVersion, "<") || version == filterVersion {
				return true
			}
		case "=":
			logger.Warningf("%s==%s", version, filterVersion)
			if version == pkgVersion {
				return true
			}
		case ">=":
			logger.Warningf("%s>=%s", version, filterVersion)
			if comparePkgVersion(version, filterVersion, ">") || version == filterVersion {
				return true
			}
		case ">>":
			logger.Warningf("%s>>%s", version, filterVersion)
			if comparePkgVersion(version, filterVersion, ">") {
				return true
			}
		}
	}
	return false
}

// Ensure that all deb packages integrated in the new system will not be uninstalled during the application migration process
func (a *AppManager) isPkgDestrCurSystem(pkg string) bool {
	// This command can tell us which packages need to be uninstalled to install a package
	out, err := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("apt install -s %s |grep Remv", pkg)).CombinedOutput()
	if err != nil {
		return false
	}
	lines := strings.Split(string(out), "\n")
	for _, line := range lines {
		if strings.HasPrefix(line, "Remv ") {
			pkgName := strings.Split(line, " ")[1]
			// The package that needs to be uninstalled contains the package integrated with the new system, then filter
			if _, ok := a.integratedPkgs[pkgName]; ok {
				return true
			}
		}
	}
	return false
}

func (a *AppManager) checkPackages(pkgs []string, pkgName string) int {
	for _, pkg := range pkgs {
		_, inReplaces := a.repalcedPkgs[pkgName]
		_, inFilter := a.packageFilter[pkg]
		if inReplaces || inFilter {
			if inReplaces && a.repalcedPkgs[pkgName] == nil {
				logger.Warningf("pkg %s have been replaced", pkgName)
				return PkgBeenReplaced
			}
			if inFilter && a.packageFilter[pkg] == nil {
				logger.Warningf("pkg %s have been filtered", pkg)
				return PkgNotAllowInstall
			}
			var version string
			// Compare version number
			if pkgName == "" {
				// Get version number from package that can be installed from repository
				out, err := exec.Command("/usr/bin/apt-cache", "madison", pkg).CombinedOutput()
				if err != nil {
					logger.Warning("failed to madison pkg", string(out))
					return PkgNotAllowInstall
				}
				if len(string(out)) == 0 {
					logger.Warning("cannot find pkg version info from repo")
					return PkgNotAllowInstall
				}
				version = strings.Split(strings.TrimSpace(strings.Split(string(out), "\n")[0]), "|")[1]

			} else {
				// If a package cannot be installed from repository, we need to dpkg-repack a package first.
				out, err := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("dpkg --info %s |grep Version", pkg)).CombinedOutput()
				if err == nil {
					if len(string(out)) == 0 {
						logger.Warning("failed to get repack pkg version")
						return PkgNotAllowInstall
					}
					version = strings.Split(strings.Trim(string(out), "\n"), "Version: ")[1]
				} else {
					logger.Warning("failed to get repack pkg version")
					return PkgNotAllowInstall
				}
				logger.Warning(pkg)
				pkg = pkgName
				logger.Warning(pkgName)
			}
			logger.Debug("get pkg version:", version)
			if inReplaces {
				if filterPackages(a.repalcedPkgs[pkg], version) {
					logger.Warningf("pkg %s have been replaced", pkg)
					return PkgBeenReplaced
				}
			}
			if inFilter {
				if filterPackages(a.packageFilter[pkg], version) {
					logger.Warningf("pkg %s have been filtered", pkg)
					return PkgNotAllowInstall
				}
			}
		}

	}

	return PkgAllowInstall
}

func (a *AppManager) installPkg(pkg, pkgName string, simulate bool) error {
	f, err := os.Open("/")
	f.Chdir()
	defer f.Close()
	out, err := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("DEBIAN_FRONTEND='noninteractive' apt-get -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold'  --allow-unauthenticated install -s %s", pkg)).CombinedOutput()
	if err != nil {
		logger.Warning("failed to install -s pkg", string(out))
		return err
	}
	// When this flag is True, it means that the software compatibility judgment is being performed on the system being upgraded
	if simulate {
		return nil
	}
	out, err = exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("DEBIAN_FRONTEND='noninteractive' apt-get -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold'  --allow-unauthenticated install %s", pkg)).CombinedOutput()
	if err != nil {
		logger.Warning("failed to install pkg:", string(out))
		if pkgName == "" {
			pkgName = pkg
		}
		out, err = exec.Command("/usr/bin/apt", "-y", "--allow-unauthenticated", "remove", pkgName).CombinedOutput()
		if err != nil {
			logger.Warning("failed to uninstall package installed failed:", string(out))
		}
		return err
	}
	return nil
}

func queryPkgExistInRepo(pkgName string) bool {
	pkgName = strings.Split(pkgName, ":")[0]
	out, err := exec.Command("/usr/bin/apt-cache", "search", pkgName).CombinedOutput() // #nosec G204
	if err != nil {
		return false
	}
	for _, pkg := range strings.Split(string(out), "\n") {
		if strings.HasPrefix(pkg, pkgName+" - ") {
			return true
		}
	}
	return false
}

func queryPkgInstalled(pkg string) bool {
	pkg = strings.Split(pkg, ":")[0]
	_, err := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("/usr/bin/dpkg -s %s", pkg)).CombinedOutput()
	if err != nil {
		return false
	}
	return true
}

func parsePkgInstalledTogether(pkg string) []string {
	var depends []string
	out, err := exec.Command("/usr/bin/sh", "-c", fmt.Sprintf("apt install -s %s |grep Inst", pkg)).CombinedOutput()
	if err != nil {
		logger.Warningf("failed to get uninstalled depends, pkg %s have been filtered", pkg)
		return nil
	}
	if out == nil {
		logger.Warning("uninstalled depends is empty")
		return nil
	}
	lines := strings.Split(string(out), "\n")
	for _, line := range lines {
		if strings.HasPrefix(line, "Inst ") {
			pkgName := strings.Split(line, " ")[1]
			if !strings.Contains(pkgName, ARCH_I386) {
				pkgName = pkgName + ":" + strings.Split(pkg, ":")[1]
			}
			depends = append(depends, pkgName)
		}
	}
	return depends
}

func getNetworkState() uint32 {
	reply, err := dbustools.GetProperty("org.freedesktop.NetworkManager", "org.freedesktop.NetworkManager", "State", "/org/freedesktop/NetworkManager")
	if err != nil {
		logger.Warning("failed to get network state:", err)
		return 0
	}
	return reply.Value().(uint32)
}

func comparePkgVersion(versionA, versionB, operator string) bool {
	var oper string
	switch operator {
	case "<":
		oper = "lt"
	case ">":
		oper = "gt"
	}
	cmd := fmt.Sprintf("dpkg --compare-versions %s %s %s && echo true", versionA, oper, versionB)
	out, err := exec.Command("/usr/bin/sh", "-c", cmd).CombinedOutput()
	if err != nil {
		return false
	}
	if strings.Contains(string(out), "true") {
		return true
	}
	return false
}

func (a *AppManager) addPkgFilter(filepath string) error {
	status, err := getStatusList(filepath)
	if err != nil {
		return err
	}
	for _, status := range status {
		// Conflicts/Replaces/Breaks
		a.parsePkgInstallConditionInfo(status.Conflicts, "Conflicts: ", status.Architecture)
		a.parsePkgInstallConditionInfo(status.Replaces, "Replaces: ", status.Architecture)
		a.parsePkgInstallConditionInfo(status.Breaks, "Breaks: ", status.Architecture)
	}
	return nil
}

// Parse package installation condition information in the new system for building filters
func (a *AppManager) parsePkgInstallConditionInfo(pkgInfo, label, arch string) {
	var pkgInfos []string
	if strings.Contains(pkgInfo, ",") {
		pkgInfos = strings.Split(pkgInfo, ", ")
	} else {
		pkgInfos = append(pkgInfos, pkgInfo)
	}
	for _, pkgInfo := range pkgInfos {
		var filter *map[string][]string = &a.packageFilter
		if label == "Replaces: " {
			filter = &a.repalcedPkgs
		}
		if strings.Contains(pkgInfo, "(") {
			pkgName := strings.Split(pkgInfo, " (")[0] + ":" + arch
			newVersionRequest := strings.TrimSpace(strings.Split(strings.Split(pkgInfo, " (")[1], ")")[0])
			if _, ok := (*filter)[pkgName]; ok {
				if (*filter)[pkgName] == nil {
					return
				} else {
					// The update conditions are as follow：
					// same operator <</<= pick the minimal, >>/>= pick the maximum
					// The operators are different, add the condition to the filter
					operNew := strings.Split(newVersionRequest, " ")[0]
					for index, pkgItem := range (*filter)[pkgName] {
						oldVersionNum := strings.Split(pkgItem, " ")[1]
						newVersionNum := strings.Split(newVersionRequest, " ")[1]
						operOld := strings.Split(pkgItem, " ")[0]
						if strings.Contains(operNew, "<") && strings.Contains(operOld, "<") {
							// pick the minimal
							if comparePkgVersion(newVersionNum, oldVersionNum, ">") {
								// update filter condition
								logger.Warningf("update filter %s >> %s:", pkgInfo, pkgItem)
								(*filter)[pkgName][index] = newVersionRequest
							}
							goto finish
						} else if strings.Contains(operNew, ">") && strings.Contains(operOld, ">") {
							// pick the maximum
							if comparePkgVersion(newVersionNum, oldVersionNum, "<") {
								// update filter condition
								logger.Warningf("update filter %s << %s:", pkgInfo, pkgItem)
								(*filter)[pkgName][index] = newVersionRequest
							}
							goto finish
						}
						if newVersionNum == oldVersionNum {
							goto finish
						}
					}
					logger.Warning("append filter:", pkgInfo)
					(*filter)[pkgName] = append((*filter)[pkgName], newVersionRequest)
				}
			} else {
				(*filter)[pkgName] = []string{newVersionRequest}
			}
		} else {
			(*filter)[pkgInfo+":"+arch] = nil
		}
	finish:
	}

}

func (a *AppManager) emitProgressValue(value int) error {
	return a.service.Emit(a, "ProgressValue", value)
}

func (a *AppManager) emitMigrateStatus(pkg string, status int) error {
	return a.service.Emit(a, "MigrateStatus", pkg, status)
}
