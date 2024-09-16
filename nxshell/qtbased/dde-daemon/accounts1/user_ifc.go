// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

/*
#cgo LDFLAGS: -lcrypt

#include <unistd.h>
#include <crypt.h>
#include <stdlib.h>

#include <shadow.h>
typedef struct spwd cspwd;
*/
import "C"
import (
	"bufio"
	"bytes"
	"encoding/base64"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"time"
	"unsafe"

	dbus "github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-api/lang_info"
	"github.com/linuxdeepin/dde-daemon/accounts1/users"
	"github.com/linuxdeepin/dde-daemon/common/sessionmsg"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/gdkpixbuf"
	"github.com/linuxdeepin/go-lib/imgutil"
	"github.com/linuxdeepin/go-lib/strv"
	dutils "github.com/linuxdeepin/go-lib/utils"
	"golang.org/x/xerrors"
)

const (
	userDBusPathPrefix = "/org/deepin/dde/Accounts1/User"
	userDBusInterface  = "org.deepin.dde.Accounts1.User"
	controlCenter      = "dde-control-center"
	resetPasswordDia   = "reset-password-dialog"
)

func (*User) GetInterfaceName() string {
	return userDBusInterface
}

func (u *User) SetFullName(sender dbus.Sender, name string) *dbus.Error {
	logger.Debugf("[SetFullName] new name: %q", name)

	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetFullName] access denied:", err)
		return dbusutil.ToError(err)
	}

	if !u.checkIsControlCenter(sender) {
		logger.Debug("[SetLocale] access denied: only dde-control-center allowed")
		return dbusutil.ToError(fmt.Errorf("only dde-control-center allowed to call this method"))
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.FullName != name {
		if err := users.ModifyFullName(name, u.UserName); err != nil {
			logger.Warning("DoAction: modify full name failed:", err)
			return dbusutil.ToError(err)
		}

		u.FullName = name
		_ = u.emitPropChangedFullName(name)
	}

	return nil
}

func (u *User) SetHomeDir(sender dbus.Sender, home string) *dbus.Error {
	logger.Debug("[SetHomeDir] new home:", home)

	err := u.checkAuth(sender, false, "")
	if err != nil {
		logger.Debug("[SetHomeDir] access denied:", err)
		return dbusutil.ToError(err)
	}

	if dutils.IsFileExist(home) {
		// if new home already exists, the `usermod -m -d` command will fail.
		return dbusutil.ToError(errors.New("new home already exists"))
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.HomeDir != home {
		if err := users.ModifyHome(home, u.UserName); err != nil {
			logger.Warning("DoAction: modify home failed:", err)
			return dbusutil.ToError(err)
		}
		u.HomeDir = home
		_ = u.emitPropChangedHomeDir(home)
	}

	return nil
}

func (u *User) SetShell(sender dbus.Sender, shell string) *dbus.Error {
	logger.Debug("[SetShell] new shell:", shell)

	err := u.checkAuth(sender, false, "")
	if err != nil {
		logger.Debug("[SetShell] access denied:", err)
		return dbusutil.ToError(err)
	}

	shells := getAvailableShells("/etc/shells")
	if len(shells) == 0 {
		err := fmt.Errorf("no available shell found")
		logger.Error("[SetShell] failed:", err)
		return dbusutil.ToError(err)
	}

	if !strv.Strv(shells).Contains(shell) {
		err := fmt.Errorf("not found the shell: %s", shell)
		logger.Warning("[SetShell] failed:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.Shell != shell {
		if err := users.ModifyShell(shell, u.UserName); err != nil {
			logger.Warning("DoAction: modify shell failed:", err)
			return dbusutil.ToError(err)
		}
		u.Shell = shell
		_ = u.emitPropChangedShell(shell)
	}

	return nil
}

func (u *User) SetPassword(sender dbus.Sender, password string) *dbus.Error {
	logger.Debug("[SetPassword] start ...")

	// set password from UnionID
	if password == "" {
		err := u.setPwdWithUnionID(sender)
		if err != nil {
			return dbusutil.ToError(err)
		} else {
			return nil
		}
	}

	err := u.checkAuth(sender, false, "")
	if err != nil {
		logger.Debug("[SetPassword] access denied:", err)
		return dbusutil.ToError(err)
	}

	var count = 10
	for {
		_, err := users.GetShadowInfo(u.UserName)

		if err == nil {
			break
		}
		count--
		if count == 0 {
			return dbusutil.ToError(err)
		}
		time.Sleep(time.Second)
	}

	if err := users.ModifyPasswd(password, u.UserName); err != nil {
		logger.Warning("DoAction: modify password failed:", err)
		return dbusutil.ToError(err)
	}

	err = removeLoginKeyring(u)
	if err != nil {
		logger.Warningf("DoAction: remove login keyring failed: %v", err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.Locked {
		if err := users.LockedUser(false, u.UserName); err != nil {
			logger.Warning("DoAction: unlock user failed:", err)
			return dbusutil.ToError(err)
		}
		u.Locked = false
		_ = u.emitPropChangedLocked(false)
	}
	return nil
}

func (u *User) SetMaxPasswordAge(sender dbus.Sender, nDays int32) *dbus.Error {
	err := u.checkAuth(sender, false, "")
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	err = users.ModifyMaxPasswordAge(u.UserName, int(nDays))
	if err != nil {
		logger.Warning("failed to set max password age:", err)
		return dbusutil.ToError(err)
	}

	return nil
}

func (u *User) IsPasswordExpired() (bool, *dbus.Error) {
	// LDAP 域用户密码由域服务器控制，系统不去做密码过期检测
	if users.IsLDAPDomainUserID(u.Uid) {
		return false, nil
	}

	v, err := users.IsPasswordExpired(u.UserName)
	return v, dbusutil.ToError(err)
}

func (u *User) SetLocked(sender dbus.Sender, locked bool) *dbus.Error {
	logger.Debug("[SetLocked] locked:", locked)

	err := u.checkAuth(sender, false, polkitActionUserAdministration)
	if err != nil {
		logger.Debug("[SetLocked] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.Locked != locked {
		if err := users.LockedUser(locked, u.UserName); err != nil {
			logger.Warning("DoAction: locked user failed:", err)
			return dbusutil.ToError(err)
		}

		u.Locked = locked
		_ = u.emitPropChangedLocked(locked)

		if locked && u.AutomaticLogin {
			if err := users.SetAutoLoginUser("", ""); err != nil {
				logger.Warning("failed to clear auto login user:", err)
				return dbusutil.ToError(err)
			}
			u.AutomaticLogin = false
			_ = u.emitPropChangedAutomaticLogin(false)
		}
	}
	return nil
}

func (u *User) AddGroup(sender dbus.Sender, group string) *dbus.Error {
	logger.Debugf("add group %s for %s", group, u.UserName)
	err := u.checkAuth(sender, false, polkitActionUserAdministration)
	if err != nil {
		logger.Debug("[AddGroup] access denied:", err)
		return dbusutil.ToError(err)
	}

	err = users.AddGroupForUser(group, u.UserName)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) DeleteGroup(sender dbus.Sender, group string) *dbus.Error {
	logger.Debugf("delete group %s for %s", group, u.UserName)
	err := u.checkAuth(sender, false, polkitActionUserAdministration)
	if err != nil {
		logger.Debug("[DeleteGroup] access denied:", err)
		return dbusutil.ToError(err)
	}

	err = users.DeleteGroupForUser(group, u.UserName)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) SetGroups(sender dbus.Sender, groups []string) *dbus.Error {
	logger.Debugf("set groups %v for %s", groups, u.UserName)
	err := u.checkAuth(sender, false, polkitActionUserAdministration)
	if err != nil {
		logger.Debug("[SetGroups] access denied:", err)
		return dbusutil.ToError(err)
	}

	err = users.SetGroupsForUser(groups, u.UserName)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) SetAutomaticLogin(sender dbus.Sender, enabled bool) *dbus.Error {
	logger.Debug("[SetAutomaticLogin] enable:", enabled)

	err := u.checkAuthAutoLogin(sender, enabled)
	if err != nil {
		logger.Debug("[SetAutomaticLogin] access denied:", err)
		return dbusutil.ToError(err)
	}

	return u.setAutomaticLogin(enabled)
}

// 设置用户是否快速登录
func (u *User) SetQuickLogin(sender dbus.Sender, enabled bool) *dbus.Error {
	logger.Infof("DBus call SetQuickLogin sender %v, enabled %t", sender, enabled)

	// 验证调用者权限
	err := u.checkAuthQuickLogin(sender, enabled)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	return u.setQuickLogin(enabled)
}

func (u *User) EnableNoPasswdLogin(sender dbus.Sender, enabled bool) *dbus.Error {
	logger.Debug("[EnableNoPasswdLogin] enabled:", enabled)

	err := u.checkAuthNoPasswdLogin(sender, enabled)
	if err != nil {
		logger.Debug("[EnableNoPasswdLogin] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.Locked {
		return dbusutil.ToError(fmt.Errorf("user %s has been locked", u.UserName))
	}

	if u.NoPasswdLogin == enabled {
		return nil
	}

	if err := users.EnableNoPasswdLogin(u.UserName, enabled); err != nil {
		logger.Warning("DoAction: enable no password login failed:", err)
		return dbusutil.ToError(err)
	}

	u.NoPasswdLogin = enabled
	_ = u.emitPropChangedNoPasswdLogin(enabled)
	return nil
}

func (v *User) setLocale(value string) {
	value = strings.Trim(value, "\"'")
	if v.Locale != value {
		v.Locale = value
	}
}

func (u *User) SetLocale(sender dbus.Sender, locale string) *dbus.Error {
	logger.Debug("[SetLocale] locale:", locale)

	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetLocale] access denied:", err)
		return dbusutil.ToError(err)
	}

	if !u.checkIsDeepinDaemon(sender) {
		logger.Debug("[SetLocale] access denied: only deepin daemons allowed")
		return dbusutil.ToError(fmt.Errorf("only deepin daemons allowed to call this method"))
	}

	if !lang_info.IsSupportedLocale(locale) {
		err := fmt.Errorf("invalid locale %q", locale)
		logger.Debug("[SetLocale]", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.Locale == locale {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyLocale, locale)
	if err != nil {
		return dbusutil.ToError(err)
	}
	u.setLocale(locale)
	_ = u.emitPropChangedLocale(locale)
	return nil
}

func (u *User) SetLayout(sender dbus.Sender, layout string) *dbus.Error {
	logger.Debug("[SetLayout] new layout:", layout)

	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetLayout] access denied:", err)
		return dbusutil.ToError(err)
	}

	// TODO: check layout validity

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.Layout == layout {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyLayout, layout)
	if err != nil {
		return dbusutil.ToError(err)
	}
	u.Layout = layout
	_ = u.emitPropChangedLayout(layout)
	return nil
}

func (u *User) SetIconFile(sender dbus.Sender, iconURI string) *dbus.Error {
	logger.Debug("[SetIconFile] new icon:", iconURI)

	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetIconFile] access denied:", err)
		return dbusutil.ToError(err)
	}

	iconURI = dutils.EncodeURI(iconURI, dutils.SCHEME_FILE)
	iconFile := dutils.DecodeURI(iconURI)

	// check if file exist
	_, err = os.Stat(iconFile)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	// 找到要移除的原始用户头像
	var oldIconPath string
	for _, icon := range u.customIconList {
		if strings.Contains(icon, path.Base(iconURI)) {
			oldIconPath = icon
			break
		}
	}

	// if iconURI not in iconList, need to create temp icon file
	if !isStrInArray(iconURI, u.IconList) {
		// copy file to temp file, update icon file
		iconFile, err = copyTempIconFile(iconFile, u.UserName)
		if err != nil {
			logger.Warningf("copy temp file failed, err: %v", err)
			return dbusutil.ToError(err)
		}
		// remove file
		defer func() {
			err := os.Remove(iconFile)
			if err != nil {
				logger.Warningf("remove temp file failed, err: %v", err)
				return
			}
		}()
		// if temp icon file is create, update icon URI
		iconURI = dutils.EncodeURI(iconFile, dutils.SCHEME_FILE)
	}

	if !gdkpixbuf.IsSupportedImage(iconFile) {
		err := fmt.Errorf("%q is not a image file", iconFile)
		logger.Debug(err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if u.IconFile == iconURI {
		return nil
	}

	newIconURI, added, err := u.setIconFile(iconURI, len(oldIconPath) > 0)
	if err != nil {
		logger.Warning("Set icon failed:", err)
		return dbusutil.ToError(err)
	}

	if added {
		// newIconURI should be custom icon if added is true
		err = u.writeUserConfigWithChanges([]configChange{
			{confKeyCustomIcon, newIconURI},
			{confKeyIcon, newIconURI},
		})

		if err != nil {
			return dbusutil.ToError(err)
		}

		// remove old custom icon
		if oldIconPath != "" {
			logger.Debugf("remove old custom icon %q", u.customIcon)
			err := os.Remove(dutils.DecodeURI(oldIconPath))
			if err != nil {
				logger.Warning(err)
			}
		}

		// 默认的icon如果不是default.png的情况下，会被设置为customIcon,会被误删,
		// 为规避已经设置为customIcon的历史版本,判断如果是icons目录下的数据,就不删除
		needRemove := func(uri string) bool {
			if uri == "" {
				return true
			}
			fPath := dutils.DecodeURI(u.customIcon)
			return filepath.Dir(fPath) != "/var/lib/AccountsService/icons"
		}
		// remove old custom icon
		if needRemove(u.customIcon) {
			logger.Debugf("remove old custom icon %q", u.customIcon)
			err := os.Remove(dutils.DecodeURI(u.customIcon))
			if err != nil {
				logger.Warning(err)
			}
		}
		u.customIcon = newIconURI
		u.customIconList = append(u.customIconList, newIconURI)
		u.updateIconList()
	} else {
		err = u.writeUserConfigWithChange(confKeyIcon, newIconURI)
		if err != nil {
			return dbusutil.ToError(err)
		}
	}

	u.IconFile = newIconURI
	_ = u.emitPropChangedIconFile(newIconURI)
	return nil
}

// 只能删除不是用户当前图标的自定义图标
func (u *User) DeleteIconFile(sender dbus.Sender, icon string) *dbus.Error {
	logger.Debug("[DeleteIconFile] icon:", icon)

	dir, err := filepath.Abs(filepath.Dir(icon))
	if err != nil || dir != userCustomIconsDir {
		return dbusutil.ToError(fmt.Errorf("%s is not in %s", icon, userCustomIconsDir))
	}

	base := filepath.Base(icon)
	if !strings.HasPrefix(base, u.UserName) {
		return dbusutil.ToError(fmt.Errorf("%s is not belong to %s", icon, u.UserName))
	}

	err = u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[DeleteIconFile] access denied:", err)
		return dbusutil.ToError(err)
	}

	icon = dutils.EncodeURI(icon, dutils.SCHEME_FILE)
	if !u.IsIconDeletable(icon) {
		err := errors.New("this icon is not allowed to be deleted")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	iconPath := dutils.DecodeURI(icon)
	if err := os.Remove(iconPath); err != nil {
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	// set custom icon to empty
	err = u.writeUserConfigWithChange(confKeyCustomIcon, "")
	if err != nil {
		return dbusutil.ToError(err)
	}
	u.customIcon = ""
	u.updateIconList()
	return nil
}

func (u *User) EnableWechatAuth(sender dbus.Sender, value bool) *dbus.Error {
	logger.Infof("DBus call EnableWechatAuth sender %v, enabled %t", sender, value)
	// 验证调用者权限
	if !u.checkIsControlCenter(sender) {
		return dbusutil.ToError(fmt.Errorf("not allow %v call this method", sender))
	}
	return u.enableWechatAuth(value)
}
func (u *User) UpdateWechatAuthState() *dbus.Error {
	logger.Infof("DBus call UpdateWechatAuthState")
	// 通过synchelper检查本地账户是否有绑定的UOS ID
	syncObj := u.service.Conn().Object("com.deepin.sync.Helper", "/com/deepin/sync/Helper")
	var uosid string
	err := syncObj.Call("com.deepin.sync.Helper.UOSID", 0).Store(&uosid)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	var ubid string
	err = syncObj.Call("com.deepin.sync.Helper.LocalBindCheck", 0, uosid, u.UUID).Store(&ubid)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	var isBindWechat bool
	var wechatNickName string
	err = syncObj.Call("com.deepin.sync.Helper.LocalBindCheckWechat", 0, uosid, u.UUID).Store(&isBindWechat, &wechatNickName)
	if err != nil {
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	logger.Infof("DBus call UpdateWechatAuthState ubid %v, wechatNickName %v", ubid, wechatNickName)
	if (ubid == "" || !isBindWechat) && u.WechatAuthEnabled {
		return u.enableWechatAuth(false)
	}

	return nil
}

func (u *User) enableWechatAuth(value bool) *dbus.Error {
	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.WechatAuthEnabled {
		return nil
	}

	err := u.writeUserConfigWithChange(confKeyWechatAuthEnabled, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.setPropWechatAuthEnabled(value)
	return nil
}

func (u *User) SetDesktopBackgrounds(sender dbus.Sender, val []string) *dbus.Error {
	logger.Debugf("[SetDesktopBackgrounds] val: %#v", val)

	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetDesktopBackgrounds] access denied:", err)
		return dbusutil.ToError(err)
	}

	if len(val) == 0 {
		return dbusutil.ToError(errors.New("val is empty"))
	}

	var newVal = make([]string, len(val))
	for idx, file := range val {
		newVal[idx] = dutils.EncodeURI(file, dutils.SCHEME_FILE)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if strv.Strv(u.DesktopBackgrounds).Equal(newVal) {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyDesktopBackgrounds, newVal)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.DesktopBackgrounds = newVal
	_ = u.emitPropChangedDesktopBackgrounds(newVal)
	return nil
}

// 记录当前工作区，登录时前端从记录文件中获取当前工作区以及相应的桌面背景
func (u *User) SetCurrentWorkspace(sender dbus.Sender, currentWorkspace int32) *dbus.Error {
	logger.Debug("[SetCurrentWorkspace] currentWorkspace", currentWorkspace)

	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetCurrentWorkspace] access denied:", err)
		return dbusutil.ToError(err)
	}

	if currentWorkspace <= 0 {
		return dbusutil.ToError(errors.New("currentWorkspace is err"))
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if currentWorkspace == u.Workspace {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyWorkspace, currentWorkspace)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.Workspace = currentWorkspace
	_ = u.emitPropChangedWorkspace(currentWorkspace)
	return nil
}

func (u *User) SetGreeterBackground(sender dbus.Sender, bg string) *dbus.Error {
	logger.Debug("[SetGreeterBackground] new background:", bg)
	if dutils.IsFileExist("/var/lib/deepin/permission-manager/wallpaper_locked") {
		_ = sessionmsg.SendMessage(sessionmsg.NewMessage(true, &sessionmsg.BodyNotify{
			Icon: "preferences-system",
			Body: &sessionmsg.LocalizeStr{
				Format: Tr("This system wallpaper is locked. Please contact your admin."),
				Args:   []string{},
			},
			ExpireTimeout: -1,
		}))
		return dbusutil.ToError(errors.New("do not have permission to change greeter background"))
	}
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetGreeterBackground] access denied:", err)
		return dbusutil.ToError(err)
	}

	bg = dutils.EncodeURI(bg, dutils.SCHEME_FILE)

	if !isBackgroundValid(bg) {
		err := ErrInvalidBackground{bg}
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer func() {
		u.PropsMu.Unlock()
		genGaussianBlur(bg)
	}()

	if u.GreeterBackground == bg {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyGreeterBackground, bg)
	if err != nil {
		return dbusutil.ToError(err)
	}
	u.GreeterBackground = bg
	_ = u.emitPropChangedGreeterBackground(bg)
	return nil
}

func (u *User) SetHistoryLayout(sender dbus.Sender, list []string) *dbus.Error {
	logger.Debug("[SetHistoryLayout] new history layout:", list)

	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetHistoryLayout] access denied:", err)
		return dbusutil.ToError(err)
	}

	// TODO: check layout list whether validity

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if isStrvEqual(u.HistoryLayout, list) {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyHistoryLayout, list)
	if err != nil {
		return dbusutil.ToError(err)
	}
	u.HistoryLayout = list
	_ = u.emitPropChangedHistoryLayout(list)
	return nil
}

func (u *User) SetUse24HourFormat(sender dbus.Sender, value bool) *dbus.Error {
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetUse24HourFormat] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.Use24HourFormat {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyUse24HourFormat, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.Use24HourFormat = value
	err = u.emitPropChangedUse24HourFormat(value)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) IsIconDeletable(iconURI string) bool {
	u.PropsMu.RLock()
	defer u.PropsMu.RUnlock()

	if iconURI != u.IconFile && iconURI == u.customIcon {
		// iconURI is custom icon, and not current icon
		return true
	}
	return true
}

// 获取当前头像的大图标
func (u *User) GetLargeIcon() string {
	u.PropsMu.RLock()
	baseName := path.Base(u.IconFile)
	dir := path.Dir(dutils.DecodeURI(u.IconFile))
	u.PropsMu.RUnlock()

	filename := path.Join(dir, "bigger", baseName)
	if !dutils.IsFileExist(filename) {
		return ""
	}

	return dutils.EncodeURI(filename, dutils.SCHEME_FILE)
}

var supportedFormats = strv.Strv([]string{"gif", "jpeg", "png", "bmp", "tiff"})

func isBackgroundValid(file string) bool {
	file = dutils.DecodeURI(file)
	format, err := imgutil.SniffFormat(file)
	if err != nil {
		return false
	}

	if supportedFormats.Contains(format) {
		return true
	}
	return false
}

type ErrInvalidBackground struct {
	FileName string
}

func (err ErrInvalidBackground) Error() string {
	return fmt.Sprintf("%q is not a valid background file", err.FileName)
}

// copy file to root dir
func copyTempIconFile(src string, username string) (string, error) {
	// make dde-daemon dir
	daemonDir := "/var/cache/deepin/dde-daemon"
	err := os.MkdirAll(daemonDir, 0644)
	if err != nil {
		logger.Warningf("make dir failed, err: %v", err)
		return "", err
	}
	// make image dir
	imageDir := filepath.Join(daemonDir, "icon")
	// make dir, only superuser can write and writer
	err = os.MkdirAll(imageDir, 0600)
	if err != nil {
		logger.Warningf("make dir failed, err: %v", err)
		return "", err
	}
	// create target file path
	ns := time.Now().UnixNano()
	base := username + "-" + strconv.FormatInt(ns, 36)
	file := filepath.Join(imageDir, base)
	// copy file
	err = dutils.CopyFile(src, file)
	if err != nil {
		logger.Warningf("copy file failed, err: %v", err)
		return "", err
	}
	return file, nil
}

func (u *User) SetWeekdayFormat(sender dbus.Sender, value int32) *dbus.Error {
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetWeekdayFormat] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.WeekdayFormat {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyWeekdayFormat, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.WeekdayFormat = value
	err = u.emitPropChangedWeekdayFormat(value)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) SetShortDateFormat(sender dbus.Sender, value int32) *dbus.Error {
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetShortDateFormat] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.ShortDateFormat {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyShortDateFormat, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.ShortDateFormat = value
	err = u.emitPropChangedShortDateFormat(value)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) SetLongDateFormat(sender dbus.Sender, value int32) *dbus.Error {
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetLongDateFormat] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.LongDateFormat {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyLongDateFormat, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.LongDateFormat = value
	err = u.emitPropChangedLongDateFormat(value)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) SetShortTimeFormat(sender dbus.Sender, value int32) *dbus.Error {
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetShortTimeFormat] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.ShortTimeFormat {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyShortTimeFormat, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.ShortTimeFormat = value
	err = u.emitPropChangedShortTimeFormat(value)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) SetLongTimeFormat(sender dbus.Sender, value int32) *dbus.Error {
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetLongTimeFormat] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.LongTimeFormat {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyLongTimeFormat, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.LongTimeFormat = value
	err = u.emitPropChangedLongTimeFormat(value)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) SetWeekBegins(sender dbus.Sender, value int32) *dbus.Error {
	err := u.checkAuth(sender, true, "")
	if err != nil {
		logger.Debug("[SetWeekBegins] access denied:", err)
		return dbusutil.ToError(err)
	}

	u.PropsMu.Lock()
	defer u.PropsMu.Unlock()

	if value == u.WeekBegins {
		return nil
	}

	err = u.writeUserConfigWithChange(confKeyWeekBegins, value)
	if err != nil {
		return dbusutil.ToError(err)
	}

	u.WeekBegins = value
	err = u.emitPropChangedWeekBegins(value)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

type ExpiredStatus int

const (
	expiredStatusNormal ExpiredStatus = iota
	expiredStatusExpiredSoon
	expiredStatusExpiredAlready
)

const secondsPerDay = 60 * 60 * 24

func (u *User) PasswordExpiredInfo() (expiredStatus ExpiredStatus, dayLeft int64, busErr *dbus.Error) {
	var pw *C.cspwd
	pw = C.getspnam(C.CString(u.UserName))
	if pw == nil {
		return expiredStatusNormal, 0, dbusutil.ToError(fmt.Errorf("get passwd for %s failed", u.UserName))
	}

	var spMax = int64(pw.sp_max)
	var spWarn = int64(pw.sp_warn)
	var spLastChg = int64(pw.sp_lstchg)

	if spLastChg == 0 {
		// expired
		return expiredStatusExpiredAlready, 0, nil
	}
	if spMax == -1 {
		// never expired
		return expiredStatusNormal, -1, nil
	}

	// pam_unix/passverify.c
	curDays := time.Now().Unix() / secondsPerDay
	daysLeft := spLastChg + spMax - curDays

	// daysLeft < 0时为已经过期，等于0时即当天过期时按1天算，因此大于等于0时需要加1天
	if daysLeft < 0 {
		return expiredStatusExpiredAlready, daysLeft, nil
	} else if spWarn > daysLeft {
		return expiredStatusExpiredSoon, daysLeft + 1, nil
	}
	return expiredStatusNormal, daysLeft + 1, nil
}

func (u *User) SetPasswordHint(hint string) (busErr *dbus.Error) {
	encodeHint := base64.StdEncoding.EncodeToString([]byte(hint))
	err := u.writeUserConfigWithChange(confKeyPasswordHint, encodeHint)
	if err == nil {
		u.setPropPasswordHint(hint)
	}
	return dbusutil.ToError(err)
}

func (u *User) GetReminderInfo() (info LoginReminderInfo, dbusErr *dbus.Error) {
	return getLoginReminderInfo(u.UserName), nil
}

/* secret question */
// #nosec G101
const secretQuestionDirectory = "/var/lib/dde-daemon/secret-question/"

func walkQuestions(username string, f func(id int, salt string) error) error {
	path := filepath.Join(secretQuestionDirectory, username)

	file, err := os.Open(path)
	if err != nil {
		if os.IsNotExist(err) {
			return nil
		}

		return err
	}
	defer file.Close()

	reader := bufio.NewReader(file)
	for {
		line, err := reader.ReadString('\n')
		if err != nil {
			if err == io.EOF {
				break
			}

			return err
		}

		line = strings.TrimSpace(line)
		if len(line) == 0 {
			return nil
		}

		item := strings.SplitN(line, ":", 2)
		if len(item) != 2 {
			return xerrors.New("wrong format")
		}

		id, err := strconv.Atoi(item[0])
		if err != nil {
			return err
		}

		salt := item[1]

		err = f(id, salt)
		if err != nil {
			return err
		}
	}

	return nil
}

func (u *User) GetSecretQuestions() (list []int, err *dbus.Error) {
	err1 := walkQuestions(u.UserName, func(id int, salt string) error {
		list = append(list, id)

		return nil
	})
	if err1 != nil {
		err = dbusutil.ToError(err1)
	}

	return
}

func (u *User) VerifySecretQuestions(answers map[int]string) (failed []int, err *dbus.Error) {
	err1 := walkQuestions(u.UserName, func(id int, salt string) error {
		answer, ok := answers[id]
		if !ok || len(answer) == 0 {
			return xerrors.New("empty answer")
		}

		answerCStr := C.CString(answer)
		defer C.free(unsafe.Pointer(answerCStr))

		saltCStr := C.CString(salt)
		defer C.free(unsafe.Pointer(saltCStr))

		data := C.struct_crypt_data{}
		data.initialized = 0

		// salt 格式：$类型$salt&encrypted，无需去除 encrypted
		res := C.crypt_r(answerCStr, saltCStr, &data)
		if res == nil {
			return fmt.Errorf("encrypt failed")
		}

		resStr := C.GoString(res)
		if resStr != salt {
			failed = append(failed, id)
		}

		return nil
	})
	if err1 != nil {
		err = dbusutil.ToError(err1)
		return
	}

	return
}

func (u *User) SetSecretQuestions(sender dbus.Sender, list map[int][]byte) *dbus.Error {
	if len(list) != 3 {
		return &dbus.ErrMsgInvalidArg
	}

	err := u.checkAuth(sender, false, polkitActionChangeOwnData)
	if err != nil {
		return dbusutil.ToError(err)
	}

	err = os.MkdirAll(secretQuestionDirectory, os.ModePerm)
	if err != nil {
		return dbusutil.ToError(err)
	}

	path := filepath.Join(secretQuestionDirectory, u.UserName)

	var content bytes.Buffer
	for id, encryptedAnswer := range list {
		content.WriteString(strconv.Itoa(id))
		content.WriteRune(':')
		content.Write(encryptedAnswer)
		content.WriteRune('\n')
	}

	err = ioutil.WriteFile(path, content.Bytes(), 0600)
	return dbusutil.ToError(err)
}

func (u *User) SetSecretKey(sender dbus.Sender, secretKey string) *dbus.Error {
	senderName := u.getSenderDBus(sender)
	logger.Debugf("[SetSecretKey] sender : %s, senderName : %s, UserName : %s : ", sender, senderName, u.UserName)
	if !strings.Contains(senderName, controlCenter) {
		return dbusutil.ToError(errors.New("invalid sender"))
	}

	if u.uadpInterface == nil {
		return nil
	}
	err := u.uadpInterface.Set(0, u.UserName, []uint8(secretKey))
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) GetSecretKey(sender dbus.Sender, username string) (string, *dbus.Error) {
	senderName := u.getSenderDBus(sender)
	logger.Debugf("[GetSecretKey] sender : %s, senderName : %s, UserName : %s : ", sender, senderName, username)
	if !(strings.Contains(senderName, resetPasswordDia) || strings.Contains(senderName, controlCenter)) {
		return "", dbusutil.ToError(errors.New("invalid sender"))
	}
	if u.uadpInterface == nil {
		return "", nil
	}
	key, err := u.uadpInterface.Get(0, username)
	if err != nil {
		return string(key), dbusutil.ToError(err)
	}
	return string(key), nil
}

func (u *User) DeleteSecretKey(sender dbus.Sender) *dbus.Error {
	senderName := u.getSenderDBus(sender)
	logger.Debugf("[DeleteSecretKey] sender : %s, senderName : %s, UserName : %s : ", sender, senderName, u.UserName)
	if !strings.Contains(senderName, controlCenter) {
		return dbusutil.ToError(errors.New("invalid sender"))
	}

	if u.uadpInterface == nil {
		return nil
	}
	err := u.uadpInterface.Delete(0, u.UserName)
	if err != nil {
		return dbusutil.ToError(err)
	}
	return nil
}

func (u *User) deleteSecretKey() error {
	if u.uadpInterface == nil {
		logger.Warning("uadpInterface is nil")
		return nil
	}

	return u.uadpInterface.Delete(0, u.UserName)
}
