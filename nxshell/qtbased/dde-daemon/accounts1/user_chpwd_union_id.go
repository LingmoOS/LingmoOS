// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package accounts

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"os/user"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/accounts1/users"
	authenticate "github.com/linuxdeepin/go-dbus-factory/system/org.deepin.dde.authenticate1"
	login1 "github.com/linuxdeepin/go-dbus-factory/system/org.freedesktop.login1"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/procfs"
	dutils "github.com/linuxdeepin/go-lib/utils"
)

// 这里出于安全考虑, 一方面为了防止被 debug, 另一方面为了防止环境变量被篡改,
// 所以需要将重置密码的对话框 (文件位于 resetPwdDialogPath)
// 以一个特殊的用户的身份运行, 这个用户只做运行对话框这一件事情.

var pwdChangerUserName = "deepin-password-admin"                               //#nosec G101
const resetPwdDialogPath = "/usr/lib/dde-control-center/reset-password-dialog" //#nosec G101

func init() {
	fileInfo, err := os.Stat(resetPwdDialogPath)
	if err != nil {
		logger.Warning(err)
		return
	}

	stat := fileInfo.Sys().(*syscall.Stat_t)
	user, err := user.LookupId(fmt.Sprintf("%d", stat.Uid))
	if err != nil {
		logger.Warning(err)
		return
	}
	pwdChangerUserName = user.Username
}

// copy from golang 1.17, comment out some code because of unexported member
//
// String returns a human-readable description of c.
// It is intended only for debugging.
// In particular, it is not suitable for use as input to a shell.
// The output of String may vary across Go releases.
func cmdToString(c *exec.Cmd) string {

	// if c.lookPathErr != nil {
	// // failed to resolve path; report the original requested path (plus args)
	// return strings.Join(c.Args, " ")
	// }

	// report the exact executable path (plus args)
	b := new(strings.Builder)
	b.WriteString(c.Path)
	for _, a := range c.Args[1:] {
		b.WriteByte(' ')
		b.WriteString(a)
	}
	return b.String()
}

// caller 表示启动短信验证码更改密码过程的调用者
type caller struct {
	display string
	xauth   string
	app     string
	wayland string // wayland socket, almost "/run/user/{UID}/wayland-0"
	proc    procfs.Process
	user    *user.User // 启动更改密码过程的用户, 不是被改密码的用户
}

// 创建 caller 对象
// 此函数主要负责验证调用者身份 (通过验证其二进制路径), 并获得其 .Xauthority 文件位置, 以及 DISPLAY 变量
// note: 尽量避免从调用者的环境变量中获取数值
func newCaller(service *dbusutil.Service, sender dbus.Sender) (ret *caller, err error) {
	pid, err := service.GetConnPID(string(sender))
	if err != nil {
		err = fmt.Errorf("fail to get sender PID: %v", err)
		return
	}

	proc := procfs.Process(pid)
	login1Manager := login1.NewManager(service.Conn())
	sessionPath, err := login1Manager.GetSessionByPID(0, pid)
	if err != nil {
		err = fmt.Errorf("fail to get sender session path: %v", err)
		return
	}

	session, err := login1.NewSession(service.Conn(), sessionPath)
	if err != nil {
		err = fmt.Errorf("fail to create session object proxy: %v", err)
		return
	}

	display, err := session.Display().Get(0)
	if err != nil {
		err = fmt.Errorf("fail to get sender display: %v", err)
		return
	}

	exe, err := proc.Exe()
	if err != nil {
		err = fmt.Errorf("get sender exe error: %v", err)
		return
	}

	// 只允许来自控制中心, 锁屏和 greetter 的调用
	var app string
	switch exe {
	case "/usr/bin/dde-control-center":
		app = "control-center"
	case "/usr/bin/dde-lock":
		app = "lock"
	case "/usr/bin/lightdm-deepin-greeter":
		app = "greeter"
	default:
		err = fmt.Errorf("set password with Union ID called by %s, which is not allow", exe)
		return
	}

	status, err := proc.Status()
	if err != nil {
		err = fmt.Errorf("failed to get sender status: %v", err)
		return
	}

	uids, err := status.Uids()
	if err != nil {
		err = fmt.Errorf("failed to get sender euid: %v", err)
		return
	}

	uid := uids[1] // EUID

	user, err := user.LookupId(fmt.Sprint(uid))
	if err != nil {
		err = fmt.Errorf("failed to lookup sender username: %v", err)
		return
	}

	envs, err := proc.Environ()
	if err != nil {
		err = fmt.Errorf("failed to get sender environment variables: %v", err)
		return
	}

	sessionType, err := session.Type().Get(0)
	if err != nil {
		err = fmt.Errorf("fail to get sender session type: %v", err)
		return
	}

	waylandSocket := getWaylandSocket(envs)

	if sessionType == "wayland" && waylandSocket == "" {
		err = fmt.Errorf("failed to get wayland socket")
		return
	}

	xauth, found := envs.Lookup("XAUTHORITY")
	if !found {
		// $HOME/.Xauthority is default authority file if XAUTHORITY isn't defined.
		xauth = filepath.Join(user.HomeDir, ".Xauthority")
	}

	ret = &caller{
		display: display,
		xauth:   xauth,
		proc:    proc,
		user:    user,
		app:     app,
		wayland: waylandSocket,
	}
	return
}

func getWaylandSocket(envs procfs.EnvVars) (waylandSocket string) {
	run, found := envs.Lookup("XDG_RUNTIME_DIR")
	if !found {
		return ""
	}

	display, found := envs.Lookup("WAYLAND_DISPLAY")
	if !found {
		return ""
	}

	return path.Join(run, display)
}

type pwdChanger interface {
	clean()                          // clean all stuff relay to pwdChanger
	runDialog() (*os.Process, error) // run the dialog
	wait()                           // wait dialog close
}

type pwdChangerBase struct {
	targetUser *User
	selfUid    int
	selfGid    int
	cmd        *exec.Cmd
	stdin      io.WriteCloser
	stdout     io.ReadCloser
	stderr     io.ReadCloser
	readPipe   *os.File
	writePipe  *os.File
}

type pwdChangerX struct {
	*pwdChangerBase
	xauthDir string
}

type pwdChangerWayland struct {
	*pwdChangerBase
	waylandSocket string
}

func (pcr *pwdChangerBase) runDialog() (*os.Process, error) {
	// start dialog
	logger.Debugf("set password with union id: run \"%s\", envs: %v", cmdToString(pcr.cmd), pcr.cmd.Env)
	pcr.cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
	err := pcr.cmd.Start()
	if err != nil {
		err = fmt.Errorf("fail to start reset-password-dialog: %v", err)
	}
	return pcr.cmd.Process, err
}

func (pcr *pwdChangerBase) wait() {
	// close writePipe which is the write end fd has been passed to and used by dialog
	err := pcr.writePipe.Close()
	if err != nil {
		logger.Warningf("fail to close write side of pipe: %v", err)
	}

	rpipeReader := bufio.NewReader(pcr.readPipe)
	stdinWriter := bufio.NewWriter(pcr.stdin)

	// read shadowed pwd from dialog, this input should end with a '\n'
	line, err := rpipeReader.ReadString('\n')
	if err != nil {
		logger.Warningf("set password with union ID: read line from reset-password-dialog failed: %v", err)
		_, _ = stdinWriter.Write([]byte(fmt.Sprintf("fail to read from reset-password-dialog: %v\n", err)))
		_ = stdinWriter.Flush()
		return
	}

	// remove '\n'
	line = line[:len(line)-1]

	// modify password
	err = users.ModifyPasswd(line, pcr.targetUser.UserName)
	if err != nil {
		logger.Warningf("set password with union ID: fail to modify password: %v", err)
		_, _ = stdinWriter.Write([]byte(fmt.Sprintf("fail to modify password: %v\n", err)))
		_ = stdinWriter.Flush()
		return
	}

	// send success message to dialog
	_, err = stdinWriter.Write([]byte("success\n"))
	_ = stdinWriter.Flush()
	if err != nil {
		logger.Warningf("set password with union ID: fail to write success message: %v", err)
	}

	// reset limits
	auth := authenticate.NewAuthenticate(pcr.targetUser.service.Conn())
	err = auth.ResetLimits(0, pcr.targetUser.UserName)
	if err != nil {
		logger.Warningf("set password with union ID: fail to reset limits: %v", err)
	}

	// force remove user's login keyring, since the data inside is useless
	err = removeLoginKeyring(pcr.targetUser)
	if err != nil {
		logger.Warningf("remove login keyring fail: %v", err)
	}

	stderrBuf := new(bytes.Buffer)
	stdoutBuf := new(bytes.Buffer)

	stdoutReader := bufio.NewReader(pcr.stdout)
	stderrReader := bufio.NewReader(pcr.stderr)

	_, err = io.Copy(stderrBuf, stderrReader)
	if err != nil {
		logger.Warningf("fail to get stderr of reset-password-dialog: %v", err)
	}

	_, err = io.Copy(stdoutBuf, stdoutReader)
	if err != nil {
		logger.Warningf("fail to get stdout of reset-password-dialog: %v", err)
	}

	err = pcr.cmd.Wait()
	if err != nil {
		logger.Warningf("reset-password-dialog exited: %v\nstderr:\n%v\nstdout:\n%v", err, stderrBuf, stdoutBuf)
	}

	// Terminate all sessions of this user, since its password is changed, and keyring has been force removed
	login1Manager := login1.NewManager(pcr.targetUser.service.Conn())
	uid, err := strconv.Atoi(pcr.targetUser.Uid)
	if err != nil {
		logger.Warningf("fail to get convert uid==%d: %v", uid, err)
		return
	}

	userPath, err := login1Manager.GetUser(0, uint32(uid))
	if err != nil {
		logger.Infof("fail to get user by uid==%d: %v", uid, err)
		return
	}

	login1User, err := login1.NewUser(pcr.targetUser.service.Conn(), userPath)
	if err != nil {
		logger.Warningf("fail to create login1 User Object: %v", err)
		return
	}

	err = login1User.Terminate(0)
	if err != nil {
		logger.Warningf("fail to terminate user session: %v", err)
		return
	}

	return
}

// clean all stuff relay to pwdChanger
func (pcrb *pwdChangerBase) clean() {
	err := pcrb.readPipe.Close()
	if err != nil {
		logger.Warningf("fail to close read end of pipe: %v", err)
	}
}

func (pcrx *pwdChangerX) clean() {
	pcrx.pwdChangerBase.clean()

	err := os.RemoveAll(pcrx.xauthDir)
	if err != nil {
		logger.Warningf("fail to remove tmp xauth dir: %v", err)
	}
}

func (pcrw *pwdChangerWayland) clean() {
	pcrw.pwdChangerBase.clean()

	if err := runSetfacl(pwdChangerUserName, pcrw.waylandSocket, false); err != nil {
		logger.Warningf("fail to clean acl for wayland socket: %v", err)
	}

	if err := runSetfacl(pwdChangerUserName, filepath.Dir(pcrw.waylandSocket), false); err != nil {
		logger.Warningf("fail to clean acl for wayland socket directory: %v", err)
	}

	return
}

func newPwdChangerBase(caller *caller, u *User) (ret *pwdChangerBase, err error) {
	pwdChangerUser, err := user.Lookup(pwdChangerUserName)
	if err != nil {
		err = fmt.Errorf("fail to get user info of %s: %v", pwdChangerUserName, err)
		return
	}

	selfUid, err := strconv.Atoi(pwdChangerUser.Uid)
	if err != nil {
		err = fmt.Errorf("fail to convert uid to int: %v", err)
		return
	}

	selfGid, err := strconv.Atoi(pwdChangerUser.Gid)
	if err != nil {
		err = fmt.Errorf("fail to convert gid to int: %v", err)
		return
	}

	// 检验可执行文件的属性
	fInfo, err := os.Stat(resetPwdDialogPath)
	if err != nil {
		err = fmt.Errorf("get reset-password-dislog stat fail: %v", err)
		return
	}

	fileSys := fInfo.Sys()
	stat, ok := fileSys.(*syscall.Stat_t)
	if !ok {
		err = errors.New("fail to get stat of reset-password-dialog")
		return
	}

	// TODO does this convert to uint32 safe?
	if !(fInfo.Mode() == 0500 && stat.Uid == uint32(selfUid)) {
		err = errors.New("reset-password-dialog permission check failed")
		return
	}

	// create pipe for dialog to send shadowed pwd
	readPipe, writePipe, err := os.Pipe()
	if err != nil {
		err = fmt.Errorf("fail to create pipe: %v", err)
		return
	}

	// -u 用户名
	// -a 应用类型
	// --fd 传递密码使用的文件描述符

	cmd := exec.Command(
		"runuser", "-u", pwdChangerUser.Username, "--",
		resetPwdDialogPath, "-u", u.UserName, "-f", u.FullName, "-a", caller.app,
		"--fd", "3") //#nosec G204

	cmd.ExtraFiles = append(cmd.ExtraFiles, writePipe)

	stdin, err := cmd.StdinPipe()
	if err != nil {
		err = fmt.Errorf("get stdinpipe failed: %v", err)
		return
	}

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		err = fmt.Errorf("get stdoutpipe failed: %v", err)
		return
	}

	stderr, err := cmd.StderrPipe()
	if err != nil {
		err = fmt.Errorf("get stderrpipe failed: %v", err)
		return
	}

	cmd.Env = append(
		cmd.Env,
		"LANG="+u.Locale,
		"LANGUAGE="+u.Locale,
	)

	ret = &pwdChangerBase{
		targetUser: u,
		selfUid:    selfUid,
		selfGid:    selfGid,
		cmd:        cmd,
		stdin:      stdin,
		stdout:     stdout,
		stderr:     stderr,
		readPipe:   readPipe,
		writePipe:  writePipe,
	}
	return
}

// 此函数负责初始化用于更改密码的用户
// 对话框的语言和被修改密码的用户的语言设置保持一致
func newPwdChanger(caller *caller, u *User) (ret pwdChanger, err error) {
	pcrb, err := newPwdChangerBase(caller, u)
	if caller.wayland == "" {
		ret, err = newPwdChangerX(caller, u, pcrb)
	} else {
		ret, err = newPwdChangerWayland(caller, u, pcrb)
	}

	return
}

// 此函数负责将来自 caller 的 xauth 凭证发送给 deepin-password-admin 用户
func newPwdChangerX(caller *caller, u *User, base *pwdChangerBase) (ret *pwdChangerX, err error) {
	xAuthDir := filepath.Join("/run", "user", fmt.Sprint(base.selfUid))
	defer func() {
		if err != nil {
			err := os.RemoveAll(xAuthDir)
			if err != nil {
				logger.Warningf("fail to remove tmp xauth dir: %v", err)
			}
		}
	}()

	err = os.Mkdir(xAuthDir, os.FileMode(0700))
	if err != nil {
		if os.IsExist(err) {
			logger.Warningf("path %s existed, remove it now", xAuthDir)
			err = os.RemoveAll(xAuthDir)
			if err != nil {
				err = fmt.Errorf("fail to remove existed dir that hold .Xauthority: %v", err)
				return
			}
			err = os.Mkdir(xAuthDir, os.FileMode(0700))
		}
		if err != nil {
			err = fmt.Errorf("fail to create dir that hold .Xauthority: %v", err)
			return
		}
	}

	err = os.Chown(xAuthDir, base.selfUid, base.selfGid)
	if err != nil {
		err = fmt.Errorf("fail to chown dir that hold .Xauthority: %v", err)
		return
	}

	xauthPath := filepath.Join(xAuthDir, ".Xauthority")
	file, err := os.Create(xauthPath)
	if err != nil {
		err = fmt.Errorf("fail to create .Xauthority: %v", err)
		return
	}

	err = file.Chown(base.selfUid, base.selfGid)
	if err != nil {
		err = fmt.Errorf("fail to chown .Xauthority: %v", err)
		return
	}

	err = file.Close()
	if err != nil {
		err = fmt.Errorf("fail to close .Xauthority: %v", err)
		return
	}

	// get caller's xauth
	cmd := exec.Command("runuser", "-u", caller.user.Username, "--", "xauth", "list") //#nosec G204
	cmd.Env = append(cmd.Env, "XAUTHORITY="+caller.xauth)
	logger.Debugf("set password with union id: run \"%s\", envs: %v", cmdToString(cmd), cmd.Env)

	xauths, err := cmd.Output()
	if err != nil {
		err = fmt.Errorf("get xauth cookie failed: %v", err)
		return
	}

	// Add all xauth entries from caller to pwdChanger
	// After change hostname, some werid things happen to .Xauthority before a reboot
	// The prefix match not working here. So we have to add all entries to pwdChanger's .Xauthority file
	lines := strings.Split(string(xauths), "\n")
	args := []string{"-u", pwdChangerUserName, "--", "xauth", "add"}
	for _, line := range lines {
		fields := strings.Split(line, "  ")
		if fields[0] == "" {
			continue
		}
		arg := append(args, fields...)
		cmd = exec.Command("runuser", arg...) //#nosec G204
		cmd.Env = append(cmd.Env, "XAUTHORITY="+xauthPath)
		logger.Debugf("set password with union id: run \"%s\", envs: %v", cmdToString(cmd), cmd.Env)

		err = cmd.Run()
		if err != nil {
			err = fmt.Errorf("add xauth fail: %v", err)
			return
		}
	}

	base.cmd.Env = append(base.cmd.Env,
		"XAUTHORITY="+xauthPath,
		"DISPLAY="+caller.display,
	)

	ret = &pwdChangerX{
		pwdChangerBase: base,
		xauthDir:       xAuthDir,
	}
	return
}

func runSetfacl(user string, filePath string, isEnable bool) (err error) {
	flag := "-x" // remove
	perms := ""
	if isEnable {
		flag = "-m" // modify
		perms = ":rwx"
	}
	cmd := exec.Command("setfacl", flag, "u:"+user+perms, filePath) //#nosec G204
	logger.Debugf("set password with union id: run \"%s\", envs: %v", cmdToString(cmd), cmd.Env)
	err = cmd.Run()
	return
}

// 此函数负责临时允许 deepin-password-admin 用户访问目标用户的 wayland socket
func newPwdChangerWayland(caller *caller, u *User, base *pwdChangerBase) (ret *pwdChangerWayland, err error) {
	if err = runSetfacl(pwdChangerUserName, caller.wayland, true); err != nil {
		return
	}

	if err = runSetfacl(pwdChangerUserName, filepath.Dir(caller.wayland), true); err != nil {
		return
	}

	base.cmd.Env = append(base.cmd.Env,
		"XDG_SESSION_TYPE=wayland",
		"QT_WAYLAND_SHELL_INTEGRATION=kwayland-shell", // env copy from startdde-wayland
		"WAYLAND_DISPLAY="+caller.wayland,
	)

	ret = &pwdChangerWayland{
		pwdChangerBase: base,
		waylandSocket:  caller.wayland,
	}

	return
}

func (u *User) setPwdWithUnionID(sender dbus.Sender) (err error) {
	// 在应用真实启动前发生的错误, 应该要和 dbus 方法一起返回
	// 在应用启动后发生的错误, 会输出在后端的日志中.
	// 这里使用一个管道来传递错误信息
	return doSetPwdWithUnionID(u, sender, 3)
}

var pwdChangerLock sync.Mutex
var pwdChangerProcess *os.Process

func doSetPwdWithUnionID(u *User, sender dbus.Sender, count int) error {
	pwdChangerLock.Lock()

	if pwdChangerProcess != nil {
		err := syscall.Kill(-pwdChangerProcess.Pid, syscall.SIGTERM)
		if err != nil {
			logger.Warning(err)
		}
		pwdChangerLock.Unlock()

		// 等待协程结束
		if count > 0 {
			logger.Debug("retry after 100ms")
			time.Sleep(100 * time.Millisecond)
			return doSetPwdWithUnionID(u, sender, count-1)
		} else {
			return fmt.Errorf("kill pwd changer process failed")
		}
	}

	caller, err := newCaller(u.service, sender)
	if err != nil {
		pwdChangerLock.Unlock()
		return fmt.Errorf("newCaller failed: %v", err)
	}

	pcr, err := newPwdChanger(caller, u)
	if err != nil {
		pwdChangerLock.Unlock()
		return fmt.Errorf("setup pwdChanger fail: %v", err)
	}

	pwdChangerProcess, err = pcr.runDialog()
	pwdChangerLock.Unlock()
	if err != nil {
		pcr.clean()
		return err
	}

	go func() {
		defer func() {
			pwdChangerLock.Lock()
			pwdChangerProcess = nil
			pwdChangerLock.Unlock()
		}()
		pcr.wait()
		pcr.clean()
	}()

	return nil
}

// 删除用户的 login keyring
// 由于重置密码时没有输入原密码, 所以恢复 keyring 中的数据是不可能的, 只能直接移除掉.
func removeLoginKeyring(user *User) (err error) {
	//白盒密钥生效后，就不需要再删除keyring文件
	dir := path.Join(user.HomeDir, "/.local/share/deepin-keyrings-wb")
	isUseWhiteboxFunc := func() bool {
		statusFile := fmt.Sprintf("%s/status", dir)
		if dutils.IsFileExist(dir) && dutils.IsFileExist(statusFile) {
			content, err := ioutil.ReadFile(statusFile)
			if err != nil {
				return false
			}
			if len(content) < 2 {
				return false
			}
			if content[0] == '1' && content[1] == '1' {
				logger.Info("[removeLoginKeyring] The WhiteBox keyring password has taken effect.")
				return true
			}
		}
		return false
	}

	if !isUseWhiteboxFunc() {
		// FIXME
		// greeter 界面触发该功能时 user 的 session bus 不存在,
		// 所以只能简单地直接删除文件, 而不可能通过 keyring 的 daemon 删除密钥环
		// FIXME login keyring 的位置有没可能变化?
		loginFile := fmt.Sprintf("%s/login.keyring", dir)
		if dutils.IsFileExist(loginFile) {
			err = os.Remove(loginFile)
		}
	}

	return
}
