// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"encoding/json"
	"errors"
	"fmt"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"sync"

	"github.com/jouyouyun/hardware/dmi"
	"github.com/linuxdeepin/go-lib/dbusutil"
)

//go:generate dbusutil-gen em -type Manager

// 该结构体可能不包含所有的lshw数据类型,如果有缺失需要新增字段
type lshwItemContent struct {
	ID            string `json:"id"`
	Class         string `json:"class"`
	Claimed       bool   `json:"claimed"`
	Handle        string `json:"handle"`
	Description   string `json:"description"`
	Product       string `json:"product"`
	Vendor        string `json:"vendor"`
	PhysID        string `json:"physid"`
	BusInfo       string `json:"businfo"`
	Version       string `json:"version"`
	Width         int    `json:"width"`
	Clock         int    `json:"clock"`
	Size          uint64 `json:"size"`
	Configuration struct {
		Driver  string `json:"driver"`
		Latency string `json:"latency"`
	} `json:"configuration"`
	Capabilities struct {
		PCIExpress    string `json:"pciexpress"`
		MSI           string `json:"msi"`
		PM            string `json:"pm"`
		VGAController bool   `json:"vga_controller"`
		BusMaster     string `json:"bus_master"`
		Capabilities  string `json:"cap_list"`
		ROM           string `json:"rom"`
	} `json:"capabilities"`
}

type lshwClassContent []lshwItemContent

// 缓存 lshw 指令获取到的数据 key/value --> class/data
var _lshwContent = make(map[string]lshwClassContent)
var _lshwContentMu sync.Mutex

const (
	dbusServiceName = "org.deepin.dde.SystemInfo1"
	dbusPath        = "/org/deepin/dde/SystemInfo1"
	dbusInterface   = dbusServiceName

	KB = 1 << 10
	MB = 1 << 20
	GB = 1 << 30
	TB = 1 << 40
	EB = 1 << 50
)

type Manager struct {
	service         *dbusutil.Service
	PropsMu         sync.RWMutex
	MemorySize      uint64
	MemorySizeHuman string
	CurrentSpeed    uint64
	DMIInfo         dmi.DMI
	DisplayDriver   string
	VideoDriver     string
}

func formatFileSize(fileSize uint64) (size string) {
	if fileSize < KB {
		return fmt.Sprintf("%.2fB", float64(fileSize)/float64(1))
	} else if fileSize < MB {
		return fmt.Sprintf("%.2fKB", float64(fileSize)/float64(KB))
	} else if fileSize < GB {
		return fmt.Sprintf("%.2fMB", float64(fileSize)/float64(MB))
	} else if fileSize < TB {
		return fmt.Sprintf("%.2fGB", float64(fileSize)/float64(GB))
	} else if fileSize < EB {
		return fmt.Sprintf("%.2fTB", float64(fileSize)/float64(TB))
	} else { // if fileSize < (1024 * 1024 * 1024 * 1024 * 1024 * 1024)
		return fmt.Sprintf("%.2fEB", float64(fileSize)/float64(EB))
	}
}

func (m *Manager) GetInterfaceName() string {
	return dbusInterface
}

func NewManager(service *dbusutil.Service) *Manager {
	var m = &Manager{
		service: service,
	}
	v, err := dmi.GetDMI()
	if err != nil {
		logger.Warning(err)
	} else {
		m.DMIInfo = *v
	}
	return m
}

func isStrEmpty(str string) (out bool) {
	return len(str) <= 0 || str == ""
}

func getLshwData(class string) (lshwClassContent, error) {
	if isStrEmpty(class) {
		logger.Info("class is Empty")
		return nil, errors.New("input is not allowed to be empty")
	}
	_lshwContentMu.Lock()
	defer _lshwContentMu.Unlock()
	_, ok := _lshwContent[class]
	if !ok {
		output, err := exec.Command("lshw", "-c", class, "-sanitize", "-quiet", "-json").Output()
		if err != nil {
			return nil, err
		}
		var newClassItemContent lshwClassContent
		err = json.Unmarshal(output, &newClassItemContent)
		if err != nil {
			return nil, err
		}
		_lshwContent[class] = newClassItemContent
	}
	return _lshwContent[class], nil
}

func (m *Manager) calculateMemoryViaLshw() error {
	classContent, err := getLshwData("memory")
	if err != nil {
		logger.Warning(err)
		return err
	}
	var ret uint64
	for _, item := range classContent {
		if strings.ToLower(item.Description) == "system memory" {
			ret = item.Size
		}
	}
	memory := formatFileSize(ret)
	m.PropsMu.Lock()
	// set property value
	m.setPropMemorySize(ret)
	m.setPropMemorySizeHuman(memory)
	m.PropsMu.Unlock()
	logger.Debug("system memory : ", ret)
	return nil
}

func GetCurrentSpeed(systemBit int) (uint64, error) {
	ret, err := getCurrentSpeed(systemBit)
	return ret, err
}

func getCurrentSpeed(systemBit int) (uint64, error) {
	var ret uint64 = 0
	cmdOutBuf, err := runDmidecode()
	if err != nil {
		return ret, err
	}
	ret, err = parseCurrentSpeed(cmdOutBuf, systemBit)
	if err != nil {
		logger.Error(err)
		return ret, err
	}
	logger.Debug("GetCurrentSpeed :", ret)
	return ret, err
}

func runDmidecode() (string, error) {
	cmd := exec.Command("dmidecode", "-t", "processor")
	out, err := cmd.Output()
	if err != nil {
		logger.Error(err)
	}
	return string(out), err
}

// From string parse "Current Speed"
func parseCurrentSpeed(bytes string, systemBit int) (result uint64, err error) {
	logger.Debug("parseCurrentSpeed data: ", bytes)
	lines := strings.Split(bytes, "\n")
	for _, line := range lines {
		if !strings.Contains(line, "Current Speed:") {
			continue
		}
		items := strings.Split(line, "Current Speed:")
		ret := ""
		if len(items) == 2 {
			// Current Speed: 3200 MHz
			ret = items[1]
			value, err := strconv.ParseUint(strings.TrimSpace(filterUnNumber(ret)), 10, systemBit)
			if err != nil {
				logger.Error(err)
				return result, err
			}
			result = value
		}
		break
	}
	return result, err
}

// 仅保留字符串中的数字
func filterUnNumber(value string) string {
	reg, err := regexp.Compile("[^0-9]+")
	if err != nil {
		logger.Fatal(err)
	}
	return reg.ReplaceAllString(value, "")
}

// 执行命令：/usr/bin/getconf LONG_BIT 获取系统位数
func (m *Manager) systemBit() string {
	output, err := exec.Command("/usr/bin/getconf", "LONG_BIT").Output()
	if err != nil {
		return "64"
	}

	v := strings.TrimRight(string(output), "\n")
	return v
}
