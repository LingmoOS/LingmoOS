// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"fmt"
	"math"
	"strconv"
	"strings"
)

const (
	cpuKeyDelim        = ":"
	cpuKeyProcessor    = "processor"
	cpuKeyName         = "model name"
	cpuKeyCPU          = "cpu"
	cpuKeyMHz          = "CPU frequency [MHz]"
	cpuKeyActive       = "cpus active"
	cpuKeyARMProcessor = "Processor"
	cpuKeyHardware     = "Hardware"
	lscpuKeyMaxMHz     = "CPU max MHz"
	lscpuKeyModelName  = "Model name"
	lscpuKeyCount      = "CPU(s)"
	lscpuKeyCPUfamily  = "CPU family"
	lscpuKeyModel      = "Model"
	lscpuKeyStepping   = "Stepping"
	compareAllowMin    = 1e-6
)

func getProcessorByLscpuExt(data map[string]string, freq float64) (string, error) {
	modelName, ok := data[lscpuKeyModelName]
	if !ok {
		return "", fmt.Errorf("can not find the key %q", lscpuKeyModelName)
	}

	cpuCountStr, ok := data[lscpuKeyCount]
	if !ok {
		logger.Warningf("can not find the key %q", lscpuKeyCount)
		return modelName, nil
	}

	cpuCount, err := strconv.ParseInt(cpuCountStr, 10, 64)
	if err != nil {
		logger.Warning(err)
		return modelName, nil
	}
	if strings.Contains(modelName, "Hz") {
		return fmt.Sprintf("%s x %d", modelName, cpuCount), nil
	} else {
		return fmt.Sprintf("%s @ %.2fGHz x %d", modelName, freq, cpuCount), nil
	}
}

func getProcessorByLscpu(data map[string]string) (string, error) {
	modelName, ok := data[lscpuKeyModelName]
	if !ok {
		return "", fmt.Errorf("can not find the key %q", lscpuKeyModelName)
	}

	cpuCountStr, ok := data[lscpuKeyCount]
	if !ok {
		logger.Warningf("can not find the key %q", lscpuKeyCount)
		return modelName, nil
	}

	cpuCount, err := strconv.ParseInt(cpuCountStr, 10, 64)
	if err != nil {
		logger.Warning(err)
		return modelName, nil
	}

	return fmt.Sprintf("%s x %d", modelName, cpuCount), nil
}

func getCPUMaxMHzByLscpu(data map[string]string) (float64, error) {
	maxMHz, ok := data[lscpuKeyMaxMHz]
	if !ok {
		return 0, fmt.Errorf("can not find the key %q", lscpuKeyMaxMHz)
	}

	return strconv.ParseFloat(maxMHz, 64)
}

// float数比较
func isFloatEqual(f1, f2 float64) bool {
	return math.Abs(f1-f2) < compareAllowMin
}

func GetCPUInfo(file string) (string, error) {
	data, err := parseInfoFile(file, cpuKeyDelim)
	if err != nil {
		return "", err
	}

	cpu := swCPUInfo(data)
	if len(cpu) != 0 {
		return cpu, nil
	}

	// huawei kirin
	cpu = hwKirinCPUInfo(data)
	if len(cpu) != 0 {
		return cpu, nil
	}

	// arm
	cpu, _ = getCPUInfoFromMap(cpuKeyARMProcessor, cpuKeyProcessor, data)
	if len(cpu) != 0 {
		return cpu, nil
	}

	return getCPUInfoFromMap(cpuKeyName, cpuKeyProcessor, data)
}

func swCPUInfo(data map[string]string) string {
	cpu, err := getCPUName(cpuKeyCPU, data)
	if err != nil {
		return ""
	}

	hz, err := getCPUHz(cpuKeyMHz, data)
	if err == nil {
		cpu = fmt.Sprintf("%s %.2fGHz", cpu, hz)
	}

	number, _ := getCPUNumber(cpuKeyActive, data)
	if number != 1 {
		cpu = fmt.Sprintf("%s x %v", cpu, number)
	}

	return cpu
}

func hwKirinCPUInfo(data map[string]string) string {
	cpu, err := getCPUName(cpuKeyHardware, data)
	if err != nil {
		return ""
	}

	number, _ := getCPUNumber(cpuKeyProcessor, data)
	if number != 1 {
		cpu = fmt.Sprintf("%s x %v", cpu, number+1)
	}

	return cpu
}

func getCPUInfoFromMap(nameKey, numKey string, data map[string]string) (string, error) {
	name, err := getCPUName(nameKey, data)
	if err != nil {
		return "", err
	}

	number, _ := getCPUNumber(numKey, data)
	if number != 0 {
		name = fmt.Sprintf("%s x %v", name, number+1)
	}

	return name, nil
}

func getCPUName(key string, data map[string]string) (string, error) {
	value, ok := data[key]
	if !ok {
		return "", fmt.Errorf("can not find the key %q", key)
	}

	var name string
	array := strings.Split(value, " ")
	for i, v := range array {
		if len(v) == 0 {
			continue
		}
		name += v
		if i != len(array)-1 {
			name += " "
		}
	}

	return name, nil
}

func getCPUNumber(key string, data map[string]string) (int, error) {
	value, ok := data[key]
	if !ok {
		return 0, fmt.Errorf("can not find the key %q", key)
	}

	number, err := strconv.ParseInt(value, 10, 64)
	if err != nil {
		return 0, err
	}

	return int(number), nil
}

func getCPUHz(key string, data map[string]string) (float64, error) {
	value, ok := data[key]
	if !ok {
		return 0, fmt.Errorf("can not find the key %q", key)
	}

	hz, err := strconv.ParseFloat(value, 64)
	if err != nil {
		return 0, err
	}

	return hz / 1000, nil
}
