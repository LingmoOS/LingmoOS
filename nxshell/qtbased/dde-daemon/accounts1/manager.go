// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strconv"
	"sync"
	"syscall"
	"time"

	dbus "github.com/godbus/dbus/v5"
	configManager "github.com/linuxdeepin/go-dbus-factory/org.desktopspec.ConfigManager"
	udcp "github.com/linuxdeepin/go-dbus-factory/system/com.deepin.udcp.iam"
	ofdbus "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.dbus"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/tasker"
	dutils "github.com/linuxdeepin/go-lib/utils"

	"github.com/linuxdeepin/dde-daemon/accounts1/users"
	"github.com/linuxdeepin/dde-daemon/common/sessionmsg"
)

const (
	actConfigDir       = "/var/lib/AccountsService"
	userConfigDir      = actConfigDir + "/deepin/users"
	userIconsDir       = actConfigDir + "/icons"
	userCustomIconsDir = actConfigDir + "/icons/local"

	actConfigFile       = actConfigDir + "/accounts.ini"
	actConfigGroupGroup = "Accounts"
	actConfigKeyGuest   = "AllowGuest"

	interfacesFile = "/usr/share/dde-daemon/accounts/dbus-udcp.json"
)

var configFile = filepath.Join(actConfigDir, "domainUser.json")

type domainUserConfig struct {
	Name      string
	Uid       string
	IsLogined bool
}

type DefaultDomainUserConfig map[string]*domainUserConfig

type InterfaceConfig struct {
	Service   string `json:"service"`
	Path      string `json:"path"`
	Interface string `json:"interface"`
}

const (
	dsettingsAppID             = "org.deepin.dde.daemon"
	dsettingsAccountName       = "org.deepin.dde.daemon.account"
	dsettingsIsTerminalLocked  = "isTerminalLocked"
	gsSchemaDdeControlCenter   = "com.deepin.dde.control-center"
	settingKeyAutoLoginVisable = "auto-login-visable"
)

//go:generate dbusutil-gen -type Manager,User manager.go user.go
//go:generate dbusutil-gen em -type Manager,User,ImageBlur

type Manager struct {
	service       *dbusutil.Service
	sysSigLoop    *dbusutil.SignalLoop
	login1Manager login1.Manager
	PropsMu       sync.RWMutex

	UserList   []string
	UserListMu sync.RWMutex

	// dbusutil-gen: ignore
	GuestIcon  string
	AllowGuest bool
	// dbusutil-gen: equal=isStrvEqual
	GroupList []string

	watcher    *dutils.WatchProxy
	usersMap   map[string]*User
	usersMapMu sync.Mutex

	enablePasswdChangedHandler   bool
	enablePasswdChangedHandlerMu sync.Mutex

	delayTaskManager *tasker.DelayTaskManager
	userAddedChanMap map[string]chan string
	udcpCache        udcp.UdcpCache
	userConfig       DefaultDomainUserConfig
	domainUserMapMu  sync.Mutex
	cfgManager       configManager.ConfigManager
	dsAccount        configManager.Manager
	// greeter 的 dconfig 配置
	dsGreeterAccounts configManager.Manager

	dbusDaemon       ofdbus.DBus
	IsTerminalLocked bool
	// 快速登录总开关
	QuickLoginEnabled bool

	//nolint
	signals *struct {
		UserAdded struct {
			objPath string
		}

		UserDeleted struct {
			objPath string
		}
	}
}

func NewManager(service *dbusutil.Service) *Manager {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		return nil
	}
	login1Manager := login1.NewManager(systemBus)
	sysSigLoop := dbusutil.NewSignalLoop(systemBus, 10)
	sysSigLoop.Start()

	var m = &Manager{
		service:                    service,
		login1Manager:              login1Manager,
		sysSigLoop:                 sysSigLoop,
		enablePasswdChangedHandler: true,
	}
	m.usersMap = make(map[string]*User)
	m.userAddedChanMap = make(map[string]chan string)

	m.GuestIcon = getRandomIcon()
	m.AllowGuest = isGuestUserEnabled()

	m.initUsers(getUserPaths())
	m.initUdcpUsers()
	m.initAccountDSettings()

	// 检测到系统加入LDAP域后，才去初始化域用户信息
	ret, err := m.isJoinLDAPDoamin()
	if err == nil {
		if ret {
			m.initDomainUsers()
		}
	}

	if m.IsTerminalLocked {
		for _, u := range m.usersMap {
			if u.AutomaticLogin {
				u.setAutomaticLogin(false)
			}
		}
	}

	m.GroupList, _ = m.GetGroups()
	m.watcher = dutils.NewWatchProxy()
	if m.watcher != nil {
		m.delayTaskManager = tasker.NewDelayTaskManager()
		_ = m.delayTaskManager.AddTask(taskNamePasswd, fileEventDelay, m.handleFilePasswdChanged)
		_ = m.delayTaskManager.AddTask(taskNameGroup, fileEventDelay, m.handleFileGroupChanged)
		_ = m.delayTaskManager.AddTask(taskNameShadow, fileEventDelay, m.handleFileShadowChanged)
		_ = m.delayTaskManager.AddTask(taskNameDM, fileEventDelay, m.handleDMConfigChanged)
		_ = m.delayTaskManager.AddTask(taskNameGreeterState, fileEventDelay, m.handleGreeterStateChanged)

		m.watcher.SetFileList(m.getWatchFiles())
		m.watcher.SetEventHandler(m.handleFileChanged)
		go m.watcher.StartWatch()
	}

	m.login1Manager.InitSignalExt(m.sysSigLoop, true)
	_, _ = m.login1Manager.ConnectSessionNew(func(id string, sessionPath dbus.ObjectPath) {
		core, err := login1.NewSession(systemBus, sessionPath)
		if err != nil {
			logger.Warningf("new login1 session failed:%v", err)
			return
		}

		userInfo, err := core.User().Get(0)
		if err != nil {
			logger.Warningf("get user info failed:%v", err)
			return
		}

		if userInfo.UID < 10000 {
			return
		}

		err = m.addDomainUser(userInfo.UID)
		if err != nil {
			logger.Warningf("add login session failed:%v", err)
		}
	})

	_, _ = m.login1Manager.ConnectPrepareForSleep(func(before bool) {
		if before {
			return
		}

		pwdChangerLock.Lock()
		defer pwdChangerLock.Unlock()

		if pwdChangerProcess != nil {
			err := syscall.Kill(-pwdChangerProcess.Pid, syscall.SIGTERM)
			if err != nil {
				logger.Warning(err)
			}
		}
	})

	return m
}

func (m *Manager) destroy() {
	if m.watcher != nil {
		m.watcher.EndWatch()
		m.watcher = nil
	}

	m.sysSigLoop.Stop()
	m.stopExportUsers(m.UserList)
	_ = m.service.StopExport(m)
}

func (m *Manager) initUsers(list []string) {
	var userList []string
	for _, p := range list {
		u, err := NewUser(p, m.service, false)
		if err != nil {
			logger.Errorf("New user '%s' failed: %v", p, err)
			continue
		}

		userList = append(userList, p)

		m.usersMapMu.Lock()
		m.usersMap[p] = u
		m.usersMapMu.Unlock()
	}
	sort.Strings(userList)
	m.UserList = userList
}

func (m *Manager) initUdcpCache() error {
	// 解析json文件 新建udcp-cache对象
	var ifcCfg InterfaceConfig
	content, err := ioutil.ReadFile(interfacesFile)
	if err != nil {
		return err
	}
	err = json.Unmarshal(content, &ifcCfg)
	if err != nil {
		logger.Warning(err)
		return err
	}
	if !dbus.ObjectPath(ifcCfg.Path).IsValid() {
		logger.Warningf("interface config file %q, path %q is invalid", interfacesFile, ifcCfg.Path)
		return err
	}

	sysBus, err := dbus.SystemBus()
	if err != nil {
		return err
	}

	udcpCache, err := udcp.NewUdcpCache(sysBus, ifcCfg.Service, dbus.ObjectPath(ifcCfg.Path))
	if err != nil {
		return err
	}

	udcpCache.SetInterfaceName_(ifcCfg.Interface)

	m.udcpCache = udcpCache
	return nil

}

func (m *Manager) initUdcpUsers() {
	// 解析json文件 新建udcp-cache对象,获取所有加域账户ID
	err := m.initUdcpCache()
	if err != nil {
		logger.Warningf("New udcp cache object failed: %v", err)
		return
	}

	userIdList, err := m.udcpCache.GetUserIdList(0)
	if err != nil {
		logger.Warningf("Udcp cache getUserIdList failed: %v", err)
		return
	}

	isJoinUdcp, err := m.udcpCache.Enable().Get(0)
	if err != nil {
		logger.Warningf("Udcp cache get Enable failed: %v", err)
		return
	}

	if !isJoinUdcp {
		return
	}

	// 构造User服务对象
	var userList = m.UserList
	for _, uId := range userIdList {
		if users.ExistPwUid(uId) != 0 {
			continue
		}
		userGroups, err := m.udcpCache.GetUserGroups(0, users.GetPwName(uId))
		if err != nil {
			logger.Warningf("Udcp cache getUserGroups failed: %v", err)
			continue
		}

		u, err := NewDomainUser(uId, m.service, userGroups)
		if err != nil {
			logger.Warningf("New udcp user '%d' failed: %v", uId, err)
			continue
		}
		userDBusPath := userDBusPathPrefix + strconv.FormatUint(uint64(uId), 10)
		userList = append(userList, userDBusPath)
		m.usersMapMu.Lock()
		m.usersMap[userDBusPath] = u
		m.usersMapMu.Unlock()
	}
	sort.Strings(userList)
	m.UserList = userList
}

func (m *Manager) initDomainUsers() {
	var domainUserList []string
	// 解析json文件,获取所有之前登录过的域账号
	err := m.loadDomainUserConfig()

	if err != nil {
		logger.Warningf("init domain user config failed: %v", err)
		return
	}

	for _, v := range m.userConfig {
		if users.IsLDAPDomainUserID(v.Uid) && (v.IsLogined == true) {
			domainUserList = append(domainUserList, v.Uid)
		}
	}

	// 构造User服务对象
	var userList = m.UserList
	for _, uId := range domainUserList {
		id, _ := strconv.Atoi(uId)
		domainUserGroups, err := users.GetADUserGroupsByUID(uint32(id))
		if err != nil {
			logger.Warningf("get domain user groups failed: %v", err)
			return
		}

		u, err := NewDomainUser(uint32(id), m.service, domainUserGroups)
		if err != nil {
			logger.Errorf("New domain user '%s' failed: %v", uId, err)
			continue
		}

		userDBusPath := userDBusPathPrefix + uId
		userList = append(userList, userDBusPath)
		m.usersMapMu.Lock()
		m.usersMap[userDBusPath] = u
		m.usersMapMu.Unlock()
	}
	sort.Strings(userList)
	m.UserList = userList
}

func (m *Manager) exportUsers() {
	m.usersMapMu.Lock()

	for _, u := range m.usersMap {
		err := m.service.Export(dbus.ObjectPath(userDBusPathPrefix+u.Uid), u)
		if err != nil {
			logger.Errorf("failed to export user %q: %v",
				u.Uid, err)
			continue
		}

	}

	m.usersMapMu.Unlock()
}

func (m *Manager) stopExportUsers(list []string) {
	for _, p := range list {
		m.stopExportUser(p)
	}
}

func (m *Manager) exportUserByUid(uId string) error {
	var err error
	var u *User
	var userGroups []string
	userPath := userDBusPathPrefix + uId
	id, _ := strconv.Atoi(uId)

	if /*m.isUserJoinUdcp()*/ id > 10000 {
		if users.ExistPwUid(uint32(id)) != 0 {
			return errors.New("no such user id")
		}

		// 域管用户
		if m.udcpCache != nil {
			userGroups, err = m.udcpCache.GetUserGroups(0, users.GetPwName(uint32(id)))
			if err != nil {
				logger.Warningf("Udcp cache getUserGroups failed: %v", err)
			}
		}

		// LDAP 域用户
		if users.IsLDAPDomainUserID(uId) {
			userGroups, err = users.GetADUserGroupsByUID(uint32(id))
			if err != nil {
				logger.Warningf("get domain user groups failed: %v", err)
			}
		}

		if err != nil {
			return err
		}

		u, err = NewDomainUser(uint32(id), m.service, userGroups)

		if users.IsLDAPDomainUserID(uId) {
			var config = &domainUserConfig{
				Name:      u.UserName,
				Uid:       u.Uid,
				IsLogined: true,
			}

			m.domainUserMapMu.Lock()
			m.userConfig[userPath] = config
			m.saveDomainUserConfig(m.userConfig)
			m.domainUserMapMu.Unlock()
		}
	} else {
		u, err = NewUser(userPath, m.service, true)
	}
	if err != nil {
		return err
	}

	var stat = &syscall.Stat_t{}
	if err := syscall.Stat(u.HomeDir, stat); err != nil {
		logger.Warning(err)
	} else if strconv.Itoa(int(stat.Uid)) != u.Uid {
		logger.Debug("incorrect ownership")
		err = recoverOwnership(u)
		if err != nil {
			logger.Warning(err)
		}
	}

	m.usersMapMu.Lock()
	ch := m.userAddedChanMap[u.UserName]
	m.usersMapMu.Unlock()

	err = m.service.Export(dbus.ObjectPath(userPath), u)
	logger.Debugf("export user %q err: %v", userPath, err)
	if ch != nil {
		if err != nil {
			ch <- ""
		} else {
			ch <- userPath
		}
		logger.Debug("after ch <- userPath")
	}

	if err != nil {
		return err
	}

	m.usersMapMu.Lock()
	m.usersMap[userPath] = u
	m.usersMapMu.Unlock()

	return nil
}

func (m *Manager) stopExportUser(userPath string) {
	m.usersMapMu.Lock()
	defer m.usersMapMu.Unlock()
	u, ok := m.usersMap[userPath]
	if !ok {
		logger.Debug("Invalid user path:", userPath)
		return
	}

	delete(m.usersMap, userPath)
	_ = m.service.StopExport(u)
}

func (m *Manager) getUserByName(name string) *User {
	m.usersMapMu.Lock()
	defer m.usersMapMu.Unlock()

	for _, user := range m.usersMap {
		if user.UserName == name {
			return user
		}
	}
	return nil
}

func (m *Manager) getUserByUid(uid string) *User {
	m.usersMapMu.Lock()
	defer m.usersMapMu.Unlock()

	for _, user := range m.usersMap {
		if user.Uid == uid {
			return user
		}
	}
	return nil
}

func getUserPaths() []string {
	infos, err := users.GetHumanUserInfos()
	if err != nil {
		return nil
	}

	var paths []string
	for _, info := range infos {
		paths = append(paths, userDBusPathPrefix+info.Uid)
	}

	return paths
}

func isGuestUserEnabled() bool {
	v, exist := dutils.ReadKeyFromKeyFile(actConfigFile,
		actConfigGroupGroup, actConfigKeyGuest, true)
	if !exist {
		return false
	}

	ret, ok := v.(bool)
	if !ok {
		return false
	}

	return ret
}

func (m *Manager) checkAuth(sender dbus.Sender) error {
	return checkAuth(polkitActionUserAdministration, string(sender))
}

func chownHomeDir(homeDir string, username string) {
	logger.Debug("change owner for dir:", homeDir)
	err := exec.Command("chown", "-hR", username+":"+username, homeDir).Run()
	if err != nil {
		logger.Warningf("change owner for dir %v failed: %v", homeDir, err)
		return
	}
	err = sessionmsg.SendMessage(sessionmsg.NewMessage(true, &sessionmsg.BodyNotify{
		Icon: "preferences-system",
		Body: &sessionmsg.LocalizeStr{
			Format: Tr("User \"%s\" existed before and its data is synced"),
			Args:   []string{username},
		},
		ExpireTimeout: -1,
	}))
	if err != nil {
		logger.Warning("send session msg failed:", err)
	}
}

func Tr(text string) string {
	return text
}

func (m *Manager) loadDomainUserConfig() error {
	logger.Debug("loadDomainUserConfig")

	m.userConfig = make(DefaultDomainUserConfig)
	if !dutils.IsFileExist(configFile) {
		err := dutils.CreateFile(configFile)
		if err != nil {
			return err
		}
	} else {
		data, err := ioutil.ReadFile(configFile)
		if err != nil {
			return err
		}

		if len(data) == 0 {
			return errors.New("domain user config file is empty")
		}

		err = json.Unmarshal(data, &m.userConfig)
		if err != nil {
			return err
		}
	}

	return nil
}

func (m *Manager) saveDomainUserConfig(config DefaultDomainUserConfig) error {
	data, err := json.Marshal(config)
	if err != nil {
		return err
	}

	dir := filepath.Dir(configFile)
	err = os.MkdirAll(dir, 0755)
	if err != nil {
		return err
	}

	err = ioutil.WriteFile(configFile, data, 0644)
	return err
}

func (m *Manager) isJoinLDAPDoamin() (bool, error) {
	out, err := exec.Command("realm", "list").CombinedOutput()
	if err != nil {
		logger.Debugf("failed to execute %s, error: %v", "realm list", err)
		return false, err
	}

	if len(out) == 0 {
		return false, nil
	}

	return true, nil
}

func (m *Manager) modifyUserConfig(path string) error {
	m.usersMapMu.Lock()
	root, ok := m.usersMap[path]
	m.usersMapMu.Unlock()
	if ok {
		loadUserConfigInfo(root)
	}

	return nil
}

func (m *Manager) checkGroupCanChange(name string) bool {
	groupInfoMap, err := users.GetGroupInfoWithCacheLock()
	if err != nil {
		logger.Warning(err)
		return false
	}
	info, ok := groupInfoMap[name]
	if !ok {
		return false
	}
	gid, err := strconv.Atoi(info.Gid)
	if err != nil {
		logger.Warning(err)
		return false
	}
	return gid >= 1000
}

func (m *Manager) initAccountDSettings() {
	m.cfgManager = configManager.NewConfigManager(m.sysSigLoop.Conn())

	accountPath, err := m.cfgManager.AcquireManager(0, dsettingsAppID, dsettingsAccountName, "")
	if err != nil {
		logger.Warning(err)
		return
	}

	m.dsAccount, err = configManager.NewManager(m.sysSigLoop.Conn(), accountPath)
	if err != nil {
		logger.Warning(err)
		return
	}

	v, err := m.dsAccount.Value(0, dsettingsIsTerminalLocked)
	if err != nil {
		logger.Warning(err)
		return
	}

	if data, ok := v.Value().(bool); ok {
		m.IsTerminalLocked = data
	}
}

const (
	greeterAppId             = "org.deepin.dde.lightdm-deepin-greeter"
	daemonAccountsResourceId = "org.deepin.dde.daemon.accounts"
	keyEnableQuickLogin      = "enableQuickLogin"
)

// 在 dconfig 设置 quick login （总开关）
func (m *Manager) setDConfigQuickLoginEnabled(enabled bool) error {
	if m.dsGreeterAccounts == nil {
		return errors.New("get greeter accounts dconfig failed")
	}
	err := m.dsGreeterAccounts.SetValue(0, keyEnableQuickLogin, dbus.MakeVariant(enabled))
	if err != nil {
		return fmt.Errorf("set greeter dconfig enableQuickLogin failed, err: %v", err)
	}
	return nil
}

// 从 dconfig 获取 quick login （总开关）的配置
func (m *Manager) getDConfigQuickLoginEnabled() (bool, error) {
	if m.dsGreeterAccounts == nil {
		return false, errors.New("get greeter accounts dconfig failed")
	}
	enabledVar, err := m.dsGreeterAccounts.Value(0, keyEnableQuickLogin)
	if err != nil {
		return false, fmt.Errorf("get greeter accounts dconfig enableQuickLogin failed, err: %v", err)
	}

	enabled, ok := enabledVar.Value().(bool)
	if !ok {
		return false, errors.New("enabledVar.Value() is not bool type")
	}
	return enabled, nil
}

func (m *Manager) initDConfigGreeterWatch() error {
	greeterAccountsPath, err := m.cfgManager.AcquireManager(0, greeterAppId, daemonAccountsResourceId, "")
	if err != nil {
		return err
	}

	m.dsGreeterAccounts, err = configManager.NewManager(m.sysSigLoop.Conn(), greeterAccountsPath)
	if err != nil {
		return err
	}

	enabled, err := m.getDConfigQuickLoginEnabled()
	if err != nil {
		logger.Warning("getDConfigQuickLoginEnabled failed, err:", err)
	} else {
		// 从 dconfig 获取值，然后同步到 /etc/lightdm/lightdm.conf 配置文件中。
		err = users.SetLightDMQuickLoginEnabled(enabled)
		if err != nil {
			logger.Warning("SetLightDMQuickLoginEnabled failed, error:", err)
		}
		m.setPropQuickLoginEnabled(enabled)
	}

	m.dsGreeterAccounts.InitSignalExt(m.sysSigLoop, true)
	m.dsGreeterAccounts.ConnectValueChanged(func(key string) {
		// dconfig 配置改变
		if key != keyEnableQuickLogin {
			return
		}

		enabled, err := m.getDConfigQuickLoginEnabled()
		if err != nil {
			logger.Warning("getDConfigQuickLoginEnabled failed, err:", err)
			return
		}

		logger.Debug("handle dconfig quick login enabled changed", enabled)
		err = users.SetLightDMQuickLoginEnabled(enabled)
		if err != nil {
			logger.Warning("SetLightDMQuickLoginEnabled failed, error:", err)
		}
	})
	return nil
}

func (m *Manager) removeGreeterDConfigWatch() {
	if m.dsGreeterAccounts != nil {
		m.dsGreeterAccounts.RemoveAllHandlers()
	}
}

// 监控 dconfig-daemon 服务是否重新启动, 如果重新启动了，需要重新注册监控器。
func (m *Manager) initDBusDaemonWatch() {
	systemBus, err := dbus.SystemBus()
	if err != nil {
		logger.Warning("get system bus failed, err:", err)
		return
	}

	m.dbusDaemon = ofdbus.NewDBus(systemBus)
	m.dbusDaemon.InitSignalExt(m.sysSigLoop, false)

	// 手工构建匹配规则
	rule := dbusutil.NewMatchRuleBuilder().Sender(m.dbusDaemon.ServiceName_()).
		Path(string(m.dbusDaemon.Path_())).Member("NameOwnerChanged").
		Arg(0, m.cfgManager.ServiceName_()).
		Build()
	logger.Debug("rule:", rule.Str)
	err = rule.AddTo(systemBus)
	if err != nil {
		logger.Warning("add match rule failed, err:", err)
		return
	}

	_, err = m.dbusDaemon.ConnectNameOwnerChanged(func(name, oldOwner, newOwner string) {
		if name == m.cfgManager.ServiceName_() && oldOwner == "" && newOwner != "" {
			// 检测到 dconfig-daemon 服务启动后，重新初始化相关配置
			logger.Debug("monitored that the dconfig-daemon service has restarted")
			time.Sleep(2 * time.Second)
			m.removeGreeterDConfigWatch()
			m.initDConfigGreeterWatch()
		}
	})
	if err != nil {
		logger.Warning("connect NameOwnerChanged signal failed, err:", err)
	}
}
