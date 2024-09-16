// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bootkit

import (
	config "deepin-upgrade-manager/pkg/config/bootkit"
	"deepin-upgrade-manager/pkg/logger"
	"deepin-upgrade-manager/pkg/module/generator"
	"deepin-upgrade-manager/pkg/module/grub"
	"deepin-upgrade-manager/pkg/module/langselector"
	"deepin-upgrade-manager/pkg/module/util"
	"fmt"
	"os"
	"path"
	"path/filepath"
	"strings"
)

const (
	ToolScriptDir = "/var/lib/deepin-boot-kit/scripts"
)

var msgRollBack = util.Tr("System Recovery")

type Bootkit struct {
	conf            *config.BootConfig
	toolConfig      config.ToolConfigList
	versionListInfo config.VersionListInfo

	verManager generator.ListManager
}

func NewBootkit(conf *config.BootConfig) (*Bootkit, error) {
	var err error
	info := Bootkit{
		conf: conf}
	tool, err := config.LoadToolConfig(conf.Kit.Config)
	if err != nil {
		return &info, err
	}
	info.toolConfig = tool

	info.verManager, err = generator.Load(conf.Kit.Data)
	if nil != err {
		logger.Warningf("failed load file %s, err:%v", conf.Kit.Data, err)
	}
	return &info, nil
}

func (b *Bootkit) InitVersionInfo() {
	verListInfo, _ := config.LoadVersionConfig(b.toolConfig)
	b.versionListInfo = verListInfo
}

func (b *Bootkit) GetNewVersion() string {
	osVersion, err := util.GetOSInfo("", "MajorVersion")
	if nil != err {
		logger.Error("failed get new version, err:", err)
	}
	if len(osVersion) == 0 {
		osVersion = "23"
	}
	version, err := b.verManager.NewVersion("v" + osVersion)
	if nil != err {
		logger.Error("failed get new version, err:", err)
	}
	b.verManager.Append(version)
	b.verManager.Save()
	return version
}

func (b *Bootkit) ListVersion() []string {
	var verList []string
	for _, infolist := range b.versionListInfo {
		for _, info := range infolist.VersionList {
			verList = append(verList, info.Version)
		}
	}
	return generator.Sort(verList)
}

func (b *Bootkit) UpdateGrub() error {
	logger.Info("start update grub")
	err := util.ExecCommand("update-grub", []string{})
	return err
}

func (b *Bootkit) UpdateInitramfs() error {
	logger.Info("start update initramfs")
	err := util.ExecCommand(" update-initramfs", []string{"-u", "-k", "all"})
	return err
}

func (b *Bootkit) GetScriptDirList() []string {
	var scriptsList []string
	for _, info := range b.toolConfig {
		scriptsList = append(scriptsList, info.Initrd.ScriptDir)
	}
	return scriptsList
}

func (b *Bootkit) GenerateGrubMenu(menu, linux, initrd, grubcmdlinelinux, grubcmdlinelinuxdefault,
	backversion, backscheme, backuuid string) []string {
	var menus []string
	arg := grubcmdlinelinux + " " + grubcmdlinelinuxdefault + " " + backversion + " " + backscheme + " " + backuuid
	CLASS := "--class gnu-linux --class gnu --class os"
	UUID := os.Getenv("GRUB_DEVICE_UUID")
	submenu_indentation := os.Getenv("grub_tab")
	menuentry_id_option := os.Getenv("menuentry_id_option")
	menuentry := fmt.Sprintf("%smenuentry '%s' %s %s $menuentry_id_option 'gnulinux-simple-%s' {",
		submenu_indentation, menu, CLASS, menuentry_id_option, UUID)
	menus = append(menus, menuentry)

	quick_boot := os.Getenv("$quick_boot")
	if quick_boot == "1" {
		recordfail := fmt.Sprintf("%s recordfail", submenu_indentation)
		if len(recordfail) > 0 {
			menus = append(menus, recordfail)
		}
	}

	out, _ := util.ExecCommandWithOut("uname", []string{"-m"})
	arch := strings.TrimSpace(string(out))

	grubGfxpayloadLinux := os.Getenv("GRUB_GFXPAYLOAD_LINUX")
	if len(grubGfxpayloadLinux) == 0 && !strings.Contains(arch, "aarch64") {
		video := fmt.Sprintf("%s load_video", submenu_indentation)
		menus = append(menus, video)
	} else {
		if (grubGfxpayloadLinux != "text") && !strings.Contains(arch, "aarch64") {
			video := fmt.Sprintf("%s load_video", submenu_indentation)
			menus = append(menus, video)
		}
	}
	gfxpayload_dynamic := os.Getenv("gfxpayload_dynamic")
	if len(grubGfxpayloadLinux) != 0 && gfxpayload_dynamic == "1" {
		linux_gfx_mode := os.Getenv("linux_gfx_mode")
		gfxmode := fmt.Sprintf("%s gfxmode %s", submenu_indentation, linux_gfx_mode)
		menus = append(menus, gfxmode)
	}
	insmod := fmt.Sprintf("%s insmod gzio", submenu_indentation)
	menus = append(menus, insmod)
	grub_platform := fmt.Sprintf("%s if [ x $grub_platform = xxen ]; then insmod xzio; insmod lzopio; fi", submenu_indentation)
	menus = append(menus, grub_platform)
	prepare := os.Getenv("BOOT_KIT_PREPARE")
	menus = append(menus, prepare)

	linuxMessage := ("	echo 'Loading Linux  ...'")
	menus = append(menus, linuxMessage)

	linuxDir := linux + ".efi.signed"
	linuxRoot := os.Getenv("LINUX_ROOT_DEVICE")
	if util.IsExists("/sys/firmware/efi") && util.IsExists(linuxDir) {
		linuxCmd := fmt.Sprintf("	linux %s root=%s ro %s", linuxDir, linuxRoot, arg)
		menus = append(menus, linuxCmd)
	} else {
		linuxCmd := fmt.Sprintf("	linux %s root=%s ro %s", linux, linuxRoot, arg)
		menus = append(menus, linuxCmd)
	}

	inirdMessage := ("	echo 'Loading initial ramdisk ...'")
	menus = append(menus, inirdMessage)
	menus = append(menus, fmt.Sprintf("	initrd %s", initrd))
	menus = append(menus, "}")
	return menus
}

func (b *Bootkit) GenerateDefaultGrub() string {
	var grubInfos []string

	var list config.VersionListConf
	for _, v := range b.versionListInfo {
		list = append(list, v.VersionList...)
	}
	sortList := list.Sort()
	if len(list) == 0 {
		return ""
	}
	var usersLine string
	grubManager := grub.Init()
	encryptedUsers, err := grubManager.GetEnabledUsers()
	if nil == err && len(encryptedUsers) != 0 {
		usersLine = fmt.Sprintf("--users %s ", strings.Join(encryptedUsers, " "))
	}
	UUID := os.Getenv("GRUB_DEVICE_UUID")
	menuentry_id_option := os.Getenv("menuentry_id_option")
	getTextOut, _ := util.GetBootKitText(msgRollBack, langselector.LocalLangEnv())
	submenu := fmt.Sprintf("submenu '%s' $menuentry_id_option %s 'gnulinux-advanced-%s' %s{",
		getTextOut, menuentry_id_option, UUID, usersLine)
	grubInfos = append(grubInfos, submenu)
	for index, v := range sortList {
		if index >= b.conf.Kit.MaxVersionRetention {
			break
		}
		display := v.DisplayInfo
		if len(display) == 0 {
			display = fmt.Sprintf("Rollback to %s", v)
		}

		menu_entry := display
		grubCmdliuxLinux := os.Getenv("GRUB_CMDLINE_LINUX")
		grubCmdliuxLinuxDefault := os.Getenv("GRUB_CMDLINE_LINUX_DEFAULT")
		backVersion := fmt.Sprintf("back_version=%s", v.Version)
		backScheme := fmt.Sprintf("back_scheme=%s", v.Scheme)
		var backuuid string
		if len(v.UUID) != 0 {
			backuuid = fmt.Sprintf("backup_uuid=%s", v.UUID)
		}
		menus := b.GenerateGrubMenu(menu_entry, v.Kernel, v.Initrd, grubCmdliuxLinux,
			grubCmdliuxLinuxDefault, backVersion, backScheme, backuuid)
		grubInfos = append(grubInfos, menus...)
	}
	grubInfos = append(grubInfos, "}")
	return util.SliceToString(grubInfos)
}

func (b *Bootkit) CopyToolScripts() error {
	err := os.MkdirAll(ToolScriptDir, 0644)
	if err != nil {
		return err
	}
	for _, info := range b.toolConfig {
		filenameall := path.Base(info.ConfigFileName)
		filesuffix := path.Ext(info.ConfigFileName)
		fileprefix := filenameall[0 : len(filenameall)-len(filesuffix)]
		dstDir := filepath.Join(ToolScriptDir, fileprefix)
		err := os.MkdirAll(dstDir, 0644)
		if err != nil {
			return err
		}
		util.CopyDir(info.Initrd.ScriptDir, dstDir, nil, nil, false)
	}
	return nil
}
