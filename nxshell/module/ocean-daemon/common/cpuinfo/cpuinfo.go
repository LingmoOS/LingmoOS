// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
package cpuinfo

import (
	"os"
	"regexp"
	"strconv"
	"strings"
)

const (
	FilePath = "/proc/cpuinfo"
)

type CPUInfo struct {
	Processors []Processor `json:"processors"`
	Hardware   string
}

func (c *CPUInfo) NumCPU() int {
	return len(c.Processors)
}

func (c *CPUInfo) NumCore() int {
	core := make(map[string]bool)

	for _, p := range c.Processors {
		pid := p.PhysicalId
		cid := p.CoreId

		if pid == -1 {
			return c.NumCPU()
		} else {
			// to avoid fmt import
			key := strconv.FormatInt(int64(pid), 10) + ":" + strconv.FormatInt(int64(cid), 10)
			core[key] = true
		}
	}

	return len(core)
}

func (c *CPUInfo) NumPhysicalCPU() int {
	pcpu := make(map[string]bool)

	for _, p := range c.Processors {
		pid := p.PhysicalId

		if pid == -1 {
			return c.NumCPU()
		} else {
			// to avoid fmt import
			key := strconv.FormatInt(int64(pid), 10)
			pcpu[key] = true
		}
	}

	return len(pcpu)
}

type Processor struct {
	Id         int64    `json:"id"`
	VendorId   string   `json:"vendor_id"`
	Model      int64    `json:"model"`
	ModelName  string   `json:"model_name"`
	Flags      []string `json:"flags"`
	Cores      int64    `json:"cores"`
	MHz        float64  `json:"mhz"`
	CacheSize  int64    `json:"cache_size"` // KB
	PhysicalId int64    `json:"physical_id"`
	CoreId     int64    `json:"core_id"`
}

var cpuinfoRegExp = regexp.MustCompile(`([^:]*?)\s*:\s*(.*)$`)

func ReadCPUInfo(path string) (*CPUInfo, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}

	content := string(b)
	lines := strings.Split(content, "\n")

	var cpuinfo = CPUInfo{}
	var processor = &Processor{CoreId: -1, PhysicalId: -1}
	isProcessor := false

	for i, line := range lines {
		var key string
		var value string

		if len(line) == 0 && i != len(lines)-1 {
			// end of processor
			if isProcessor {
				isProcessor = false
				cpuinfo.Processors = append(cpuinfo.Processors, *processor)
				processor = &Processor{}
			} else {
				processor = &Processor{CoreId: -1, PhysicalId: -1}
			}
			continue
		} else if i == len(lines)-1 {
			continue
		}

		if submatches := cpuinfoRegExp.FindStringSubmatch(line); submatches != nil {
			key = submatches[1]
			value = submatches[2]
		} else {
			continue
		}

		switch key {
		case "processor":
			processor.Id, _ = strconv.ParseInt(value, 10, 64)
			isProcessor = true
		case "vendor_id":
			processor.VendorId = value
		case "model":
			processor.Model, _ = strconv.ParseInt(value, 10, 64)
		case "model name":
			processor.ModelName = value
		case "flags":
			processor.Flags = strings.Fields(value)
		case "cpu cores":
			processor.Cores, _ = strconv.ParseInt(value, 10, 64)
		case "cpu MHz":
			processor.MHz, _ = strconv.ParseFloat(value, 64)
		case "cache size":
			processor.CacheSize, _ = strconv.ParseInt(value[:strings.IndexAny(value, " \t\n")], 10, 64)
			if strings.HasSuffix(line, "MB") {
				processor.CacheSize *= 1024
			}
		case "physical id":
			processor.PhysicalId, _ = strconv.ParseInt(value, 10, 64)
		case "core id":
			processor.CoreId, _ = strconv.ParseInt(value, 10, 64)
		case "Hardware":
			cpuinfo.Hardware = value
		}
	}
	return &cpuinfo, nil
}
