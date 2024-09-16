// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package systeminfo

import (
	"os"
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestCPUInfo(t *testing.T) {
	cpu, err := GetCPUInfo("testdata/cpuinfo")
	assert.Equal(t, cpu,
		"Intel(R) Core(TM) i3 CPU M 330 @ 2.13GHz x 4")
	assert.NoError(t, err)

	cpu, err = GetCPUInfo("testdata/sw-cpuinfo")
	assert.Equal(t, cpu, "sw 1.40GHz x 4")
	assert.NoError(t, err)

	cpu, err = GetCPUInfo("testdata/arm-cpuinfo")
	assert.Equal(t, cpu, "NANOPI2 x 4")
	assert.NoError(t, err)

	cpu, err = GetCPUInfo("testdata/hw_kirin-cpuinfo")
	assert.Equal(t, cpu, "HUAWEI Kirin 990 x 8")
	assert.NoError(t, err)
}

func TestMemInfo(t *testing.T) {
	mem, err := getMemoryFromFile("testdata/meminfo")
	assert.Equal(t, mem, uint64(4005441536))
	assert.NoError(t, err)
}

func TestVersion(t *testing.T) {
	lang := os.Getenv("LANGUAGE")
	os.Setenv("LANGUAGE", "en_US")
	defer os.Setenv("LANGUAGE", lang)

	deepin, err := getVersionFromDeepin("testdata/deepin-version")
	assert.Equal(t, deepin, "2015 Desktop Alpha1")
	assert.NoError(t, err)

	lsb, err := getVersionFromLSB("testdata/lsb-release")
	assert.Equal(t, lsb, "2014.3")
	assert.NoError(t, err)
}

func TestDistro(t *testing.T) {
	lang := os.Getenv("LANGUAGE")
	os.Setenv("LANGUAGE", "en_US")
	defer os.Setenv("LANGUAGE", lang)

	distroId, distroDesc, distroVer, err := getDistroFromLSB("testdata/lsb-release")
	assert.Equal(t, distroId, "Deepin")
	assert.Equal(t, distroDesc, "Deepin 2014.3")
	assert.Equal(t, distroVer, "2014.3")
	assert.NoError(t, err)
}

func TestSystemBit(t *testing.T) {
	v := systemBit()
	if v != "32" {
		assert.Equal(t, v, "64")
	}

	if v != "64" {
		assert.Equal(t, v, "32")
	}
}

func TestIsFloatEqual(t *testing.T) {
	assert.Equal(t, isFloatEqual(0.001, 0.0), false)
	assert.Equal(t, isFloatEqual(0.001, 0.001), true)
}

func TestParseInfoFile(t *testing.T) {
	v, err := parseInfoFile("testdata/lsb-release", "=")
	assert.NoError(t, err)
	assert.Equal(t, v["DISTRIB_ID"], "Deepin")
	assert.Equal(t, v["DISTRIB_RELEASE"], "2014.3")
	assert.Equal(t, v["DISTRIB_DESCRIPTION"], strconv.Quote("Deepin 2014.3"))
}

func TestGetCPUMaxMHzByLscpu(t *testing.T) {
	ret, err := parseInfoFile("testdata/lsCPU", ":")
	assert.NoError(t, err)
	v, err := getCPUMaxMHzByLscpu(ret)
	assert.NoError(t, err)
	assert.Equal(t, v, 3600.0000)
}

func TestGetProcessorByLscpuu(t *testing.T) {
	ret, err := parseInfoFile("testdata/lsCPU", ":")
	assert.NoError(t, err)
	v, err := getProcessorByLscpu(ret)
	assert.NoError(t, err)
	assert.Equal(t, v, "Intel(R) Core(TM) i5-4570 CPU @ 3.20GHz x 4")
}

func TestDoReadCache(t *testing.T) {
	ret, err := doReadCache("testdata/systeminfo.cache")
	assert.NoError(t, err)
	assert.Equal(t, ret.Version, "20 专业版")
	assert.Equal(t, ret.DistroID, "uos")
	assert.Equal(t, ret.DistroDesc, "UnionTech OS 20")
	assert.Equal(t, ret.DistroVer, "20")
	assert.Equal(t, ret.Processor, "Intel(R) Core(TM) i5-4570 CPU @ 3.20GHz x 4")
	assert.Equal(t, ret.DiskCap, uint64(500107862016))
	assert.Equal(t, ret.MemoryCap, uint64(8280711168))
	assert.Equal(t, ret.SystemType, int64(64))
	assert.Equal(t, ret.CPUMaxMHz, float64(3600))
	assert.Equal(t, ret.CurrentSpeed, uint64(0))
}
