// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package display1

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"

	"github.com/godbus/dbus/v5"
	"github.com/linuxdeepin/dde-daemon/common/cpuinfo"
	"github.com/linuxdeepin/go-lib/dbusutil"
	"github.com/linuxdeepin/go-lib/log"
	"github.com/linuxdeepin/go-lib/procfs"
)

const (
	dbusServiceName     = "org.deepin.dde.Display1"
	dbusInterfaceName   = dbusServiceName
	dbusPath            = "/org/deepin/dde/Display1"
	configFilePath      = "/var/lib/dde-daemon/display/config.json"
	rendererConfigPath  = "/var/lib/dde-daemon/display/rendererConfig.json"
	supportLabcFilePath = "/sys/firmware/devicetree/base/sensors_cfg/support_labc"
)

//go:generate dbusutil-gen -type Display displaycfg.go
//go:generate dbusutil-gen em -type Display

type Display struct {
	service *dbusutil.Service
	cfg     *Config
	cfgMu   sync.Mutex

	rendererWaylandBlackList []string
	propMu                   sync.RWMutex
	doDetectMu               sync.Mutex
	SupportLabc              bool `prop:"access:r"`
	AutoBacklightEnabled     bool `prop:"access:rw"`

	signals *struct {
		ConfigUpdated struct {
			updateAt string
		}
		BacklightBrightnessUpdated struct {
			brightness float64
		}
	}
}

func newDisplay(service *dbusutil.Service) *Display {
	d := &Display{
		service: service,
	}
	cfg, err := loadConfig(configFilePath)
	if err != nil {
		if !os.IsNotExist(err) {
			logger.Warning(err)
		}
	}
	d.cfg = cfg
	rendererConfig, err := loadRendererConfig(rendererConfigPath)
	if err != nil {
		d.rendererWaylandBlackList = []string{
			"llvmpipe",
		} // 读取配置文件失败时，使用默认项，默认llvmpipe不支持wayland
		if !os.IsNotExist(err) {
			logger.Warning(err)
		} else {
			var cfg RendererConfig
			cfg.BlackList = d.rendererWaylandBlackList
			err := genRendererConfig(&cfg, rendererConfigPath) // 无该配置文件时，生成默认配置文件
			if err != nil {
				logger.Warning(err)
			}
		}
	} else {
		d.rendererWaylandBlackList = rendererConfig.BlackList
	}

	content, err := ioutil.ReadFile(supportLabcFilePath)
	if err != nil {
		logger.Warning(err)
	} else if strings.Contains(string(content), "enable") {
		cpuinfo, err := cpuinfo.ReadCPUInfo("/proc/cpuinfo")
		if err != nil {
			logger.Warning(err)
		} else if strings.HasPrefix(strings.TrimSpace(cpuinfo.Hardware), "PANGU") {
			d.SupportLabc = true
		}
	}

	if d.cfg != nil {
		d.AutoBacklightEnabled = d.cfg.AutoBacklightEnable
	}

	return d
}

func (d *Display) GetInterfaceName() string {
	return dbusInterfaceName
}

func (d *Display) GetConfig() (cfgStr string, busErr *dbus.Error) {
	var err error
	cfgStr, err = d.getConfig()
	return cfgStr, dbusutil.ToError(err)
}

func (d *Display) getConfig() (string, error) {
	d.cfgMu.Lock()
	defer d.cfgMu.Unlock()

	data, err := json.Marshal(d.cfg)
	if err != nil {
		return "", err
	}
	return string(data), nil
}

func (d *Display) SetConfig(cfgStr string) *dbus.Error {
	err := d.setConfig(cfgStr)
	return dbusutil.ToError(err)
}

func (d *Display) setConfig(cfgStr string) error {
	var cfg Config
	err := json.Unmarshal([]byte(cfgStr), &cfg)
	if err != nil {
		return err
	}

	d.cfgMu.Lock()
	defer d.cfgMu.Unlock()
	d.cfg = &cfg

	d.cfg.AutoBacklightEnable = d.AutoBacklightEnabled

	err = saveConfig(&cfg, configFilePath)
	if err != nil {
		return err
	}

	err = d.service.Emit(d, "ConfigUpdated", cfg.UpdateAt)
	if err != nil {
		logger.Warning(err)
	}

	return nil
}

type Config struct {
	Version             string
	Config              json.RawMessage
	AutoBacklightEnable bool
	UpdateAt            string
}

func loadConfig(filename string) (*Config, error) {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var cfg Config
	err = json.Unmarshal(content, &cfg)
	if err != nil {
		return nil, err
	}
	return &cfg, nil
}

func saveConfig(cfg *Config, filename string) error {
	content, err := json.Marshal(cfg)
	if err != nil {
		return err
	}

	err = os.MkdirAll(filepath.Dir(filename), 0755)
	if err != nil {
		return err
	}

	tmpFile := filename + ".tmp"
	err = ioutil.WriteFile(tmpFile, content, 0644)
	if err != nil {
		return err
	}

	err = os.Rename(tmpFile, filename)
	if err != nil {
		return err
	}

	return nil
}

type RendererConfig struct {
	BlackList []string
}

func loadRendererConfig(filename string) (*RendererConfig, error) {
	content, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var cfg RendererConfig
	err = json.Unmarshal(content, &cfg)
	if err != nil {
		return nil, err
	}
	return &cfg, nil
}

func genRendererConfig(cfg *RendererConfig, filename string) error {
	content, err := json.Marshal(cfg)
	if err != nil {
		return err
	}

	err = os.MkdirAll(filepath.Dir(filename), 0755)
	if err != nil {
		return err
	}

	tmpFile := filename + ".tmp"
	err = ioutil.WriteFile(tmpFile, content, 0644)
	if err != nil {
		return err
	}

	err = os.Rename(tmpFile, filename)
	if err != nil {
		return err
	}

	return nil
}

func (d *Display) SupportWayland(sender dbus.Sender) (bool, *dbus.Error) {
	if log.LevelDebug == logger.GetLogLevel() { // debug情况下，默认支持wayland，方便虚拟机调试
		return true, nil
	}
	rendererConfig, err := loadRendererConfig(rendererConfigPath)
	if err != nil {
		logger.Warning(err)
	}
	d.propMu.Lock()
	d.rendererWaylandBlackList = rendererConfig.BlackList
	d.propMu.Unlock()

	supportWayland, err := d.doDetectSupportWayland(sender)
	return supportWayland, dbusutil.ToError(err)
}

func (d *Display) doDetectSupportWayland(sender dbus.Sender) (bool, error) {
	d.doDetectMu.Lock()
	defer d.doDetectMu.Unlock()
	if binExist("glxinfo") {
		pid, err := d.service.GetConnPID(string(sender))
		if err != nil {
			logger.Warning(err)
			return false, err
		}

		p := procfs.Process(pid)
		environ, err := p.Environ()
		if err != nil {
			logger.Warning(err)
			return false, err
		}
		execPath, err := p.Exe()
		if err != nil {
			logger.Warning(err)
			return false, err
		}
		var cmd *exec.Cmd
		if execPath == "/usr/bin/lightdm-deepin-greeter" {
			cmd = exec.Command("runuser", "-u", "lightdm", "glxinfo") // runuser -u lightdm glxinfo
		} else {
			cmd = exec.Command("glxinfo")
		}
		environ = append(environ, "LC_ALL=C")
		cmd.Env = environ
		outPipe, err := cmd.StdoutPipe()
		if err != nil {
			logger.Warning(err)
			return false, err
		}
		reader := bufio.NewReader(outPipe)
		err = cmd.Start()
		if err != nil {
			logger.Warning(err)
			return false, err
		}
		for {
			line, err := reader.ReadBytes('\n')
			if err != nil {
				if io.EOF == err {
					break
				}
				logger.Warning(err)
				return false, err
			}

			if bytes.Contains(line, []byte("OpenGL renderer string:")) {
				// 待判断字段
				d.propMu.RLock()
				blackList := d.rendererWaylandBlackList
				d.propMu.RUnlock()
				// 当有一个renderer满足条件，即可表示支持wayland
				renderer := string(bytes.TrimSpace(bytes.Replace(line, []byte("OpenGL renderer string:"), []byte(""), 1)))
				logger.Debug("renderer: ", renderer)
				if renderer == "" {
					break
				}
				for i, blackRenderer := range blackList {
					if strings.Contains(renderer, blackRenderer) { // 该renderer包含黑名单的显卡，表示该renderer不支持wayland
						break
					}
					if i == len(blackList)-1 { // 黑名单中全部未命中，表示该显卡支持
						return true, nil
					}
				}
			}
		}
	} else {
		return !isInVM(), nil // 如果mesa-utils包没有安装，则直接判断是否在虚拟环境中
	}
	return false, nil
}

func binExist(bin string) bool {
	_, err := exec.LookPath(bin)
	return err == nil
}

func isInVM() bool {
	cmd := exec.Command("systemd-detect-virt", "-v", "-q")
	err := cmd.Start()
	if err != nil {
		logger.Warning(err)
		return false
	}

	err = cmd.Wait()
	return err == nil
}

func (d *Display) autoBacklightEnabledWriteCb(write *dbusutil.PropertyWrite) *dbus.Error {
	enabled, ok := write.Value.(bool)
	if !ok {
		err := fmt.Errorf("type of value is not bool")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}
	if enabled && !d.SupportLabc {
		err := fmt.Errorf("not support labc")
		logger.Warning(err)
		return dbusutil.ToError(err)
	}

	d.cfgMu.Lock()
	defer d.cfgMu.Unlock()

	if d.cfg == nil {
		d.cfg = &Config{}
	}

	d.cfg.AutoBacklightEnable = enabled

	err := saveConfig(d.cfg, configFilePath)
	if err != nil {
		logger.Warning(err)
	}

	return nil
}

func (d *Display) SetBacklightBrightness(val float64) *dbus.Error {
	logger.Infof("DBus Call SetBacklightBrightness(%v)", val)
	err := d.setBacklightBrightness(val)
	return dbusutil.ToError(err)
}

func (d *Display) setBacklightBrightness(val float64) error {
	if !d.AutoBacklightEnabled {
		return fmt.Errorf("not on auto backlight")
	}
	if !d.SupportLabc {
		return fmt.Errorf("not support labc")
	}

	err := d.service.Emit(d, "BacklightBrightnessUpdated", val)
	if err != nil {
		logger.Warning(err)
		return err
	}

	return nil
}
